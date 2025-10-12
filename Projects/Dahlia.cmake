add_library("Dahlia" SHARED
  "${DH_SRC_DIR}/Dahlia/3D/Buffers.cpp"
  "${DH_SRC_DIR}/Dahlia/3D/Framebuffer.cpp"
  "${DH_SRC_DIR}/Dahlia/3D/ShaderProgram.cpp"
  "${DH_SRC_DIR}/Dahlia/3D/Texture.cpp"
  "${DH_SRC_DIR}/Dahlia/3D/VertexArray.cpp"
  "${DH_SRC_DIR}/Dahlia/3D/stb/stb_image.cpp"
  "${DH_SRC_DIR}/Dahlia/Application/GraphicsApp.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/Display.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/ImGuiBuild.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/ImGuiLayer.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/Text.cpp"
  "${DH_SRC_DIR}/Dahlia/Layers/Layer.cpp"
  "${DH_SRC_DIR}/Dahlia/Layers/LayerStack.cpp"
  "${DH_SRC_DIR}/Dahlia/Maths/Geometry.cpp"
  "${DH_SRC_DIR}/Dahlia/Maths/Maths.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/GLFW/GLFWDisplayBase.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/GPUContext.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLBuffers.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLContext.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLDisplay.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLFramebuffer.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLShaderProgram.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLTexture.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLVertexArray.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/Windows/Console.cpp"
  "${DH_SRC_DIR}/Dahlia/Resources/FontsRes.cpp"
  "${DH_SRC_DIR}/Dahlia/Resources/ShaderSources.cpp"
  "${DH_SRC_DIR}/Dahlia/Scene/Camera2D.cpp"
  "${DH_SRC_DIR}/Dahlia/Scene/Camera3D.cpp"
  "${DH_SRC_DIR}/Dahlia/Utilities/IO.cpp"
)

add_dependencies("Dahlia"
    "ImGui"
    "DahliaTools"
)

target_include_directories("Dahlia" PRIVATE
  ${DH_SRC_DIR}
  ${IMGUI_DIR}
  ${DH_LIB_DIR}
  ${Boost_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)
target_compile_definitions("Dahlia" PRIVATE
  GLM_ENABLE_EXPERIMENTAL
  DH_BUILD
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)

target_link_libraries("Dahlia" PRIVATE
  ImGui
  glfw
  Boost::filesystem
  OpenGL::GL
  $<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW::GLEW>
  Freetype::Freetype
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
target_compile_options("Dahlia" PRIVATE ${COMPILE_OPTIONS})

set_target_properties("Dahlia" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
    OUTPUT_NAME "Dahlia")
    

if(CMAKE_CONFIGURATION_TYPES)
  set(DEBUG_BIN_DIR "${DH_BIN_DIR}")
  set(RELEASE_BIN_DIR "${DH_BIN_DIR}")
else()
  set(DEBUG_BIN_DIR "${DH_BIN_DIR}/Debug")
  set(RELEASE_BIN_DIR "${DH_BIN_DIR}/Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Dahlia" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
  )
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Dahlia" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
  )
endif()
