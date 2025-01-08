add_library("GraphicsEngine" SHARED
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Buffers.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Buffers.h"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Framebuffer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Framebuffer.h"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/ShaderProgram.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/ShaderProgram.h"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Texture.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/Texture.h"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/VertexArray.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/VertexArray.h"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/stb/stb_image.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/3D/stb/stb_image.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Application/GraphicsApp.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Application/GraphicsApp.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Events/AppEvents.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Events/Events.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Events/KeyEvents.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Events/MouseEvents.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Events/WindowEvents.h"
  "${GENG_SRC_DIR}/GraphicsEngine/GEng_API.h"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Display.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Display.h"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImGuiBuild.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImGuiLayer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImGuiLayer.h"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/ImSpinner.h"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Text.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/GUI/Text.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/Layer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/Layer.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/LayerStack.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Layers/LayerStack.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Geometry.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Geometry.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Maths.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Maths/Maths.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GLFW/GLFWDisplayBase.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GLFW/GLFWDisplayBase.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GPUContext.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/GPUContext.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/Intrinsics.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLBuffers.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLBuffers.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLContext.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLDisplay.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLDisplay.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLTexture.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLTexture.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/Windows/Console.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Platform/Windows/Console.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Resources/FontsRes.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Resources/Resources.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Resources/ShaderSources.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera2D.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera2D.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera3D.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Scene/Camera3D.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/Assert.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/IO.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/IO.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/Memory.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/PathUtils.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/StringOperations.cpp"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/StringOperations.h"
  "${GENG_SRC_DIR}/GraphicsEngine/Utilities/Threading.h"
)

add_dependencies("GraphicsEngine"
    "ImGui"
)

target_include_directories("GraphicsEngine" PRIVATE
  ${GENG_SRC_DIR}
  /media/charlie/Data/dev/Libraries/glm
  /media/charlie/Data/dev/Libraries/ImGui
  /media/charlie/Data/dev/Libraries
  ${Boost_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  /usr/include/libpng16
)
target_compile_definitions("GraphicsEngine" PRIVATE
  OS_LINUX
  GENG_BUILD
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("GraphicsEngine" PRIVATE
  ImGui
  glfw
  Boost::filesystem
  OpenGL::GL
  GLEW::glew_s
  Freetype::Freetype
)
target_compile_options("GraphicsEngine" PRIVATE
  -m64
  -fPIC
  -std=c++20
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("GraphicsEngine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "GraphicsEngine"
    ARCHIVE_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
    LIBRARY_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
    RUNTIME_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Debug-x64"
  )
  target_compile_options("GraphicsEngine" PRIVATE
    -g
    -O0
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("GraphicsEngine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
    OUTPUT_NAME "GraphicsEngine"
    ARCHIVE_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
    LIBRARY_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
    RUNTIME_OUTPUT_DIRECTORY "${GENG_BIN_DIR}/Release-x64"
  )
  target_compile_options("GraphicsEngine" PRIVATE
    -O2
  )
endif()
