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

#include "AtlasViewer.hpp"
#include "DummyTexture.hpp"

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

static void Init(Context& context)
{
	Memory::InitMemoryState();
	auto Graphics = &context.Graphics;
	auto path = fmt::format("{}/{}", context.Args.Root, context.Args.Input);

	MemoryArena fileArena = Memory::GetTempArena(Megabytes(128));
	Memory::EstablishTempScope(Megabytes(16));
	Defer {
		Memory::EndTempScope();
		Memory::DestoryTempArena(fileArena);
	};
	int width, height, channels;
	unsigned char* data = stbi_load_from_memory(DUMMYTEXTURE_PNG, DUMMYTEXTURE_PNG_LEN, &width, &height, &channels, 4);
	context.DummyTex = Graphics->CreateTexture(width, height, PngFormat(channels), data, width*height*channels);

	ReadWholeFile(path.c_str(), fileArena);
	auto header = (AtlasFileHeader*)(fileArena.Memory);	
	context.Texs.resize(header->NumAtlases);
	for (size_t i = 0; i < header->NumAtlases; ++i) 
	{
		auto atlas = (AtlasEntry*)(fileArena.Memory + sizeof(AtlasFileHeader) + sizeof(AtlasEntry) * i);
		context.Texs[i] = Graphics->CreateTexture(atlas->Width, atlas->Height, atlas->Format, fileArena.Memory + atlas->Offset, 0);
	}

}

void HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, Context* context)
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
		  if ( param == 37) {
			  context->currentTex = (context->currentTex - 1) % context->Texs.size();

		  } else if ( param == 39) {
			  context->currentTex = (context->currentTex + 1) % context->Texs.size();
		  }
		  
		  break;
	  }
	  
	}
}

static void ParseCommandLineArguments(int argc, char *argv[], CommandLineArguments& arguments)
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
