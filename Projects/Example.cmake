add_executable("Example"
  "${GENG_SRC_DIR}/Example/Application.cpp"
  "${GENG_SRC_DIR}/Example/Application.h"
)

add_dependencies("Example"
    "ImGui"
    "GraphicsEngine"
    "GraphicsEngineTools"
)
target_include_directories("Example" PRIVATE
    ${GENG_SRC_DIR}
    ${GENG_SRC_DIR}/Example
    ${GENG_LIB_DIR}
    ${IMGUI_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)
target_compile_definitions("Example" PRIVATE
  GLM_ENABLE_EXPERIMENTAL
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("Example" PRIVATE
  GraphicsEngine
  ImGui
  GraphicsEngineTools
)

set(COMPILE_OPTIONS)

if((CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR 
   (CMAKE_CXX_COMPILER_ID STREQUAL GNU))
    list(APPEND COMPILE_OPTIONS
      -m64
      -fPIC
      -std=c++20
    )

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
      list(APPEND COMPILE_OPTIONS
        -g
        -O0
      )
    elseif(CMAKE_BUILD_TYPE STREQUAL Release)
      list(APPEND COMPILE_OPTIONS
        -O3
      )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  list(APPEND COMPILE_OPTIONS
      /std:c++20
    )
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
      list(APPEND COMPILE_OPTIONS
        /Zi
        /Od
      )
    elseif(CMAKE_BUILD_TYPE STREQUAL Release)
      list(APPEND COMPILE_OPTIONS /O2)
    endif()
endif()

target_compile_options("Example" PRIVATE ${COMPILE_OPTIONS})

set_target_properties("Example" PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  POSITION_INDEPENDENT_CODE True
  INTERPROCEDURAL_OPTIMIZATION False
  MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
  OUTPUT_NAME "Example")

if(CMAKE_CONFIGURATION_TYPES)
  set(DEBUG_BIN_DIR "${GENG_BIN_DIR}")
  set(RELEASE_BIN_DIR "${GENG_BIN_DIR}")
else()
  set(DEBUG_BIN_DIR "${GENG_BIN_DIR}/Debug")
  set(RELEASE_BIN_DIR "${GENG_BIN_DIR}/Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Example" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
  )
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Example" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
  )
endif()
