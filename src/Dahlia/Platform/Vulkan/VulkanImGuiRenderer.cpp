#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Dahlia/Platform/Vulkan/VulkanCommandList.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanDisplay.h"
#include "Dahlia/Platform/Vulkan/VulkanImGuiRenderer.h"
#include "Dahlia/Platform/Vulkan/VulkanSwapChain.h"

namespace DH
{
    VulkanImGuiRenderer::VulkanImGuiRenderer(const Display& display)
    {
        const auto& vkDisplay = static_cast<const VulkanDisplay&>(display);
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const VulkanSwapChain& swapChain = vkDisplay.GetSwapChain();

        ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(display.GetAPIDisplay()), true);

        // Must match the display render pass attachments
        static VkFormat colourFormat;
        colourFormat = swapChain.GetImageFormat();

        ImGui_ImplVulkan_InitInfo info{};
        info.ApiVersion = ctx->GetApiVersion();
        info.Instance = ctx->GetInstance();
        info.PhysicalDevice = ctx->GetPhysicalDevice();
        info.Device = ctx->GetDevice();
        info.QueueFamily = ctx->GetGraphicsQueueFamily();
        info.Queue = ctx->GetGraphicsQueue();
        info.DescriptorPoolSize = 256; // ImGui owns a pool for its fonts and user textures
        info.MinImageCount = swapChain.GetMinImageCount();
        info.ImageCount = swapChain.GetImageCount();
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.UseDynamicRendering = true;
        info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colourFormat;
        info.PipelineRenderingCreateInfo.depthAttachmentFormat =
          ctx->ToTextureFormat(VulkanSwapChain::DEPTH_FORMAT);
        info.CheckVkResultFn = [](VkResult result)
        {
            if (result != VK_SUCCESS)
                throw std::runtime_error("ImGui Vulkan backend error " + std::to_string((int)result));
        };

        if (!ImGui_ImplVulkan_Init(&info))
            throw std::runtime_error("Failed to initialise the ImGui Vulkan backend");
        sActive = true;
    }

    VulkanImGuiRenderer::~VulkanImGuiRenderer()
    {
        vkDeviceWaitIdle(GPUContext::Get<VulkanGPUContext>()->GetDevice());
        sActive = false;
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void VulkanImGuiRenderer::NewFrame(CommandList&)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
    }

    void VulkanImGuiRenderer::Render(ImDrawData* drawData, CommandList& cmd)
    {
        ImGui_ImplVulkan_RenderDrawData(drawData,
                                        static_cast<VulkanCommandList&>(cmd).GetCommandBuffer());
    }
}
