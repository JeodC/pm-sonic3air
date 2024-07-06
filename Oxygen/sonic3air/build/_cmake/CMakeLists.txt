cmake_minimum_required(VERSION 3.13)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(Sonic3AIR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Define workspace directory
set(WORKSPACE_DIR ../../../..)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# zlib sources as virtual subdirectory "zlib"
include_directories(${WORKSPACE_DIR}/framework/external/zlib/zlib)
add_subdirectory(${WORKSPACE_DIR}/framework/external/zlib/zlib zlib)

# Include directories
include_directories(${WORKSPACE_DIR}/framework/external/ogg-vorbis/libogg/include)
include_directories(${WORKSPACE_DIR}/framework/external/ogg-vorbis/libvorbis/include)
include_directories(${WORKSPACE_DIR}/framework/external/ogg-vorbis/libvorbis/lib)
include_directories(${WORKSPACE_DIR}/framework/external/zlib/zlib/contrib/minizip)
include_directories(build/zlib)  # Needed for zconf.h
include_directories(${WORKSPACE_DIR}/librmx/source)
include_directories(${WORKSPACE_DIR}/librmx/source/rmxmedia/_glew)
include_directories(${WORKSPACE_DIR}/Oxygen/lemonscript/source)
include_directories(${WORKSPACE_DIR}/Oxygen/oxygenengine/source)
include_directories(${WORKSPACE_DIR}/Oxygen/sonic3air/source)
include_directories(${WORKSPACE_DIR}/Oxygen/sonic3air/source/external)

# ogg vorbis
file(GLOB OGGVORBIS_SOURCES ${WORKSPACE_DIR}/framework/external/ogg-vorbis/libogg/src/*.c
                            ${WORKSPACE_DIR}/framework/external/ogg-vorbis/libvorbis/lib/*.c)
add_library(oggvorbis ${OGGVORBIS_SOURCES})

# minizip
file(GLOB MINIZIP_SOURCES ${WORKSPACE_DIR}/framework/external/zlib/zlib/contrib/minizip/ioapi.c
                          ${WORKSPACE_DIR}/framework/external/zlib/zlib/contrib/minizip/unzip.c
                          ${WORKSPACE_DIR}/framework/external/zlib/zlib/contrib/minizip/zip.c)
add_library(minizip STATIC ${MINIZIP_SOURCES})
target_link_libraries(minizip zlibstatic)

# rmxbase
file(GLOB_RECURSE RMXBASE_SOURCES ${WORKSPACE_DIR}/librmx/source/rmxbase/*.cpp)
add_library(rmxbase ${RMXBASE_SOURCES})
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(rmxbase PRIVATE ${WORKSPACE_DIR}/librmx/source/rmxbase.h)
endif()
target_link_libraries(rmxbase stdc++fs)
target_link_libraries(rmxbase zlibstatic)

# rmxmedia
file(GLOB_RECURSE RMXMEDIA_SOURCES ${WORKSPACE_DIR}/librmx/source/rmxmedia/*.cpp
                                   ${WORKSPACE_DIR}/librmx/source/rmxmedia/_glew/*.c)
add_library(rmxmedia ${RMXMEDIA_SOURCES})
target_link_libraries(rmxmedia rmxbase)
target_link_libraries(rmxmedia SDL2::SDL2)
target_link_libraries(rmxmedia OpenGL::GL)

# rmxext_oggvorbis
file(GLOB RMXEXT_OGGVORBIS_SOURCES ${WORKSPACE_DIR}/librmx/source/rmxext_oggvorbis/*.cpp)
add_library(rmxext_oggvorbis ${RMXEXT_OGGVORBIS_SOURCES})
target_link_libraries(rmxext_oggvorbis rmxmedia)
target_link_libraries(rmxext_oggvorbis oggvorbis)

# lemonscript
file(GLOB_RECURSE LEMONSCRIPT_SOURCES ${WORKSPACE_DIR}/Oxygen/lemonscript/source/lemon/*.cpp)
add_library(lemonscript ${LEMONSCRIPT_SOURCES})
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(lemonscript PRIVATE ${WORKSPACE_DIR}/Oxygen/lemonscript/source/lemon/pch.h)
endif()
target_link_libraries(lemonscript rmxbase)

# oxygen_netcore
file(GLOB_RECURSE OXYGEN_NETCORE_SOURCES ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/oxygen_netcore/*.cpp)
add_library(oxygen_netcore ${OXYGEN_NETCORE_SOURCES})
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(oxygen_netcore PRIVATE ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/oxygen_netcore/pch.h)
endif()
target_link_libraries(oxygen_netcore rmxbase)

# oxygenengine
file(GLOB_RECURSE OXYGEN_SOURCES ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/oxygen/*.cpp)
add_library(oxygen ${OXYGEN_SOURCES})
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(oxygen PRIVATE ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/oxygen/pch.h)
endif()
target_link_libraries(oxygen minizip)
target_link_libraries(oxygen rmxmedia)
target_link_libraries(oxygen rmxext_oggvorbis)
target_link_libraries(oxygen lemonscript)
target_link_libraries(oxygen oxygen_netcore)

if (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    target_link_libraries(oxygen atomic)  # Needed when building on Raspberry Pi
endif()

if (UNIX)
    find_package(CURL REQUIRED)
    target_link_libraries(oxygen CURL::libcurl)
endif()

# OxygenApp
file(GLOB_RECURSE ENGINEAPP_SOURCES ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/engineapp/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WORKSPACE_DIR}/oxygenengine")
add_executable(OxygenApp ${ENGINEAPP_SOURCES})
if (UNIX AND NOT APPLE)
    # Different executable name on Linux
    set_target_properties(OxygenApp PROPERTIES OUTPUT_NAME "oxygenapp_linux")
endif ()
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(OxygenApp PRIVATE ${WORKSPACE_DIR}/Oxygen/oxygenengine/source/engineapp/pch.h)
endif()
target_link_libraries(OxygenApp SDL2::SDL2 oxygen Threads::Threads)

# Sonic3AIR
file(GLOB_RECURSE SONIC3AIR_SOURCES ${WORKSPACE_DIR}/Oxygen/sonic3air/source/sonic3air/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WORKSPACE_DIR}/sonic3air")
add_executable(Sonic3AIR ${SONIC3AIR_SOURCES})
target_compile_definitions(Sonic3AIR PUBLIC ENDUSER)
if (NOT CMAKE_VERSION VERSION_LESS "3.16.0")
    target_precompile_headers(Sonic3AIR PRIVATE ${WORKSPACE_DIR}/Oxygen/sonic3air/source/sonic3air/pch.h)
endif()
target_link_libraries(Sonic3AIR SDL2::SDL2 oxygen Threads::Threads)
