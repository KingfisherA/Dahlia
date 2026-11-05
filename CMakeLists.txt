# Build script for the Dahlia graphics engine
#
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build
#
# Outputs: build/Dahlia (libDahlia, libDahliaTools) and build/DahliaInfo (DahliaInfo,
# and DahliaInfo.app on macOS via the DahliaInfoApp target).

# 3.24 for $<LINK_LIBRARY:FRAMEWORK,...>. Newer policies (up to 4.0) are used when available, e.g.
# CMP0156 stops static libraries being repeated on the link line (ld "duplicate libraries" warning).
cmake_minimum_required(VERSION 3.24...4.0)

project(Dahlia VERSION 0.1.0 LANGUAGES C CXX)
if(APPLE)
  # The Metal backend has Objective-C++ (.mm) sources
  enable_language(OBJCXX)
endif()

# ---- Build type -----------------------------------------------------------------------------------
get_property(DH_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set(DH_BUILD_TYPES Debug Release)
if(DH_MULTI_CONFIG)
  set(CMAKE_CONFIGURATION_TYPES ${DH_BUILD_TYPES} CACHE STRING "" FORCE)
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${DH_BUILD_TYPES})
  if(NOT CMAKE_BUILD_TYPE IN_LIST DH_BUILD_TYPES)
    message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of: ${DH_BUILD_TYPES} (got '${CMAKE_BUILD_TYPE}')")
  endif()
endif()

# ---- Options --------------------------------------------------------------------------------------
# The demo app is only built by default when Dahlia is the top level project, so another project can
# add_subdirectory(Dahlia) and get just the library.
option(DH_BUILD_INFO_APP "Build the DahliaInfo demo application" ${PROJECT_IS_TOP_LEVEL})

find_package(Vulkan QUIET)
option(DH_ENABLE_VULKAN "Build the Vulkan backend (needs Vulkan 1.3)" ${Vulkan_FOUND})
option(DH_ENABLE_METAL "Build the Metal backend (Apple only)" ${APPLE})
set(DH_METAL_CPP_DIR "/opt/metal-cpp_macOS26_iOS26-beta" CACHE PATH "metal-cpp headers")

if(DH_ENABLE_VULKAN AND NOT Vulkan_FOUND)
  message(FATAL_ERROR "DH_ENABLE_VULKAN is ON but no Vulkan SDK/loader was found")
endif()
if(DH_ENABLE_METAL AND NOT APPLE)
  message(FATAL_ERROR "The Metal backend can only be built on Apple platforms")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Third party static libraries are linked into the Dahlia shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(DahliaHelpers)
include(DahliaShaders)

# ---- Dependencies ---------------------------------------------------------------------------------
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG QUIET)
if(NOT GLEW_FOUND)
  find_package(GLEW REQUIRED)
endif()
# Homebrew's GLEW package links OpenGL.framework by absolute path inside the SDK it was built with.
# That adds the SDK as a non-system -F path (so SDK headers emit warnings) and can mix two SDKs,
# so link the OpenGL found for this build instead.
if(APPLE)
  foreach(GLEW_TARGET GLEW::GLEW GLEW::glew GLEW::glew_s)
    if(TARGET ${GLEW_TARGET})
      set_property(TARGET ${GLEW_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES OpenGL::GL)
    endif()
  endforeach()
endif()
find_package(Freetype CONFIG QUIET)
if(NOT Freetype_FOUND)
  find_package(Freetype REQUIRED)
endif()

# Git submodules: ImGui and SPIRV-Cross
add_subdirectory(Libraries)

# ---- Targets --------------------------------------------------------------------------------------
add_subdirectory(src/Dahlia Dahlia)
if(DH_BUILD_INFO_APP)
  add_subdirectory(src/DahliaInfo DahliaInfo)
endif()
