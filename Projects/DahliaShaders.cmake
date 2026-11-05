# Shader compilation for Dahlia and for applications built on it.
#
# Shaders are Vulkan-style GLSL (#version 450) compiled offline to SPIR-V with glslc. Backends
# translate the SPIR-V at load time (SPIRV-Cross for OpenGL/Metal).
#
#   dahlia_compile_shaders(<target> [SOURCES] <file>...
#                          [OUTPUT_DIR <dir>]        # default: ${CMAKE_CURRENT_BINARY_DIR}/GeneratedShaders/<target>
#                          [INCLUDE_DIRS <dir>...]   # extra glslc -I dirs for #include
#                          [DEFINES <NAME[=VALUE]>...]
#                          [FORMAT c|bin]            # default: c
#                          [TARGET_ENV <env>])       # default: vulkan1.0
#
# FORMAT c emits each shader as a C initializer list (glslc -mfmt=c) into <OUTPUT_DIR>/<file>.inc,
# for use as:
#     static const uint32_t kSpirv[] =
#     #include "MyShader.frag.inc"
#     ;
# FORMAT bin emits a binary <OUTPUT_DIR>/<file>.spv to load at runtime.
#
# Each source's own directory is always an include directory, so shaders can #include files next
# to them. Dependencies on included files are tracked through glslc depfiles. A custom target
# named <target>Shaders builds the shaders; <target> depends on it and gets OUTPUT_DIR as a
# private include directory. The output directory is returned in <target>_SHADER_OUTPUT_DIR.

# Ninja: depfile paths are relative to the build directory. Recorded with the function definition
# below, so it applies wherever dahlia_compile_shaders is called from.
if(POLICY CMP0116)
  cmake_policy(SET CMP0116 NEW)
endif()

find_program(GLSLC glslc HINTS $ENV{VULKAN_SDK}/bin)

# Kept for scripts that used the previous shader build's variables
set(DH_SHADER_SRC_DIR ${DH_DIR}/Shaders)
set(DH_SHADER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/GeneratedShaders)
if(NOT GLSLC)
  message(FATAL_ERROR
    "glslc not found. Install the Vulkan SDK or shaderc (macOS: brew install shaderc).")
endif()

function(dahlia_compile_shaders TARGET)
  cmake_parse_arguments(PARSE_ARGV 1 ARG
    ""
    "OUTPUT_DIR;FORMAT;TARGET_ENV"
    "SOURCES;INCLUDE_DIRS;DEFINES")

  set(SOURCES ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
  if(NOT SOURCES)
    message(FATAL_ERROR "dahlia_compile_shaders(${TARGET}): no shader sources given")
  endif()
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "dahlia_compile_shaders: '${TARGET}' is not a target")
  endif()

  if(NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/GeneratedShaders/${TARGET})
  endif()
  if(NOT ARG_FORMAT)
    set(ARG_FORMAT c)
  endif()
  if(NOT ARG_TARGET_ENV)
    set(ARG_TARGET_ENV vulkan1.0)
  endif()

  if(ARG_FORMAT STREQUAL "c")
    set(FORMAT_FLAGS -mfmt=c)
    set(EXTENSION .inc)
  elseif(ARG_FORMAT STREQUAL "bin")
    set(FORMAT_FLAGS -mfmt=bin)
    set(EXTENSION .spv)
  else()
    message(FATAL_ERROR "dahlia_compile_shaders(${TARGET}): FORMAT must be c or bin")
  endif()

  set(INCLUDE_FLAGS)
  foreach(DIR ${ARG_INCLUDE_DIRS})
    get_filename_component(DIR ${DIR} ABSOLUTE)
    list(APPEND INCLUDE_FLAGS -I ${DIR})
  endforeach()
  set(DEFINE_FLAGS)
  foreach(DEFINE ${ARG_DEFINES})
    list(APPEND DEFINE_FLAGS -D${DEFINE})
  endforeach()

  set(OUTPUTS)
  foreach(SOURCE ${SOURCES})
    get_filename_component(SHADER_IN ${SOURCE} ABSOLUTE)
    get_filename_component(SHADER_NAME ${SOURCE} NAME)
    get_filename_component(SHADER_DIR ${SHADER_IN} DIRECTORY)
    set(SHADER_OUT ${ARG_OUTPUT_DIR}/${SHADER_NAME}${EXTENSION})

    add_custom_command(
      OUTPUT ${SHADER_OUT}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ARG_OUTPUT_DIR}
      COMMAND ${GLSLC} --target-env=${ARG_TARGET_ENV} -O ${FORMAT_FLAGS}
              -I ${SHADER_DIR} ${INCLUDE_FLAGS} ${DEFINE_FLAGS}
              -MD -MF ${SHADER_OUT}.d
              -o ${SHADER_OUT} ${SHADER_IN}
      DEPENDS ${SHADER_IN}
      DEPFILE ${SHADER_OUT}.d
      COMMENT "Compiling shader ${SHADER_NAME} to SPIR-V"
      VERBATIM
    )
    list(APPEND OUTPUTS ${SHADER_OUT})
  endforeach()

  add_custom_target(${TARGET}Shaders DEPENDS ${OUTPUTS})
  add_dependencies(${TARGET} ${TARGET}Shaders)
  target_include_directories(${TARGET} PRIVATE ${ARG_OUTPUT_DIR})
  set(${TARGET}_SHADER_OUTPUT_DIR ${ARG_OUTPUT_DIR} PARENT_SCOPE)
endfunction()
