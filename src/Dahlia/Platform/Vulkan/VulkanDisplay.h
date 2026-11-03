#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class VulkanCommandList;
    class VulkanSwapChain;

    // Window + surface + swapchain with VulkanGPUContext::MAX_FRAMES_IN_FLIGHT frames in flight.
    // The swapchain is recreated when the window size changes or presentation reports it is
    // out of date.
    class DH_API VulkanDisplay : public GLFWDisplayBase
    {
      public:
        VulkanDisplay(GPUContext* ctx, const DisplayProps& props);
        ~VulkanDisplay() override;

        [[nodiscard]] CommandList* BeginFrame() override;
        void EndFrame() override;

        [[nodiscard]] RenderTargetLayout GetRenderTargetLayout() const override;

        [[nodiscard]] VulkanSwapChain& GetSwapChain() const { return *mSwapChain; }
        [[nodiscard]] uint32_t GetCurrentImageIndex() const { return mImageIndex; }

      private:
        static constexpr uint32_t FRAMES = VulkanGPUContext::MAX_FRAMES_IN_FLIGHT;

        void RecreateSwapChain();
        void CreateRenderFinishedSemaphores();
        void DestroyRenderFinishedSemaphores();

        VkSurfaceKHR mSurface = VK_NULL_HANDLE;
        uRef<VulkanSwapChain> mSwapChain;

        VkCommandPool mCommandPool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, FRAMES> mCommandBuffers{};
        std::array<VkSemaphore, FRAMES> mImageAvailable{};
        std::array<VkFence, FRAMES> mInFlight{};
        // One per swapchain image: presentation may still be waiting on it after the frame's fence
        std::vector<VkSemaphore> mRenderFinished;

        uint32_t mFrame = 0;
        uint32_t mImageIndex = 0;
        bool mFrameActive = false;

        uRef<VulkanCommandList> mCommandList;
    };
}
