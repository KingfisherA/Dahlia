# Compiles the engine's Vulkan-style GLSL shaders in Shaders/ to SPIR-V with glslc.
# Each shader is emitted as a C initializer list (glslc -mfmt=c) into
# ${DH_SHADER_OUT_DIR}/<file>.inc, which src/Dahlia/Resources/ShaderSources.cpp #includes.
# Backends translate the SPIR-V at load time (SPIRV-Cross for OpenGL/Metal).

find_program(GLSLC glslc HINTS $ENV{VULKAN_SDK}/bin)
if(NOT GLSLC)
  message(FATAL_ERROR
    "glslc not found. Install the Vulkan SDK or shaderc (macOS: brew install shaderc).")
endif()

set(DH_SHADER_SRC_DIR ${DH_DIR}/Shaders)
set(DH_SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/GeneratedShaders)

set(DH_SHADER_SOURCES
  ColourDiffuse.vert
  ColourDiffuse.frag
  SolidColour.vert
  SolidColour.frag
  Text.vert
  Text.frag
)

set(DH_SHADER_OUTPUTS)
foreach(SHADER ${DH_SHADER_SOURCES})
  set(SHADER_IN ${DH_SHADER_SRC_DIR}/${SHADER})
  set(SHADER_OUT ${DH_SHADER_OUT_DIR}/${SHADER}.inc)
  add_custom_command(
    OUTPUT ${SHADER_OUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DH_SHADER_OUT_DIR}
    COMMAND ${GLSLC} --target-env=vulkan1.0 -O -mfmt=c
            -I ${DH_SHADER_SRC_DIR}
            -MD -MF ${SHADER_OUT}.d
            -o ${SHADER_OUT} ${SHADER_IN}
    DEPENDS ${SHADER_IN}
    DEPFILE ${SHADER_OUT}.d
    COMMENT "Compiling shader ${SHADER} to SPIR-V"
    VERBATIM
  )
  list(APPEND DH_SHADER_OUTPUTS ${SHADER_OUT})
endforeach()

add_custom_target("DahliaShaders" DEPENDS ${DH_SHADER_OUTPUTS})
