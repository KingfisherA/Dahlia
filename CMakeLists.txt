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

get_filename_component(DH_DIR ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
set(DH_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${DH_PLATFORM} DH_PLATFORM)
set(DH_SRC_DIR ${DH_DIR}/src)
set(DH_BIN_DIR ${DH_DIR}/bin/${DH_PLATFORM})
set(DH_LIB_DIR ${DH_DIR}/Libraries)

add_subdirectory(${DH_DIR}/Libraries/ImGui_Build)
set(IMGUI_DIR ${DH_LIB_DIR}/ImGui)

find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(GLEW_USE_STATIC_LIBS true)
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
include(Projects/Dahlia.cmake)
include(Projects/DahliaInfo.cmake)

