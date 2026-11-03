#include <stdexcept>

#include <imgui.h>

#include "Dahlia/GUI/ImGuiRenderer.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLImGuiRenderer.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanImGuiRenderer.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalImGuiRenderer.h"
#endif

namespace DH
{
    void ImGuiRenderer::RenderPlatformWindows()
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    uRef<ImGuiRenderer> ImGuiRenderer::Create(const Display& display)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return umake<OpenGLImGuiRenderer>(display);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return umake<VulkanImGuiRenderer>(display);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return umake<MetalImGuiRenderer>(display);
#endif
            default:
                throw std::runtime_error("No ImGui renderer for this graphics API");
        }
    }
}
