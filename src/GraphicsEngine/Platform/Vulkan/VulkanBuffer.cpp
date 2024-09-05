#include "GraphicsEngine/Platform/Vulkan/VulkanBuffer.h"

#include <cstring>

#include "VulkanCommandBuffers.h"
#include "VulkanGPUContext.h"
#include "GraphicsEngine/Platform/GPUContext.h"

namespace GEng {
    //
    //
    VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties, DataType dataType): mBufferSize(size),
        mDataType(dataType) {
        CreateVulkanBuffer(size, usage, properties, mBuffer, mBufferMemory);
    }
    //
    VulkanBuffer::~VulkanBuffer() {
        VkDevice device = GPUContext::Get<VulkanGPUContext>()->GetDevice();
        vkDeviceWaitIdle(device);
        vkDestroyBuffer(device, mBuffer, nullptr);
        vkFreeMemory(device, mBufferMemory, nullptr);
    }
    //
    void VulkanBuffer::Copy(VulkanBuffer *destinationBuffer) {
        VulkanGPUContext *ctx = GPUContext::Get<VulkanGPUContext>();

        GENG_ASSERT(this->mBufferSize != destinationBuffer->mBufferSize, GEng::VulkanBuffer,
                    "Cannot copy from source to destination buffer if they are different sizes")

        VulkanOneTimeCommandBuffer cmdBuffer(ctx->GetCommandPool());
        cmdBuffer.Begin();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = mBufferSize;
        vkCmdCopyBuffer(cmdBuffer.GetBuffer(),
                        mBuffer, destinationBuffer->mBuffer, 1, &copyRegion);

        cmdBuffer.EndAndSubmit(ctx->GetGraphicsQueue());
    }
    //
    void VulkanBuffer::Copy(VulkanBuffer *destinationBuffer, VkDeviceSize byteSize, VkDeviceSize offset) {
        VulkanGPUContext *ctx = GPUContext::Get<VulkanGPUContext>();

        GENG_ASSERT(byteSize > destinationBuffer->mBufferSize || byteSize > mBufferSize, GEng::VulkanBuffer,
                    "Copy command failed. Size to copy bigger than source or destination buffer")

        VulkanOneTimeCommandBuffer cmdBuffer(ctx->GetCommandPool());
        cmdBuffer.Begin();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = offset;
        copyRegion.dstOffset = offset;
        copyRegion.size = byteSize;
        vkCmdCopyBuffer(cmdBuffer.GetBuffer(),
                        mBuffer, destinationBuffer->mBuffer, 1, &copyRegion);

        cmdBuffer.EndAndSubmit(ctx->GetGraphicsQueue());
    }
    //
    void VulkanBuffer::UpdateData(VulkanStagingBuffer *srcBuf, VkDeviceSize size) {
        srcBuf->Copy(this, size);
    }
    //
    void VulkanBuffer::CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                          VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VulkanGPUContext *ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer \n");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ctx->FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate buffer memory \n");

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }
    //
    //
    void VulkanBufferMappable::Map(void **hostMem, VkDeviceSize offset) {
        VulkanGPUContext *ctx = GPUContext::Get<VulkanGPUContext>();
        vkMapMemory(ctx->GetDevice(), mBufferMemory, offset, mBufferSize, 0, hostMem);
    }
    //
    void VulkanBufferMappable::Unmap() {
        VulkanGPUContext *ctx = GPUContext::Get<VulkanGPUContext>();
        vkUnmapMemory(ctx->GetDevice(), mBufferMemory);
    }
    //
    void VulkanBufferMappable::StoreData(const void *data, size_t byteSize, VkDeviceSize offset) {
        void *mappedMemory = nullptr;
        Map(&mappedMemory, offset);
        memcpy(mappedMemory, data, byteSize);
        Unmap();
    }
    //
    //
    VulkanStagingBuffer::VulkanStagingBuffer(VkDeviceSize size, DataType dataType): VulkanBufferMappable(
        size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dataType) {
    }
    //
    //
    VulkanVertexBuffer::VulkanVertexBuffer(VkDeviceSize size, DataType dataType): VulkanBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dataType) {
        GENG_ASSERT([dataType]() {
                    switch(dataType) {
                    case DataType::Float64:
                    case DataType::Float32:
                    return false;

                    default:
                    return true;
                    }
                    }(), GEng::VulkanVertexBuffer, "Vertex Buffer can only hold floating data types")
    }
    //
    //
    VulkanUniformBuffer::VulkanUniformBuffer(VkDeviceSize size): VulkanBufferMappable(
        size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        DataType::Void) {
    }
    //
    VulkanUniformBuffer::VulkanUniformBuffer(const void *data, VkDeviceSize size): VulkanUniformBuffer(size) {
        StoreData(data, size);
    }
    //
    //
    VulkanVertexBuffer::VulkanVertexBuffer(const void *data, VkDeviceSize size, DataType dataType): VulkanVertexBuffer(
        size, dataType) {
        VulkanStagingBuffer stagingBuffer(size);
        stagingBuffer.StoreData(data, size, 0);
        stagingBuffer.Copy(this);
    }
    //
    //
    VulkanIndexBuffer::VulkanIndexBuffer(VkDeviceSize size, DataType dataType): VulkanBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dataType) {
        GENG_ASSERT([dataType]() {
                    switch(dataType) {
                    case DataType::UInt32:
                    case DataType::UInt16:
                    return false;

                    default:
                    return true;
                    }
                    }(), GEng::VulkanIndexBuffer, "Index Buffer can only hold unsigned int data types")

        mElementCount = size / [dataType]() {
            switch (dataType) {
                case DataType::UInt32:
                    return sizeof(uint32_t);
                case DataType::UInt16:
                    return sizeof(uint16_t);
                default:
                    return (unsigned long)1;
            }
        }();
    }
    //
    VulkanIndexBuffer::VulkanIndexBuffer(const void *data, VkDeviceSize size, DataType dataType): VulkanIndexBuffer(
        size, dataType) {
        VulkanStagingBuffer stagingBuffer(size);
        stagingBuffer.StoreData(data, size, 0);
        stagingBuffer.Copy(this);
    }
    //
    void VulkanIndexBuffer::UpdateData(VulkanStagingBuffer *srcBuf, VkDeviceSize size) {
        mElementCount = size / [this]() {
            switch (mDataType) {
                case DataType::UInt32:
                    return sizeof(uint32_t);
                case DataType::UInt16:
                    return sizeof(uint16_t);
                default:
                    return (unsigned long)1;
            }
        }();
        VulkanBuffer::UpdateData(srcBuf, size);
    }
    //
    //
} // GEng
