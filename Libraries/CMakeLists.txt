# Third party libraries vendored as git submodules

foreach(FILE ImGui/imgui.h SPIRV-Cross/CMakeLists.txt)
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
    message(FATAL_ERROR
      "Libraries/${FILE} is missing. Run: git submodule update --init --recursive")
  endif()
endforeach()

# ---- Dear ImGui (docking branch) ------------------------------------------------------------------
# Only the core; the platform/renderer backends are compiled into Dahlia (src/Dahlia/GUI/ImGuiBuild*).
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGui)
add_library(ImGui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
)
# <ImGui/imgui.h> for Dahlia code, "imgui.h" / "backends/..." for ImGui's own sources
target_include_directories(ImGui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${IMGUI_DIR}
)
dahlia_target_defaults(ImGui)

# ---- SPIRV-Cross ----------------------------------------------------------------------------------
# Translates SPIR-V shaders to GLSL 410 (OpenGL) and MSL (Metal) at runtime.
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
add_subdirectory(SPIRV-Cross EXCLUDE_FROM_ALL)
