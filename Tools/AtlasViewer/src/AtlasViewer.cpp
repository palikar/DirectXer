#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <cmath>

#include <fmt/format.h>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_rect_pack.h>

#include <Platform.hpp>
#include <Graphics.hpp>
#include <GraphicsCommon.hpp>
#include <Memory.hpp>
#include <Types.hpp>
#include <Utils.hpp>
#include <Assets.hpp>
#include <FileUtils.hpp>
#include <Platform.hpp>
#include <TexturePacker.hpp>

#include "AtlasViewer.hpp"
#include "DummyTexture.hpp"

static bool EndsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static void Update(Context& context)
{
	auto Graphics = &context.Graphics;

	Graphics->SetViewport(0, 0, 1080, 720);
	Graphics->SetBlendingState(BS_AlphaBlending);
	Graphics->SetDepthStencilState(DSS_2DRendering);
	Graphics->ResetRenderTarget();
	Graphics->ClearBuffer(0.0f, 0.0f, 0.0f);
	Graphics->ClearZBuffer();

	DrawFullscreenQuad(Graphics, context.DummyTex, SC_QUAD_SIMPLE);
	DrawFullscreenQuad(Graphics, context.Texs[context.currentTex], SC_QUAD_SIMPLE);
}

static void DumpImages(char* memory, uint16 count)
{
	for (size_t i = 0; i < count; ++i)
	{
		auto image = (ImageEntry*)(memory + sizeof(ImageEntry) * i);
		fmt::print("Image [{}]\tis in atlas [{}] and has size of [{}x{}]\n", image->Id, image->Id,
				   (int)roundf(image->Image.AtlasSize.x * image->Image.AtlasSize.y), (int)roundf(image->Image.AtlasSize.y *  image->Image.ScreenSize.y));

	}
}
template<typename T>
static void* GetData(MemoryArena& fileArena, T& entry)
{
	return fileArena.Memory + entry.DataOffset;
}

static void LoadAtlas(Context& context, const char* path)
{
	if (!PlatformLayer::IsValidPath(path)) return;
	
	auto Graphics = &context.Graphics;
	MemoryArena fileArena = Memory::GetTempArena(Megabytes(220));
	Defer {
		Memory::DestoryTempArena(fileArena);
	};
	ReadWholeFile(path, fileArena);
	context.Texs.clear();

	// @Todo: Delete the textures from GPU memory here
	
	
	if (EndsWith(path, ".datlas"))
	{
		auto current = fileArena.Memory;
		auto header = ReadBlob<AtlasFileHeader>(current);
		
		fmt::print("Opening an atlas file with [{}] images and [{}] atlases\n", header.NumImages, header.NumAtlases);
		context.Texs.resize(header.NumAtlases);
		
		for (size_t i = 0; i < header.NumAtlases; ++i)
		{
			auto atlas = ReadBlob<AtlasEntry>(current);
			fmt::print("Atlas [{}] has size [{}x{}]\n", i, atlas.Width, atlas.Height);
			context.Texs[i] = NextTextureId();
			Graphics->CreateTexture(context.Texs[i], {(uint16)atlas.Width, (uint16)atlas.Height, atlas.Format}, fileArena.Memory + atlas.Offset);
		}
	}
	else if (EndsWith(path, ".dbundle"))
	{
		auto current = fileArena.Memory;
		auto header = ReadBlob<AssetColletionHeader>(current);

		fmt::print("Opening an atlas file with [{}] images and [{}] atlases\n", header.ImagesCount, header.AtlasesCount);

		for (uint32 i = 0; i < header.TexturesCount; ++i)
		{
			const TextureLoadEntry& entry = ReadBlob<TextureLoadEntry>(current);
			Graphics->CreateTexture(entry.Id, entry.Desc, GetData(fileArena, entry));
			context.Texs.push_back(entry.Id);
		}		
	}	
}

