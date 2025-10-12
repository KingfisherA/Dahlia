add_executable("DahliaInfo"
  "${DH_SRC_DIR}/DahliaInfo/Application.cpp"
  "${DH_SRC_DIR}/DahliaInfo/Application.h"
)

add_dependencies("DahliaInfo"
    "ImGui"
    "Dahlia"
    "DahliaTools"
)
target_include_directories("DahliaInfo" PRIVATE
    ${DH_SRC_DIR}
    ${DH_SRC_DIR}/DahliaInfo
    ${DH_LIB_DIR}
    ${IMGUI_DIR}
    ${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)
target_compile_definitions("DahliaInfo" PRIVATE
  GLM_ENABLE_EXPERIMENTAL
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("DahliaInfo" PRIVATE
  Dahlia
  ImGui
  DahliaTools
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

target_compile_options("DahliaInfo" PRIVATE ${COMPILE_OPTIONS})

set_target_properties("DahliaInfo" PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  POSITION_INDEPENDENT_CODE True
  INTERPROCEDURAL_OPTIMIZATION False
  MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
  OUTPUT_NAME "DahliaInfo")

if(CMAKE_CONFIGURATION_TYPES)
  set(DEBUG_BIN_DIR "${DH_BIN_DIR}")
  set(RELEASE_BIN_DIR "${DH_BIN_DIR}")
else()
  set(DEBUG_BIN_DIR "${DH_BIN_DIR}/Debug")
  set(RELEASE_BIN_DIR "${DH_BIN_DIR}/Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("DahliaInfo" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
  )
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("DahliaInfo" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
  )
endif()
