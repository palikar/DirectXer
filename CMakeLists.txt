cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(DirectXer LANGUAGES CXX)

enable_language(C)

add_library(project_warnings INTERFACE)
add_library(project_options INTERFACE)
add_library(project_defines INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_17)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options (project_options INTERFACE -fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (project_options INTERFACE -fcolor-diagnostics)
    endif ()
endif ()


# target_compile_options(project_warnings
#     INTERFACE
#     -Wall
#     -Wextra
#     -Wshadow
#     )

# option(ALL_WARNINGS "Enable all waring that the compiler supports" FALSE)
# if(${ALL_WARNINGS})
#     target_compile_options(project_warnings
#         INTERFACE
#         -Wnon-virtual-dtor
#         -Wold-style-cast
#         -Wcast-align
#         -Wunused
#         -Woverloaded-virtual
#         -Wpedantic
#         -Wconversion
#         -Wsign-conversion
#         -Wnull-dereference
#         -Wdouble-promotion
#         -Wformat=2)
# endif()

# option(LIBCPP "Build with libc++" FALSE)
# if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND ${ALL_WARNINGS})
#     if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.3)
#         target_compile_options(project_warnings INTERFACE
#             -Wduplicated-branches)
#     endif()
#     target_compile_options(project_warnings INTERFACE
#         -Wmisleading-indentation
#         -Wduplicated-cond
#         -Wlogical-op
#         -Wuseless-cast)
#     target_link_libraries(project_options INTERFACE stdc++fs)
# else()
#     if(LIBCPP)
#         target_compile_options(project_options INTERFACE -stdlib=libc++)
#     else()
#         target_link_libraries(project_options INTERFACE stdc++fs)
#     endif()
# endif()

target_link_libraries(project_options INTERFACE -fno-exceptions)
target_include_directories(project_options INTERFACE ./DirectXer/src)

target_include_directories(project_options INTERFACE ThirdParty/Glm)
target_include_directories(project_options INTERFACE ThirdParty/RobinHood)
target_include_directories(project_options INTERFACE ThirdParty/Stb/include)


add_library(Stb ThirdParty/Stb/src/stb.cpp)
target_include_directories(Stb PUBLIC ThirdParty/Stb/include)

add_library(Fmt ThirdParty/Fmt/src/format.cc ThirdParty/Fmt/src/os.cc)
target_include_directories(Fmt PUBLIC ThirdParty/Fmt/include)

add_library(FreeType
    ThirdParty/Freetype/src/autofit/autofit.c
    ThirdParty/Freetype/src/base/ftbase.c
    ThirdParty/Freetype/src/base/ftbbox.c
    ThirdParty/Freetype/src/base/ftbdf.c
    ThirdParty/Freetype/src/base/ftbitmap.c
    ThirdParty/Freetype/src/base/ftcid.c
    ThirdParty/Freetype/src/base/ftfstype.c
    ThirdParty/Freetype/src/base/ftgasp.c
    ThirdParty/Freetype/src/base/ftglyph.c
    ThirdParty/Freetype/src/base/ftgxval.c
    ThirdParty/Freetype/src/base/ftinit.c
    ThirdParty/Freetype/src/base/ftmm.c
    ThirdParty/Freetype/src/base/ftotval.c
    ThirdParty/Freetype/src/base/ftpatent.c
    ThirdParty/Freetype/src/base/ftpfr.c
    ThirdParty/Freetype/src/base/ftstroke.c
    ThirdParty/Freetype/src/base/ftsynth.c
    ThirdParty/Freetype/src/base/ftsystem.c
    ThirdParty/Freetype/src/base/fttype1.c
    ThirdParty/Freetype/src/base/ftwinfnt.c
    ThirdParty/Freetype/src/bdf/bdf.c
    ThirdParty/Freetype/src/cache/ftcache.c
    ThirdParty/Freetype/src/cff/cff.c
    ThirdParty/Freetype/src/cid/type1cid.c
    ThirdParty/Freetype/src/gzip/ftgzip.c
    ThirdParty/Freetype/src/lzw/ftlzw.c
    ThirdParty/Freetype/src/pcf/pcf.c
    ThirdParty/Freetype/src/pfr/pfr.c
    ThirdParty/Freetype/src/psaux/psaux.c
    ThirdParty/Freetype/src/pshinter/pshinter.c
    ThirdParty/Freetype/src/psnames/psmodule.c
    ThirdParty/Freetype/src/raster/raster.c
    ThirdParty/Freetype/src/sfnt/sfnt.c
    ThirdParty/Freetype/src/smooth/smooth.c
    ThirdParty/Freetype/src/truetype/truetype.c
    ThirdParty/Freetype/src/type1/type1.c
    ThirdParty/Freetype/src/type42/type42.c
    ThirdParty/Freetype/src/winfonts/winfnt.c
    ThirdParty/Freetype/src/ftdebug.c
    )

target_compile_definitions(FreeType PRIVATE
    FT2_BUILD_LIBRARY
    DLL_EXPORT
    _LIB
    _CRT_SECURE_NO_WARNINGS
    )
target_include_directories(FreeType PUBLIC ThirdParty/Freetype/include)

add_subdirectory(ThirdParty/openal-soft-1.21.1)

add_library(Optik
    ThirdParty/Optik/src/optick_core.cpp
    ThirdParty/Optik/src/optick_server.cpp
    ThirdParty/Optik/src/optick_capi.cpp
    ThirdParty/Optik/src/optick_serialization.cpp
    ThirdParty/Optik/src/optick_message.cpp
    ThirdParty/Optik/src/optick_gpu.cpp
    # ThirdParty/Optik/src/optick_gpu.vulkan.cpp
    ThirdParty/Optik/src/optick_miniz.cpp
    )
target_include_directories(Optik PUBLIC ThirdParty/Optik/include)


add_executable(SpaceGame    
    ./DirectXer/src/PlatformLinux/LinuxMain.cpp
    ./DirectXer/src/PlatformLinux/PlatformLinux.cpp
    )

target_link_libraries(SpaceGame
    PUBLIC project_options project_warnings project_defines
    Fmt Stb FreeType Optik OpenAL
    )