static void Init(Context& context)
{
	auto Graphics = &context.Graphics;

	Memory::EstablishTempScope(Megabytes(16));
	Defer {
		Memory::EndTempScope();
	};
	int width, height, channels;
	unsigned char* data = stbi_load_from_memory(DUMMYTEXTURE_PNG, DUMMYTEXTURE_PNG_LEN, &width, &height, &channels, 4);
	context.DummyTex = NextTextureId();
	Graphics->CreateTexture(context.DummyTex, {(uint16)width, (uint16)height, PngFormat(channels)}, data);

	auto resolvedPath = fmt::format("{}/{}", context.Args.Root, context.Args.Input);
	LoadAtlas(context, resolvedPath.c_str());
}

void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Context& context)
{
	switch (msg)
	{
	  case WM_CLOSE:
	  {
		  PostQuitMessage(0);
	  }

	  case WM_KEYUP:
	  case WM_SYSKEYUP:
	  {
		  auto param = static_cast<uint16_t>(wParam);
		  // @Note: Change the current atlas with the arrow keys
		  if ( param == 37) {
			  context.currentTex = (context.currentTex - 1) % context.Texs.size();
		  } else if ( param == 39) {
			  context.currentTex = (context.currentTex + 1) % context.Texs.size();
		  }
		  break;
	  }
	  case WM_DROPFILES:
	  {
		  HDROP dropHandle = (HDROP)wParam;
		  auto numberOfDroppedFiles = DragQueryFileA(dropHandle, 0xFFFFFFFF, nullptr, 0);
		  if (numberOfDroppedFiles > 1) break;
		  auto droppedFilepathSize = DragQueryFileA(dropHandle, 0, nullptr, 0);
		  std::string droppedFilepath;
		  droppedFilepath.resize(droppedFilepathSize);
		  auto bytesCopied = DragQueryFileA(dropHandle, 0, &droppedFilepath[0], droppedFilepathSize + 1);
		  // @Note: this does not work for some reason with full path; the probelm seems to be the Win32 function
		  // CreateFile but I don't really care right now
		  // -> the problem has probably been the fact that the file is still opened and can't be open for a second time
		   LoadAtlas(context, droppedFilepath.c_str());
		  DragFinish(dropHandle);
		  break;

	  }

	}
}

static void ParseCommandLineArguments(int argc, char *argv[], AtlasViewer::CommandLineArguments& arguments)
{
	for (size_t i = 0; i < argc; ++i)
	{
		std::string current{argv[i]};
		if(current == "-r") {
			arguments.Root = argv[++i];
		} else if (current == "-i") {
			arguments.Input = argv[++i];
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{
	Memory::InitMemoryState();
	SetupConsole();
	gDxgiManager.Init();

	int argc;
	char** argv;
	argv = CommandLineToArgvA(GetCommandLine(), &argc);

	Context context;
	ParseCommandLineArguments(argc, argv, context.Args);

	WNDCLASSEX windowClass{ 0 };
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = HandleMsg;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(NULL);
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = "TextureViewerClass";
	windowClass.hIconSm = nullptr;
	RegisterClassEx(&windowClass);

	context.WindowStyle = WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	context.WindowRect.left = 100;
	context.WindowRect.right = 1080 + context.WindowRect.left;
	context.WindowRect.top = 100;
	context.WindowRect.bottom = 720 + context.WindowRect.top;

	auto hWnd = CreateWindow("TextureViewerClass", "Texture Viewer", context.WindowStyle,
							 CW_USEDEFAULT, CW_USEDEFAULT, context.WindowRect.right - context.WindowRect.left,
							 context.WindowRect.bottom - context.WindowRect.top,
							 nullptr, nullptr, GetModuleHandleA(NULL), &context);

	DragAcceptFiles(hWnd, TRUE);
	ShowWindow(hWnd, SW_SHOW);

	Init(context);

	while (true)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return (int)msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Update(context);
		context.Graphics.EndFrame();
	}

	return 0;
}
