#include "VulkanCommandBuffers.h"

#include "VulkanGPUContext.h"


namespace GEng {
    //
    //
    VulkanCommandPool::VulkanCommandPool(uint32_t queueFamilyIndex) {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(vkCtx->GetDevice(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool \n");
    }
    //
    VulkanCommandPool::~VulkanCommandPool() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        vkDestroyCommandPool(vkCtx->GetDevice(), mCommandPool, nullptr);
    }
    //
    //
    VulkanOneTimeCommandBuffer::VulkanOneTimeCommandBuffer(VulkanCommandPool *pool) {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pool->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        mCommandPool = pool->GetCommandPool();

        vkAllocateCommandBuffers(vkCtx->GetDevice(), &allocInfo, &mCommandBuffer);
    }
    //
    void VulkanOneTimeCommandBuffer::Begin() {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(mCommandBuffer, &beginInfo);
    }
    //
    void VulkanOneTimeCommandBuffer::End() {
        vkEndCommandBuffer(mCommandBuffer);
    }
    //
    void VulkanOneTimeCommandBuffer::Submit(VkQueue queue) {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mCommandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    }
    //
    VulkanOneTimeCommandBuffer::~VulkanOneTimeCommandBuffer() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        vkFreeCommandBuffers(vkCtx->GetDevice(), mCommandPool, 1, &mCommandBuffer);
    }
    //
    //
} // GEng
