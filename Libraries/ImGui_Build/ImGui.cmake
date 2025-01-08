add_library("ImGui"
    "${IMGUI_SRC_DIR}/imgui.cpp"
    "${IMGUI_SRC_DIR}/imgui_demo.cpp"
    "${IMGUI_SRC_DIR}/imgui_draw.cpp"
    "${IMGUI_SRC_DIR}/imgui_tables.cpp"
    "${IMGUI_SRC_DIR}/imgui_widgets.cpp"
)


target_include_directories("ImGui" PRIVATE
  ${IMGUI_SRC_DIR}
)

target_compile_options("ImGui" PRIVATE
    -m64
    -fPIC
    -std=c++20
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImGui" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "ImGui"
    ARCHIVE_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Debug-x64"
    LIBRARY_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Debug-x64"
    RUNTIME_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Debug-x64"
  )

  target_compile_options("ImGui" PRIVATE
    -g
    -O0
  )
endif()


if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImGui" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "ImGui"
    ARCHIVE_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Release-x64"
    LIBRARY_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Release-x64"
    RUNTIME_OUTPUT_DIRECTORY "${IMGUI_BIN_DIR}/Release-x64"
  )

  target_compile_options("ImGui" PRIVATE
      -mavx2
      -O2
  )
endif()

