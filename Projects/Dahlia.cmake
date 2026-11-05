add_library("Dahlia" SHARED
  "${DH_SRC_DIR}/Dahlia/Graphics/BindGroup.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/Buffer.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/Framebuffer.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/GraphicsPipeline.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/GPUMesh.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/ShaderModule.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/Texture.cpp"
  "${DH_SRC_DIR}/Dahlia/Graphics/stb/stb_image.cpp"
  "${DH_SRC_DIR}/Dahlia/Application/GraphicsApp.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/Display.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/ImGuiBuild.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/ImGuiLayer.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/ImGuiRenderer.cpp"
  "${DH_SRC_DIR}/Dahlia/GUI/Text.cpp"
  "${DH_SRC_DIR}/Dahlia/Layers/Layer.cpp"
  "${DH_SRC_DIR}/Dahlia/Layers/LayerStack.cpp"
  "${DH_SRC_DIR}/Dahlia/Maths/Geometry.cpp"
  "${DH_SRC_DIR}/Dahlia/Maths/Maths.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/GLFW/GLFWDisplayBase.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/GPUContext.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLBuffer.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLCommandList.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLContext.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLDisplay.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLFramebuffer.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLGraphicsPipeline.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLShaderModule.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLTexture.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/OpenGL/OpenGLImGuiRenderer.cpp"
  "${DH_SRC_DIR}/Dahlia/Platform/Windows/Console.cpp"
  "${DH_SRC_DIR}/Dahlia/Resources/FontsRes.cpp"
  "${DH_SRC_DIR}/Dahlia/Resources/ShaderSources.cpp"
  "${DH_SRC_DIR}/Dahlia/Scene/Camera2D.cpp"
  "${DH_SRC_DIR}/Dahlia/Scene/Camera3D.cpp"
  "${DH_SRC_DIR}/Dahlia/Utilities/IO.cpp"
)

# Optional graphics backends. OpenGL 4.1 is always built.
find_package(Vulkan QUIET)
option(DH_ENABLE_VULKAN "Build the Vulkan backend (needs Vulkan 1.3)" ${Vulkan_FOUND})
option(DH_ENABLE_METAL "Build the Metal backend (Apple only)" ${APPLE})
set(DH_METAL_CPP_DIR "/opt/metal-cpp_macOS26_iOS26-beta" CACHE PATH "metal-cpp headers")

if(DH_ENABLE_VULKAN)
  if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "DH_ENABLE_VULKAN is ON but no Vulkan SDK/loader was found")
  endif()
  target_sources("Dahlia" PRIVATE
    "${DH_SRC_DIR}/Dahlia/GUI/ImGuiBuildVulkan.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanBindGroup.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanBuffer.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanCommandList.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanContext.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanDisplay.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanFramebuffer.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanGraphicsPipeline.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanImGuiRenderer.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanShaderModule.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanSwapChain.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Vulkan/VulkanTexture.cpp"
  )
  target_compile_definitions("Dahlia" PRIVATE DH_VULKAN_ENABLED)
  target_link_libraries("Dahlia" PRIVATE Vulkan::Vulkan)
  message(STATUS "Dahlia: Vulkan backend enabled")
endif()

if(DH_ENABLE_METAL)
  if(NOT APPLE)
    message(FATAL_ERROR "The Metal backend can only be built on Apple platforms")
  endif()
  set(DH_METAL_SOURCES
    "${DH_SRC_DIR}/Dahlia/GUI/ImGuiBuildMetal.mm"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalBuffer.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalCommandList.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalContext.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalDisplay.mm"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalFramebuffer.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalGraphicsPipeline.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalImGuiRenderer.mm"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalImpl.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalShaderModule.cpp"
    "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalTexture.cpp"
  )
  target_sources("Dahlia" PRIVATE ${DH_METAL_SOURCES})
  set_source_files_properties("${DH_SRC_DIR}/Dahlia/GUI/ImGuiBuildMetal.mm"
                              "${DH_SRC_DIR}/Dahlia/Platform/Metal/MetalImGuiRenderer.mm"
                              PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
  target_include_directories("Dahlia" PRIVATE ${DH_METAL_CPP_DIR})
  target_compile_definitions("Dahlia" PRIVATE DH_METAL_ENABLED)
  target_link_libraries("Dahlia" PRIVATE
    spirv-cross-msl
    $<LINK_LIBRARY:FRAMEWORK,Foundation>
    $<LINK_LIBRARY:FRAMEWORK,QuartzCore>
    $<LINK_LIBRARY:FRAMEWORK,Metal>
    $<LINK_LIBRARY:FRAMEWORK,Cocoa>
  )
  message(STATUS "Dahlia: Metal backend enabled")
endif()

add_dependencies("Dahlia"
    "ImGui"
    "DahliaTools"
)

# Engine shaders, #included by src/Dahlia/Resources/ShaderSources.cpp
dahlia_compile_shaders("Dahlia"
  OUTPUT_DIR ${DH_SHADER_OUT_DIR}
  SOURCES
    ${DH_DIR}/Shaders/ColourDiffuse.vert
    ${DH_DIR}/Shaders/ColourDiffuse.frag
    ${DH_DIR}/Shaders/SolidColour.vert
    ${DH_DIR}/Shaders/SolidColour.frag
    ${DH_DIR}/Shaders/Text.vert
    ${DH_DIR}/Shaders/Text.frag
)

# Public headers include glm and ImGui, so applications linking Dahlia get these too
target_include_directories("Dahlia" PUBLIC
  ${DH_SRC_DIR}
  ${DH_LIB_DIR}
  ${IMGUI_DIR}
  ${GLM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)
target_include_directories("Dahlia" PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)
target_compile_definitions("Dahlia" PUBLIC
  GLM_ENABLE_EXPERIMENTAL
)
target_compile_definitions("Dahlia" PRIVATE
  DH_BUILD
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_libraries("Dahlia" PUBLIC glm::glm)

target_link_libraries("Dahlia" PRIVATE
  ImGui
  glfw
  Boost::filesystem
  OpenGL::GL
  $<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW::GLEW>
  Freetype::Freetype
  spirv-cross-glsl
  spirv-cross-core
  $<$<PLATFORM_ID:Darwin>:$<LINK_LIBRARY:FRAMEWORK,OpenGL>>
  DahliaTools
)

set(COMPILE_OPTIONS )
if((CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR 
   (CMAKE_CXX_COMPILER_ID STREQUAL GNU))
    list(APPEND COMPILE_OPTIONS
      -fPIC
      -Wno-elaborated-enum-base
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
