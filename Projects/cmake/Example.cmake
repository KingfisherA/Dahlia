add_executable("Example"
  "${GENG_SRC_DIR}/Example/Application.cpp"
  "${GENG_SRC_DIR}/Example/Application.h"
)

add_dependencies("Example"
    "GraphicsEngine"
)
target_include_directories("Example" PRIVATE
    ${GENG_SRC_DIR}
    ${GENG_SRC_DIR}/Example
    /media/charlie/Data/dev/Libraries/ImGui
    /media/charlie/Data/dev/Libraries
    ${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    /usr/include/libpng16
)
target_compile_definitions("Example" PRIVATE#
  GENG_STATIC
  OS_LINUX
  GENG_BUILD
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("Example" PRIVATE
  GraphicsEngine
)
target_compile_options("Example" PRIVATE
  -m64
  -fPIC
  -std=c++20
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Example" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "Example"
    ARCHIVE_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
    LIBRARY_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
    RUNTIME_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
  )
  target_compile_options("Example" PRIVATE
    -g
    -O0
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Example" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "Example"
    ARCHIVE_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
    LIBRARY_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
    RUNTIME_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
  )
  target_compile_options("Example" PRIVATE
    -O2
  )
endif()
