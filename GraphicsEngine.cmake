add_library("GraphicsEngine" STATIC
    "src/GraphicsEngine/3D/Buffers.cpp"
    "src/GraphicsEngine/3D/Buffers.h"
    "src/GraphicsEngine/3D/Framebuffer.cpp"
    "src/GraphicsEngine/3D/Framebuffer.h"
    "src/GraphicsEngine/3D/ShaderProgram.cpp"
    "src/GraphicsEngine/3D/ShaderProgram.h"
    "src/GraphicsEngine/3D/Texture.cpp"
    "src/GraphicsEngine/3D/Texture.h"
    "src/GraphicsEngine/3D/VertexArray.cpp"
    "src/GraphicsEngine/3D/VertexArray.h"
      "src/GraphicsEngine/3D/stb/stb_image.cpp"
      "src/GraphicsEngine/3D/stb/stb_image.h"
    "src/GraphicsEngine/Application/GraphicsApp.cpp"
    "src/GraphicsEngine/Application/GraphicsApp.h"
    "src/GraphicsEngine/Events/AppEvents.h"
    "src/GraphicsEngine/Events/Events.h"
    "src/GraphicsEngine/Events/KeyEvents.h"
    "src/GraphicsEngine/Events/MouseEvents.h"
    "src/GraphicsEngine/Events/WindowEvents.h"
  "src/GraphicsEngine/GEng_API.h"
    "src/GraphicsEngine/GUI/Display.cpp"
    "src/GraphicsEngine/GUI/Display.h"
    "src/GraphicsEngine/GUI/ImGuiBuild.cpp"
    "src/GraphicsEngine/GUI/ImGuiLayer.cpp"
    "src/GraphicsEngine/GUI/ImGuiLayer.h"
    "src/GraphicsEngine/GUI/ImSpinner.h"
    "src/GraphicsEngine/GUI/Text.cpp"
    "src/GraphicsEngine/GUI/Text.h"
    "src/GraphicsEngine/Layers/Layer.cpp"
    "src/GraphicsEngine/Layers/Layer.h"
    "src/GraphicsEngine/Layers/LayerStack.cpp"
    "src/GraphicsEngine/Layers/LayerStack.h"
    "src/GraphicsEngine/Maths/Geometry.cpp"
    "src/GraphicsEngine/Maths/Geometry.hpp"
    "src/GraphicsEngine/Maths/Maths.cpp"
    "src/GraphicsEngine/Maths/Maths.hpp"
    "src/GraphicsEngine/Platform/API Core.h"
      "src/GraphicsEngine/Platform/GLFW/GLFWDisplay.cpp"
      "src/GraphicsEngine/Platform/GLFW/GLFWDisplay.h"
    "src/GraphicsEngine/Platform/Intrinsics.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLBuffers.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLBuffers.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLCore.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLCore.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLTexture.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLTexture.h"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.cpp"
      "src/GraphicsEngine/Platform/OpenGL/OpenGLVertexArray.h"
      "src/GraphicsEngine/Platform/Windows/Console.cpp"
      "src/GraphicsEngine/Platform/Windows/Console.h"
    "src/GraphicsEngine/Scene/Camera3D.cpp"
    "src/GraphicsEngine/Scene/Camera3D.hpp"
    "src/GraphicsEngine/Utilities/Assert.h"
    "src/GraphicsEngine/Utilities/IO.cpp"
    "src/GraphicsEngine/Utilities/IO.h"
    "src/GraphicsEngine/Utilities/Memory.h"
    "src/GraphicsEngine/Utilities/PathUtils.h"
    "src/GraphicsEngine/Utilities/StringOperations.cpp"
    "src/GraphicsEngine/Utilities/StringOperations.h"
    "src/GraphicsEngine/Utilities/Threading.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("GraphicsEngine" PROPERTIES
    OUTPUT_NAME "GraphicsEngine"
    ARCHIVE_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64/GraphicsEngine"
    LIBRARY_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64/GraphicsEngine"
    RUNTIME_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64/GraphicsEngine"
  )
endif()
target_include_directories("GraphicsEngine" PRIVATE
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/C++/GraphicsEngine/src>
  $<$<CONFIG:Debug>:/usr/include>
  $<$<CONFIG:Debug>:/usr/local/include>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries/glm>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries/ImGui>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries>
  $<$<CONFIG:Debug>:/usr/include/freetype2>
  $<$<CONFIG:Debug>:/usr/include/libpng16>
)
target_compile_definitions("GraphicsEngine" PRIVATE
  $<$<CONFIG:Debug>:OS_LINUX>
  $<$<CONFIG:Debug>:GENG_STATIC>
  $<$<CONFIG:Debug>:GENG_BUILD>
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
)
target_link_directories("GraphicsEngine" PRIVATE
)
target_link_libraries("GraphicsEngine"
)
target_compile_options("GraphicsEngine" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-O0>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-O0>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("GraphicsEngine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("GraphicsEngine" PROPERTIES
    OUTPUT_NAME "GraphicsEngine"
    ARCHIVE_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64/GraphicsEngine"
    LIBRARY_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64/GraphicsEngine"
    RUNTIME_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64/GraphicsEngine"
  )
endif()
target_include_directories("GraphicsEngine" PRIVATE
  $<$<CONFIG:Release>:/media/charlie/Data/dev/C++/GraphicsEngine/src>
  $<$<CONFIG:Release>:/usr/include>
  $<$<CONFIG:Release>:/usr/local/include>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries/glm>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries/ImGui>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries>
  $<$<CONFIG:Release>:/usr/include/freetype2>
  $<$<CONFIG:Release>:/usr/include/libpng16>
)
target_compile_definitions("GraphicsEngine" PRIVATE
  $<$<CONFIG:Release>:OS_LINUX>
  $<$<CONFIG:Release>:GENG_STATIC>
  $<$<CONFIG:Release>:GENG_BUILD>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_directories("GraphicsEngine" PRIVATE
)
target_link_libraries("GraphicsEngine"
)
target_compile_options("GraphicsEngine" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("GraphicsEngine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()