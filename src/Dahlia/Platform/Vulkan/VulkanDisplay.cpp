#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Dahlia/Platform/Vulkan/VulkanCommandList.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanDisplay.h"
#include "Dahlia/Platform/Vulkan/VulkanSwapChain.h"

namespace DH
{
    VulkanDisplay::VulkanDisplay(GPUContext* ctx, const DisplayProps& props)
    {
        if (ctx->GetContextAPI() != GraphicsAPI::Vulkan)
            throw std::runtime_error("Cannot create a Vulkan display in a non-Vulkan GPU context");
        auto* vkCtx = static_cast<VulkanGPUContext*>(ctx);

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        CreateDisplay(props);
        if (mWindow == nullptr)
            throw std::runtime_error("Failed to create a window for Vulkan");

        DH_VK_CHECK(glfwCreateWindowSurface(vkCtx->GetInstance(), mWindow, nullptr, &mSurface),
                    "glfwCreateWindowSurface");
        vkCtx->CreateDevice(mSurface);
        VkDevice device = vkCtx->GetDevice();

        const Dimension2D size = GetFramebufferSize();
        mSwapChain = umake<VulkanSwapChain>(mSurface, VkExtent2D{ (uint32_t)size.x, (uint32_t)size.y });

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = vkCtx->GetGraphicsQueueFamily();
        DH_VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &mCommandPool),
                    "vkCreateCommandPool");

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = FRAMES;
        DH_VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, mCommandBuffers.data()),
                    "vkAllocateCommandBuffers");

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (uint32_t i = 0; i < FRAMES; i++)
        {
            DH_VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &mImageAvailable[i]),
                        "vkCreateSemaphore");
            DH_VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &mInFlight[i]), "vkCreateFence");
        }
        CreateRenderFinishedSemaphores();

        mCommandList = umake<VulkanCommandList>(*this);

        GLFWDisplayBase::Show();
    }

    VulkanDisplay::~VulkanDisplay()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();
        vkDeviceWaitIdle(device);

        mCommandList.reset();
        DestroyRenderFinishedSemaphores();
        for (uint32_t i = 0; i < FRAMES; i++)
        {
            vkDestroySemaphore(device, mImageAvailable[i], nullptr);
            vkDestroyFence(device, mInFlight[i], nullptr);
        }
        vkDestroyCommandPool(device, mCommandPool, nullptr);
        mSwapChain.reset();
        ctx->WaitIdle(); // Releases the swapchain's depth buffer
        vkDestroySurfaceKHR(ctx->GetInstance(), mSurface, nullptr);
    }

    void VulkanDisplay::CreateRenderFinishedSemaphores()
    {
        VkDevice device = GPUContext::Get<VulkanGPUContext>()->GetDevice();
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        mRenderFinished.resize(mSwapChain->GetImageCount());
        for (VkSemaphore& semaphore : mRenderFinished)
            DH_VK_CHECK(vkCreateSemaphore(device, &info, nullptr, &semaphore), "vkCreateSemaphore");
    }

    void VulkanDisplay::DestroyRenderFinishedSemaphores()
    {
        VkDevice device = GPUContext::Get<VulkanGPUContext>()->GetDevice();
        for (VkSemaphore semaphore : mRenderFinished)
            vkDestroySemaphore(device, semaphore, nullptr);
        mRenderFinished.clear();
    }

    void VulkanDisplay::RecreateSwapChain()
    {
        const Dimension2D size = GetFramebufferSize();
        if (size.x == 0 || size.y == 0)
            return;

        mSwapChain->Recreate({ (uint32_t)size.x, (uint32_t)size.y });
        DestroyRenderFinishedSemaphores();
        CreateRenderFinishedSemaphores();
    }

    CommandList* VulkanDisplay::BeginFrame()
    {
        if (mFrameActive)
            throw std::logic_error("BeginFrame called twice without EndFrame");

        const Dimension2D size = GetFramebufferSize();
        if (size.x == 0 || size.y == 0)
            return nullptr; // Minimised

        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        if (size.x != mSwapChain->GetExtent().width || size.y != mSwapChain->GetExtent().height)
            RecreateSwapChain();

        vkWaitForFences(device, 1, &mInFlight[mFrame], VK_TRUE, UINT64_MAX);
        ctx->CollectGarbage();

        VkResult result = vkAcquireNextImageKHR(device,
                                                mSwapChain->GetSwapChain(),
                                                UINT64_MAX,
                                                mImageAvailable[mFrame],
                                                VK_NULL_HANDLE,
                                                &mImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("vkAcquireNextImageKHR failed");

        vkResetFences(device, 1, &mInFlight[mFrame]);

        VkCommandBuffer cmd = mCommandBuffers[mFrame];
        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        DH_VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo), "vkBeginCommandBuffer");

        mCommandList->Begin(cmd);
        mFrameActive = true;
        return mCommandList.get();
    }

    void VulkanDisplay::EndFrame()
    {
        if (!mFrameActive)
            throw std::logic_error("EndFrame called without BeginFrame");
        mFrameActive = false;

        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkCommandBuffer cmd = mCommandBuffers[mFrame];

        VkImageLayout& layout = mSwapChain->ImageLayout(mImageIndex);
        VK::TransitionImage(cmd,
                            mSwapChain->GetImage(mImageIndex),
                            VK_IMAGE_ASPECT_COLOR_BIT,
                            layout,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        DH_VK_CHECK(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");

        const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &mImageAvailable[mFrame];
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &mRenderFinished[mImageIndex];
        DH_VK_CHECK(vkQueueSubmit(ctx->GetGraphicsQueue(), 1, &submitInfo, mInFlight[mFrame]),
                    "vkQueueSubmit");
        ctx->OnFrameSubmitted();

        VkSwapchainKHR swapChain = mSwapChain->GetSwapChain();
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &mRenderFinished[mImageIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &mImageIndex;
        const VkResult result = vkQueuePresentKHR(ctx->GetGraphicsQueue(), &presentInfo);

        mFrame = (mFrame + 1) % FRAMES;

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            RecreateSwapChain();
        else if (result != VK_SUCCESS)
            throw std::runtime_error("vkQueuePresentKHR failed");
    }

    RenderTargetLayout VulkanDisplay::GetRenderTargetLayout() const
    {
        return { { mSwapChain->GetFormat() }, VulkanSwapChain::DEPTH_FORMAT, 1 };
    }
}
