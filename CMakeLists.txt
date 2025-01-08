#  Build Script for GraphicsEngine

cmake_minimum_required(VERSION 3.16)

set(GENG_BUILD_TYPES "Debug" "Release")
if(NOT CMAKE_BUILD_TYPE IN_LIST GENG_BUILD_TYPES)
  message(FATAL_ERROR
    "Invalid build type '${CMAKE_BUILD_TYPE}'.
    CMAKE_BUILD_TYPE must be any one of the possible values:
    ${GENG_BUILD_TYPES}"
  )
endif()

if(NOT CMAKE_SYSTEM_NAME)
  set(CMAKE_SYSTEM_NAME "Linux")
endif()
if(NOT GENG_INCLUDE_IMGUI)
    set(GENG_INCLUDE_IMGUI true)
endif()

get_filename_component(GENG_DIR ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
set(GENG_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${GENG_PLATFORM} GENG_PLATFORM)
set(GENG_SRC_DIR ${GENG_DIR}/src)
set(GENG_BIN_DIR ${GENG_DIR}/bin/${GENG_PLATFORM})

add_subdirectory(${GENG_DIR}/Libraries/ImGui_Build)

set(CMAKE_PREFIX_PATH "/usr/local")

project("GraphicsEngine"
        LANGUAGES CXX)

find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
set(GLEW_USE_STATIC_LIBS true)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

include(Projects/cmake/GraphicsEngine.cmake)  
include(Projects/cmake/Example.cmake)

