#  Build Script for Dahlia Graphics Engine

cmake_minimum_required(VERSION 3.16)

set(DH_BUILD_TYPES "Debug" "Release")
if(CMAKE_CONFIGURATION_TYPES)
#  set(CMAKE_CONFIGURATION_TYPES DH_BUILD_TYPES)
  message(NOTICE "---Multiconfiguration build generator selected---")
elseif(NOT CMAKE_BUILD_TYPE IN_LIST DH_BUILD_TYPES)
  message(FATAL_ERROR
    "Invalid build type '${CMAKE_BUILD_TYPE}'.
    CMAKE_BUILD_TYPE must be any one of the possible values:
    ${DH_BUILD_TYPES}"
  )
endif()

if(NOT DH_INCLUDE_IMGUI)
    set(DH_INCLUDE_IMGUI true)
endif()

project("Dahlia" LANGUAGES CXX)

# The demo/info app is only built by default when Dahlia is the top level project, so another
# project can add_subdirectory(Dahlia) and get just the library. The target architecture comes
# from CMAKE_OSX_ARCHITECTURES (or the host) rather than being hard-coded.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(DH_IS_TOP_LEVEL ON)
else()
  set(DH_IS_TOP_LEVEL OFF)
endif()
option(DH_BUILD_INFO_APP "Build the DahliaInfo demo application" ${DH_IS_TOP_LEVEL})

get_filename_component(DH_DIR ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
set(DH_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${DH_PLATFORM} DH_PLATFORM)
set(DH_SRC_DIR ${DH_DIR}/src)
set(DH_BIN_DIR ${DH_DIR}/bin/${DH_PLATFORM})
set(DH_LIB_DIR ${DH_DIR}/Libraries)

add_subdirectory(${DH_DIR}/Libraries/ImGui_Build)
set(IMGUI_DIR ${DH_LIB_DIR}/ImGui)

# SPIRV-Cross: translates SPIR-V shaders to GLSL 410 (OpenGL) and MSL (Metal) at runtime.
# Built static and position independent so it can be linked into the Dahlia shared library.
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(${DH_LIB_DIR}/SPIRV-Cross EXCLUDE_FROM_ALL)

find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

#set(GLEW_USE_STATIC_LIBS true)
find_package(GLEW CONFIG)
if(NOT GLEW_FOUND)
    find_package(GLEW REQUIRED)
    message(NOTICE "Found GLEW with Find.cmake")
endif()

find_package(Freetype CONFIG)
if(NOT ${Freetype_FOUND})
    find_package(Freetype REQUIRED)
    message(NOTICE "Found Freetype with Find.cmake")
endif()

include(Projects/DahliaTools.cmake)
include(Projects/DahliaShaders.cmake)
include(Projects/Dahlia.cmake)
if(DH_BUILD_INFO_APP)
  include(Projects/DahliaInfo.cmake)
endif()

