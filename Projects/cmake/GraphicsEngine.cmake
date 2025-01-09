add_library("GraphicsEngine" SHARED
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Buffers.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Framebuffer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/ShaderProgram.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Texture.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/VertexArray.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/stb/stb_image.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Application/GraphicsApp.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Display.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImGuiBuild.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImGuiLayer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Text.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/Layer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/LayerStack.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Geometry.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Maths.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GLFW/GLFWDisplayBase.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GPUContext.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLBuffers.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLContext.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLDisplay.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLTexture.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/Windows/Console.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Resources/FontsRes.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Resources/ShaderSources.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera2D.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera3D.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/IO.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/StringOperations.cpp"
)

add_dependencies("GraphicsEngine"
    "ImGui"
)

target_include_directories("GraphicsEngine" PRIVATE
  ${GENG_SRC_DIR}
  ${IMGUI_DIR}
  ${GENG_LIB_DIR}
  ${Boost_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)
target_compile_definitions("GraphicsEngine" PRIVATE
  GLM_ENABLE_EXPERIMENTAL
  GENG_BUILD
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("GraphicsEngine" PRIVATE
  ImGui
  glfw
  Boost::filesystem
  OpenGL::GL
  $<$<PLATFORM_ID:Linux>:GLEW::glew_s>
  $<$<PLATFORM_ID:Windows>:GLEW::GLEW>
  Freetype::Freetype
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
      list(APPEND COMPILE_OPTIONS
        /O2
      )
    endif()
endif()

target_compile_options("GraphicsEngine" PRIVATE ${COMPILE_OPTIONS})

set_target_properties("GraphicsEngine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "GraphicsEngine")


if(CMAKE_CONFIGURATION_TYPES)
  set(DEBUG_BIN_DIR "${GENG_BIN_DIR}")
  set(RELEASE_BIN_DIR "${GENG_BIN_DIR}")
else()
  set(DEBUG_BIN_DIR "${GENG_BIN_DIR}/Debug")
  set(RELEASE_BIN_DIR "${GENG_BIN_DIR}/Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("GraphicsEngine" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEBUG_BIN_DIR}"
  )
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("GraphicsEngine" PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${RELEASE_BIN_DIR}"
  )
endif()
