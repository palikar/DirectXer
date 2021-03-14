#include "PlatformWindows.hpp"
#include "Memory.hpp"
#include "App.hpp"

static void ParseCommandLineArguments(CommandLineSettings& t_Settings, char** argv, int argc)
{
	
	for (size_t i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--resources") == 0)
		{
			DXLOG("[INIT] Argument: {} -> {}", argv[i], argv[i + 1]);
			t_Settings.ResourcesPath = argv[i + 1];
			++i;
		}

		
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{
	SetupConsole();
	gInput.Init();
	gDxgiManager.Init();
	Memory::InitMemoryState();

	App application;

	char** argv;
	int argc;
	argv = CommandLineToArgvA(GetCommandLine(), &argc);
	ParseCommandLineArguments(application.Arguments, argv, argc);

	WindowsSettings settings{0};
	settings.ClassName = "DirectXClass";
	settings.WindowTitle = "DirectXer";
	settings.InitialWidth = 1080;
	settings.InitialHeight = 720;
	settings.Fullscreen = 0;

	WindowsWindow window;
	window.Application = &application;

	size_t InitialStackMemory{0};
	InitialStackMemory += sizeof(App);
	InitialStackMemory += sizeof(WindowsWindow);
	InitialStackMemory += sizeof(WindowsSettings);
	InitialStackMemory += sizeof(int);
	InitialStackMemory += sizeof(char**);
	DXLOG("[INIT] Initial Stack Memory: {:.3} kB ", InitialStackMemory/1024.0f);
	
	window.Init(settings);
	window.Run();

	return 0;
}

// @Memory magament
// @Done: Allcating some amount of memory upfront
// @Done: Pool temporary allocation strategy
// @Todo: Buffering the logging output
// @Todo: Temporary Vector and Map
// @Todo: Have a separate palce for "debug" memory

// @Done: Ability to transform vertices on the CPU
// @Done: Proper Vertex struct format
// @Done: Transformation per vertex type
// @Done: Robing hood map from the github repo
// @Done: Basic setup for command line arguments passig

// @Graphics
// @Done: Loading textures from file
// @Done: Creating textures in the graphics
// @Done: Rasterization state types setup
// @Done: Shader types setup
// -> Shader files(switching the actual shaders), Shader types(value in a CB)
// -> Debug Shader -- can draw with color or texture mapped (to test materials and stuff)
// @Done: Input layout tpyes setup
// -- debug layout (pos, color, uv, normals, tangents, bitangents)
// -- standard layout (pos, uv, normals, tangents, bitangents)
// @Done: Constant buffers setup
// @Done: Fix the prim CBs ids place
// @Todo: Blending State Setup
// @Todo: Depth Stencil State Setup
// @Todo: Scissor test support
// @Todo: Render target support

// @Done: Lines "geometry" + ability to use lines primiteves
// @Done: Cylinsder geometry
// @Done: Cone geometry
// @Done: Torus geometry

// @Rendering
// @Done: Rendering sky box
// @Todo: Rendering groups
// @Todo: Rendering fog
// @Done: Material setup
// @Done: Phong material
// @Done: Sollid color material
// @Todo: PBR material
// @Done: Point light
// @Todo: Text rendering

// @Data management
// @Done: Container for geometry info!
// @Done: Color vertex type -- hence solid color material (ish)
// @Todo: Loading obj files
// @Todo: Mouse picking
// @Done: Texture catalog but a good one, not holding textures in memory all the time
// @Todo: Tags for the textures -- for certain level, for certain material instance
//     -- load all textures in a level
//     -- get textures for some material instance

// @Debug
// @Done: Axis helper
// @Done: Grid Helper
// @Done: Point light helper
// @Done: FPS Camara controller
// @Done: Adjust all of the geometry generators
// @Done: Add the normals and uvs to the color vertex
// @Done: Spot light helper
// @Todo: Drop down console

// @Architecture
// @Done: Resizing
// @Done: FPS independence
// @Done: Windows Abstraction Layer
// @Done: Handle Minimization
// @Todo: Some job\task system for basic multithreading support
//    -- maybe this is only useful for loading resources
// @Todo: Deffered loading of scenes resources
// @Todo: Memory tracking -- temp; bulk; GPU memory; general
// purpose allocations
//    -- Have some global tracking structs in Memory.hpp and track
//the memory consumption in Debug


// @Notes
// @Note: In a scene, some things are dynamic, and some things are static


