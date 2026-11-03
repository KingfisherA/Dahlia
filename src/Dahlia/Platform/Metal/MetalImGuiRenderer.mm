// MetalImGuiRenderer.mm - compiled with ARC. Bridges metal-cpp objects to the Objective-C
// ImGui Metal backend.

#include <Metal/Metal.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_metal.h>

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/Metal/MetalCommandList.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalImGuiRenderer.h"

namespace DH
{
    MetalImGuiRenderer::MetalImGuiRenderer(const Display& display)
    {
        ImGui_ImplGlfw_InitForOther(static_cast<GLFWwindow*>(display.GetAPIDisplay()), true);
        id<MTLDevice> device =
          (__bridge id<MTLDevice>)GPUContext::Get<MetalGPUContext>()->GetNativeDevice();
        ImGui_ImplMetal_Init(device);
    }

    MetalImGuiRenderer::~MetalImGuiRenderer()
    {
        GPUContext::Get<MetalGPUContext>()->WaitIdle();
        ImGui_ImplMetal_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void MetalImGuiRenderer::NewFrame(CommandList& cmd)
    {
        auto& mtlCmd = static_cast<MetalCommandList&>(cmd);
        ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor*)mtlCmd.GetRenderPassDescriptor());
        ImGui_ImplGlfw_NewFrame();
    }

    void MetalImGuiRenderer::Render(ImDrawData* drawData, CommandList& cmd)
    {
        auto& mtlCmd = static_cast<MetalCommandList&>(cmd);
        ImGui_ImplMetal_RenderDrawData(drawData,
                                       (__bridge id<MTLCommandBuffer>)mtlCmd.GetCommandBuffer(),
                                       (__bridge id<MTLRenderCommandEncoder>)mtlCmd.GetEncoder());
    }
}
