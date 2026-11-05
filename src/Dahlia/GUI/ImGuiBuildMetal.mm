// Compiles the Dear ImGui Metal renderer backend into Dahlia (only when Metal is enabled).
// Built with ARC (see src/Dahlia/CMakeLists.txt).

// Upstream backend code: it annotates only some pointers with nullability specifiers, and uses
// MTLStorageModeManaged (deprecated in the macOS 27 SDK) for Intel Macs
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include "backends/imgui_impl_metal.mm"
#pragma clang diagnostic pop
