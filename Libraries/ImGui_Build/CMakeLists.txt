# ImGui Custom Build Script
cmake_minimum_required(VERSION 3.16)

set(GENG_BUILD_TYPES "Debug" "Release")
if(NOT CMAKE_BUILD_TYPE IN_LIST GENG_BUILD_TYPES)
  message(FATAL_ERROR
    "Invalid build type '${CMAKE_BUILD_TYPE}'.
    CMAKE_BUILD_TYPE must be any one of the possible values:
    ${GENG_BUILD_TYPES}"
  )
endif()

project(ImGui LANGUAGES CXX)

get_filename_component(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ImGui ABSOLUTE)
set(IMGUI_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOLOWER ${IMGUI_PLATFORM} IMGUI_PLATFORM)
set(IMGUI_SRC_DIR ${IMGUI_DIR})
set(IMGUI_BIN_DIR ${IMGUI_DIR}/bin/${IMGUI_PLATFORM})

include(ImGui.cmake)
