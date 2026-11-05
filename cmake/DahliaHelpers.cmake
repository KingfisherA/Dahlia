# Settings shared by every Dahlia target.
#
#   dahlia_target_defaults(<target>)
#
# C++20 without extensions, the shared MSVC runtime, and CONFIG_DEBUG / CONFIG_RELEASE defines.
# Optimisation and debug-info flags come from CMake's per-configuration defaults.
function(dahlia_target_defaults TARGET)
  set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OBJCXX_STANDARD 20
    OBJCXX_STANDARD_REQUIRED ON
    OBJCXX_EXTENSIONS OFF
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
  )
  target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:CONFIG_DEBUG>
    $<$<CONFIG:Release>:CONFIG_RELEASE>
  )
endfunction()
