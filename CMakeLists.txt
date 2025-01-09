#  Build Script for GraphicsEngine

cmake_minimum_required(VERSION 3.16)

set(GENG_BUILD_TYPES "Debug" "Release")
if(CMAKE_CONFIGURATION_TYPES)
#  set(CMAKE_CONFIGURATION_TYPES GENG_BUILD_TYPES)
  message(NOTICE "---Multiconfiguration build generator selected---")
elseif(NOT CMAKE_BUILD_TYPE IN_LIST GENG_BUILD_TYPES)
  message(FATAL_ERROR
    "Invalid build type '${CMAKE_BUILD_TYPE}'.
    CMAKE_BUILD_TYPE must be any one of the possible values:
    ${GENG_BUILD_TYPES}"
  )
endif()

if(NOT GENG_INCLUDE_IMGUI)
    set(GENG_INCLUDE_IMGUI true)
endif()

project("GraphicsEngine"
        LANGUAGES CXX)

get_filename_component(GENG_DIR ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
set(GENG_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${GENG_PLATFORM} GENG_PLATFORM)
set(GENG_SRC_DIR ${GENG_DIR}/src)
set(GENG_BIN_DIR ${GENG_DIR}/bin/${GENG_PLATFORM})
set(GENG_LIB_DIR ${GENG_DIR}/Libraries)

add_subdirectory(${GENG_DIR}/Libraries/ImGui_Build)
set(IMGUI_DIR ${GENG_LIB_DIR}/ImGui)

find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

if(MSVC)
    set(GLEW_USE_STATIC_LIBS false)
else()
    set(GLEW_USE_STATIC_LIBS true)
endif()

find_package(GLEW CONFIG)
if(NOT ${}GLEW_FOUND})
    find_package(GLEW REQUIRED)
endif()

find_package(Freetype CONFIG)
if(NOT ${}Freetype_FOUND})
    find_package(Freetype REQUIRED)
endif()

include(Projects/cmake/GraphicsEngine.cmake)  
include(Projects/cmake/Example.cmake)

