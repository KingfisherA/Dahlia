#pragma once

#include<vulkan/vulkan.h>

#include "VulkanCommandBuffers.h"
#include "VulkanSwapChain.h"
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
        [[nodiscard]] VulkanSwapChain *GetSwapChain() const { return mSwapChain; }
        [[nodiscard]] VkRenderPass GetRenderPass() const {
            if (mSwapChain != nullptr)
                return mSwapChain->GetRenderPass();

            return nullptr;
        }

        //
        //
    private:
        VkSurfaceKHR mSurface = nullptr;
        VulkanSwapChain *mSwapChain = nullptr;

        void CreateSyncPrimitives();

        void CreateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        static void OnWindowResize(GLFWwindow *window, int width, int height);

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
