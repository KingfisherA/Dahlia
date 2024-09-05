#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "GraphicsEngine/Platform/Vulkan/VulkanGPUContext.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanDisplay.h"

#include "GraphicsEngine/Events/WindowEvents.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanSwapChain.h"


namespace GEng {
    //
    //
    VulkanDisplay::VulkanDisplay(GPUContext *ctx, const DisplayProps &props) {
        if (ctx->GetContextAPI() != API::Vulkan)
            throw std::runtime_error("Cannot create a Vulkan display in a non-Vulkan GPU Context");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWDisplayBase::CreateDisplay(props);

        VulkanGPUContext *vkCtx = reinterpret_cast<VulkanGPUContext *>(ctx);
        if (glfwCreateWindowSurface(vkCtx->GetInstance(), mWindow, nullptr, &mSurface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan window surface");

        vkCtx->CreateDevice(mSurface);
        mSwapChain = new VulkanSwapChain(vkCtx, this);
        CreateSyncPrimitives();
        CreateCommandBuffers();

        //To-Do
        //  glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *window, int width, int height) {
        //     GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);
        //     data.mWidth = width;
        //     data.mHeight = height;
        //
        //     WindowResizeEvent resizeEvt(width, height);
        //     data.mEventCallback(resizeEvt);
        // });
    }
    //
    VulkanDisplay::~VulkanDisplay() {
        if (GPUContext::Get()->GetContextAPI() != API::Vulkan)
            return;

        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vkCtx->GetDevice(), mImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vkCtx->GetDevice(), mRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vkCtx->GetDevice(), mInFlightFences[i], nullptr);
        }
        delete mSwapChain;
        vkDestroySurfaceKHR(vkCtx->GetInstance(), mSurface, nullptr);
    }
    //
    void VulkanDisplay::RenderFrame() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        vkWaitForFences(vkCtx->GetDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(vkCtx->GetDevice(), mSwapChain->GetSwapChain(), UINT64_MAX,
                                                mImageAvailableSemaphores[mCurrentFrame],
                                                VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            mSwapChain->RecreateSwapChain();
            return;
        } else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");


        vkResetFences(vkCtx->GetDevice(), 1, &mInFlightFences[mCurrentFrame]);

        vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
        RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

        //UpdateUniformBuffers(mCurrentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

        VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(vkCtx->GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {mSwapChain->GetSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vkCtx->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
            mFramebufferResized = false;
            mSwapChain->RecreateSwapChain();
        } else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");

        mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    //
    void VulkanDisplay::CreateSyncPrimitives() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            if (vkCreateSemaphore(vkCtx->GetDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateSemaphore(vkCtx->GetDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateFence(vkCtx->GetDevice(), &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create synchronisation primitives \n");
    }
    //
    bool VulkanDisplay::CreateCommandBuffers() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vkCtx->GetCommandPool()->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) mCommandBuffers.size();

        if (vkAllocateCommandBuffers(vkCtx->GetDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command buffer \n");
    }
    //
    void VulkanDisplay::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer!\n");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mSwapChain->GetRenderPass();
        renderPassInfo.framebuffer = mSwapChain->GetSwapChainFramebuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();


        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        if (mDrawingCommands.mPipeline != nullptr)
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              mDrawingCommands.mPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mSwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(mSwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mSwapChain->GetSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        size_t noOfVBuffers = mDrawingCommands.mVertexBuffers.size();
        VkBuffer *vertexBuffers = (VkBuffer *) alloca(noOfVBuffers*sizeof(VkBuffer*));
        VkDeviceSize *offsets = (VkDeviceSize *) alloca(noOfVBuffers*sizeof(VkDeviceSize*));
        for (size_t i = 0; i < noOfVBuffers; i++) {
            vertexBuffers[i] = mDrawingCommands.mVertexBuffers[i]->GetBuffer();
            offsets[i] = 0;
        }
        vkCmdBindVertexBuffers(commandBuffer, 0, noOfVBuffers, vertexBuffers, offsets);

        if (mDrawingCommands.mIndexBuffer != nullptr)
            vkCmdBindIndexBuffer(
                commandBuffer, mDrawingCommands.mIndexBuffer->GetBuffer(), 0,
                mDrawingCommands.mIndexBuffer->GetIndexType());

        // static auto startTime = std::chrono::high_resolution_clock::now();
        //
        // auto currentTime = std::chrono::high_resolution_clock::now();
        // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        // PushConstants pConstants{};
        // pConstants.uColour = (glm::vec3((float) cos(time * 2.0), (float) cos(time * 1.5), (float) cos(time * 0.5)) +
        //                       glm::vec3(1.0)) * 0.5f;
        //
        // vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants),
        //                    (const void *) &pConstants);
        //
        // vkCmdBindDescriptorSets(commandBuffer,
        //                         VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout,
        //                         0, 1, &mDescriptorSets[mCurrentFrame],
        //                         0, nullptr);


        if (mDrawingCommands.mIndexBuffer != nullptr)
            vkCmdDrawIndexed(commandBuffer, mDrawingCommands.mIndexBuffer->GetElementCount(),
                             mDrawingCommands.mInstanceCount, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, mDrawingCommands.mElementCount, mDrawingCommands.mInstanceCount, 1, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to end recording to command buffer!");
    }
    //
    //
} // GEng
