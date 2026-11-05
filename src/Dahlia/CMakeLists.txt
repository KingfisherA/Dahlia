# Dahlia engine: libDahlia (shared) and libDahliaTools (static), built into build/Dahlia

# ---- DahliaTools ----------------------------------------------------------------------------------
add_library(DahliaTools STATIC
  Tools/Strings.cpp
  Tools/Strings.h
  Tools/Threading.h
)
target_include_directories(DahliaTools PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(DahliaTools PRIVATE DH_BUILD)
dahlia_target_defaults(DahliaTools)

# ---- Dahlia ---------------------------------------------------------------------------------------
add_library(Dahlia SHARED
  Application/GraphicsApp.cpp
  Graphics/BindGroup.cpp
  Graphics/Buffer.cpp
  Graphics/Framebuffer.cpp
  Graphics/GPUMesh.cpp
  Graphics/GraphicsPipeline.cpp
  Graphics/ShaderModule.cpp
  Graphics/Texture.cpp
  Graphics/stb/stb_image.cpp
  GUI/Display.cpp
  GUI/ImGuiBuild.cpp
  GUI/ImGuiLayer.cpp
  GUI/ImGuiRenderer.cpp
  GUI/Text.cpp
  Layers/Layer.cpp
  Layers/LayerStack.cpp
  Maths/Geometry.cpp
  Maths/Maths.cpp
  Platform/GLFW/GLFWDisplayBase.cpp
  Platform/GPUContext.cpp
  Platform/OpenGL/OpenGLBuffer.cpp
  Platform/OpenGL/OpenGLCommandList.cpp
  Platform/OpenGL/OpenGLContext.cpp
  Platform/OpenGL/OpenGLDisplay.cpp
  Platform/OpenGL/OpenGLFramebuffer.cpp
  Platform/OpenGL/OpenGLGraphicsPipeline.cpp
  Platform/OpenGL/OpenGLImGuiRenderer.cpp
  Platform/OpenGL/OpenGLShaderModule.cpp
  Platform/OpenGL/OpenGLTexture.cpp
  Platform/Windows/Console.cpp
  Resources/FontsRes.cpp
  Resources/ShaderSources.cpp
  Scene/Camera2D.cpp
  Scene/Camera3D.cpp
  Utilities/IO.cpp
)
dahlia_target_defaults(Dahlia)

# Public headers include glm, GLEW and ImGui (<ImGui/imgui.h>), so applications get these too
target_include_directories(Dahlia PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/Libraries
  ${PROJECT_SOURCE_DIR}/Libraries/ImGui
  ${GLEW_INCLUDE_DIRS}
)
target_compile_definitions(Dahlia
  PUBLIC GLM_ENABLE_EXPERIMENTAL
  PRIVATE DH_BUILD
)
target_compile_options(Dahlia PRIVATE
  $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-elaborated-enum-base>
)
target_link_libraries(Dahlia
  PUBLIC
    glm::glm
  PRIVATE
    DahliaTools
    ImGui
    glfw
    Boost::filesystem
    OpenGL::GL
    $<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW::GLEW>
    Freetype::Freetype
    spirv-cross-glsl
    spirv-cross-core
)

# Engine shaders, #included by Resources/ShaderSources.cpp
dahlia_compile_shaders(Dahlia
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/GeneratedShaders
  SOURCES
    ${PROJECT_SOURCE_DIR}/Shaders/ColourDiffuse.vert
    ${PROJECT_SOURCE_DIR}/Shaders/ColourDiffuse.frag
    ${PROJECT_SOURCE_DIR}/Shaders/SolidColour.vert
    ${PROJECT_SOURCE_DIR}/Shaders/SolidColour.frag
    ${PROJECT_SOURCE_DIR}/Shaders/Text.vert
    ${PROJECT_SOURCE_DIR}/Shaders/Text.frag
)

# ---- Optional backends (OpenGL 4.1 is always built) -----------------------------------------------
if(DH_ENABLE_VULKAN)
  target_sources(Dahlia PRIVATE
    GUI/ImGuiBuildVulkan.cpp
    Platform/Vulkan/VulkanBindGroup.cpp
    Platform/Vulkan/VulkanBuffer.cpp
    Platform/Vulkan/VulkanCommandList.cpp
    Platform/Vulkan/VulkanContext.cpp
    Platform/Vulkan/VulkanDisplay.cpp
    Platform/Vulkan/VulkanFramebuffer.cpp
    Platform/Vulkan/VulkanGraphicsPipeline.cpp
    Platform/Vulkan/VulkanImGuiRenderer.cpp
    Platform/Vulkan/VulkanShaderModule.cpp
    Platform/Vulkan/VulkanSwapChain.cpp
    Platform/Vulkan/VulkanTexture.cpp
  )
  target_compile_definitions(Dahlia PRIVATE DH_VULKAN_ENABLED)
  target_link_libraries(Dahlia PRIVATE Vulkan::Vulkan)
  message(STATUS "Dahlia: Vulkan backend enabled")
endif()

if(DH_ENABLE_METAL)
  target_sources(Dahlia PRIVATE
    GUI/ImGuiBuildMetal.mm
    Platform/Metal/MetalBuffer.cpp
    Platform/Metal/MetalCommandList.cpp
    Platform/Metal/MetalContext.cpp
    Platform/Metal/MetalDisplay.mm
    Platform/Metal/MetalFramebuffer.cpp
    Platform/Metal/MetalGraphicsPipeline.cpp
    Platform/Metal/MetalImGuiRenderer.mm
    Platform/Metal/MetalImpl.cpp
    Platform/Metal/MetalShaderModule.cpp
    Platform/Metal/MetalTexture.cpp
  )
  # The ImGui Metal backend and the renderer built on it use ARC
  set_source_files_properties(GUI/ImGuiBuildMetal.mm Platform/Metal/MetalImGuiRenderer.mm
    PROPERTIES COMPILE_OPTIONS -fobjc-arc)
  target_include_directories(Dahlia PRIVATE ${DH_METAL_CPP_DIR})
  target_compile_definitions(Dahlia PRIVATE DH_METAL_ENABLED)
  target_link_libraries(Dahlia PRIVATE
    spirv-cross-msl
    $<LINK_LIBRARY:FRAMEWORK,Foundation>
    $<LINK_LIBRARY:FRAMEWORK,QuartzCore>
    $<LINK_LIBRARY:FRAMEWORK,Metal>
    $<LINK_LIBRARY:FRAMEWORK,Cocoa>
  )
  message(STATUS "Dahlia: Metal backend enabled")
endif()
