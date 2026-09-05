cmake_minimum_required(VERSION 3.24)

# ---- Static linking (required for SKSE plugins) ----
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

project(VrikInventorySelfie VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

# Configure build options for CommonLibSSE-NG
# This plugin talks to VRIK and reads the player's HMD node, so it is VR-only,
# exactly like PhysicalCollision.
set(ENABLE_SKYRIM_SE OFF CACHE BOOL "Enable support for Skyrim Special Edition" FORCE)
set(ENABLE_SKYRIM_AE OFF CACHE BOOL "Enable support for Skyrim Anniversary Edition" FORCE)
set(ENABLE_SKYRIM_VR ON CACHE BOOL "Enable support for Skyrim VR" FORCE)

find_package(CommonLibSSE REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME}
    SOURCES
        src/Config.cpp
        src/Config.h
        src/Main.cpp
        src/MenuEventHandler.cpp
        src/MenuEventHandler.h
        src/vrikinterface001.cpp
        src/vrikinterface001.h
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/PCH.h
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP              # Multi-processor compilation
        /utf-8           # Set source and execution character sets to UTF-8
        /Zc:preprocessor # Enable conformance mode preprocessor
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)
