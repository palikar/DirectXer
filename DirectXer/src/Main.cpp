#include <Memory.hpp>
#include <Resources.hpp>
#include <App.hpp>
#include <Random.hpp>
#include <Logging.hpp>
#include <Audio.hpp>
#include <Timing.hpp>

static void ParseCommandLineArguments(CommandLineSettings& t_Settings, char** argv, int argc)
{
	for (size_t i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--resources") == 0 || strcmp(argv[i], "-r") == 0)
		{
			DXDEBUG("[Init] Argument: {} -> {}", argv[i], argv[i + 1]);
			t_Settings.ResourcesPath = argv[i + 1];
			++i;
		}		
	}
}

// @Note: This is not the true main funtion; this will be called from the platform
// specific main function (WinMain or main); the point of this functions is to initalize
// all subsystems and create the appclication object will be used by the platfrom layer
App* InitMain(char** argv, int argc)
{
    OPTICK_EVENT();

    // @Note: Initalize every subsystem here
    Input::Init();
    Memory::InitMemoryState();
    PlatformLayer::Init();
    Random::Init();
    Audio::Init();

    // @Todo: The telemetry system has to be initilzed only in profile builds
    Telemetry::Init();

    DXDEBUG("[Init] Application size {:.3} KB", sizeof(App) / 1024.0f);
    App* application = Memory::BulkGetType<App>();

    // @Note: Ideally we won't this be we have some things in the games
    // that have default values that will be set only if the "root object" is
    // initialized
    new(application) App();
	
    ParseCommandLineArguments(application->Arguments, argv, argc);

    // @Note: Handl any command line arguemnt that are passed; ieally, we won't take
    // any command line arguments will load the settings from some file on the diska
    if (application->Arguments.ResourcesPath.empty())
    {
        DXERROR("[INIT] The given resource path is empty. Please provide a sensible one.");
        assert(false);
    }

    Resources::Init(application->Arguments.ResourcesPath);
	return application;
}


// == Memory magament ==
// @Done: Allcating some amount of memory upfront
// @Done: Pool temporary allocation strategy
// @Done: Temporary Vector and Map
// @Done: Platform independent logging
// @Todo: Have a separate place for "debug" memory

// @Done: Ability to transform vertices on the CPU
// @Done: Proper Vertex struct format
// @Done: Transformation per vertex type
// @Done: Robing hood map from the github repo
// @Done: Basic setup for command line arguments passig

// == Graphics ==
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
// @Done: Blending State Setup
// @Done: Depth Stencil State Setup
// @Done: Render target support
// @Done: Scissor test support
// @Done: Debug: Print out the current feature level\API
// @Done: Draw
// @Done: Memory tracking for common objects
// @Done: Debug marksers
// @Done: Timing queries
// @Done: Destroying objects
// @Done: DrawInstanced
// @Done: Debug names
// @Done: Release the things that are not needed


// @Done: Lines "geometry" + ability to use lines primiteves
// @Done: Cylinsder geometry
// @Done: Cone geometry
// @Done: Torus geometry

// == Rendering ==
// @Done: Rendering sky box
// @Done: Material setup
// @Done: Phong material
// @Done: Sollid color material
// @Done: Point light
// @Done: Text rendering
// @Todo: PBR material
// @Todo: Rendering groups
// @Todo: Rendering fog
// @Todo: Post Processing setup

// @Todo: Basic UI rendering using the Renderer2D
// -> Button
// -> Checkbox
// -> Slider
// -> Textfield

// == Data management ==
// @Done: Container for geometry info!
// @Done: Color vertex type -- hence solid color material (ish)
// @Done: Texture catalog but a good one, not holding textures in memory all the time
// @Done: Loading obj files
// @Todo: Mouse picking

// === theses are probably part of some bigger setup to stream resources asynchronously ===
// @Todo: Tags for the textures -- for certain level, for certain material instance
// @Todo: Deffered loading of scenes resources
//     -- load all textures in a level
//     -- get textures for some material instance

// == Debug ==
// @Done: Axis helper
// @Done: Grid Helper
// @Done: Point light helper
// @Done: FPS Camara controller
// @Done: Adjust alrl of the geometry generators
// @Done: Add the normals and uvs to the color vertex
// @Done: Spot light helper
// @Todo: Drop down console
 
// == Architecture ==
// @Done: Resizing
// @Done: FPS independence
// @Done: Windows Abstraction Layer
// @Done: Handle Minimization
// @Done: Image merger -- take sevaral pngs, put them in a sprite sheet
// @Done: 3D renderer API
// @Todo: Some job\task system for basic multithreading support
//    -- maybe this is only useful for loading resources
//    -- Defer certain things "for later"
// @Done: Memory tracking -- temp; bulk; GPU memory; general
// purpose allocations
//    -- Have some global tracking structs in Memory.hpp and track
//the memory consumption in Debug
// @Todo: Input Abstraction -- left, right, actionXXX, axis x/y
// @Todo: 2D Collision detection with something like GJK Algorithm
// @Todo: Inter image based animator setup

// == Telemtry ==
// @Done: Current Bulk
// @Done: GPU memory tracking
// @Done: GPU timing tracking
// @Done: Bulk Memory tracking
// @Done: Allocation tracking
// @Done: Tagged bulk allocation
// @Done: Tracking support for the map
// @Done: Displaying the used GPU resources in ImGui
// @Todo: Temorary memory tracking(?)

// == Memory ==
// @Todo Allocating general arenas in BulkStorage

// == Tool Ideas ==
// @Todo: Packing meshes into a single GPU buffer
// @Todo: Packing sprites
// @Todo: Packing bounding boxes (AABBs probably) for the meshes
// @Todo: Mesh viewre

// ***** Queue *****
//  <--- this one
