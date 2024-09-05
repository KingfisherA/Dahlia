#pragma once

#include<vulkan/vulkan.h>

#include "VulkanCommandBuffers.h"
#include "GraphicsEngine/Platform/GLFW/GLFWDisplayBase.h"

namespace GEng {
    class GPUContext;
    class VulkanSwapChain;

    class VulkanDisplay : public GLFWDisplayBase {
    public:
        constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        VulkanDisplay(GPUContext *ctx, const DisplayProps &props);
        ~VulkanDisplay() override;
        //
        VulkanDrawingCommands &GetCommandList() const { return mDrawingCommands; }
        //
        void RenderFrame();
        //
        //
        [[nodiscard]] VkSurfaceKHR GetSurface() const { return mSurface; }
        //
        //
    private:
        VkSurfaceKHR mSurface = nullptr;
        VulkanSwapChain *mSwapChain = nullptr;

        void CreateSyncPrimitives();

        bool CreateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        //Frame synchronisation primitives
        std::vector<VkSemaphore> mImageAvailableSemaphores;
        std::vector<VkSemaphore> mRenderFinishedSemaphores;
        std::vector<VkFence> mInFlightFences;

        //Drawing Command Buffers
        std::vector<VkCommandBuffer> mCommandBuffers;

        mutable VulkanDrawingCommands mDrawingCommands;

        uint32_t mCurrentFrame = 0;

        bool mFramebufferResized = false;
    };
} // GEng
