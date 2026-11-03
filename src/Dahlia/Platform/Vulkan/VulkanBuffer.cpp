#include <algorithm>
#include <cstring>

#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"

namespace DH
{
    static VkBufferUsageFlags ToUsageFlags(BufferUsage usage)
    {
        VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (usage & BufferUsage::Vertex)
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (usage & BufferUsage::Index)
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (usage & BufferUsage::Uniform)
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        return flags;
    }

    void VulkanBuffer::CreateRaw(VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer& buffer,
                                 VkDeviceMemory& memory)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = std::max<VkDeviceSize>(size, 4);
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        DH_VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "vkCreateBuffer");

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device, buffer, &requirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = ctx->FindMemoryType(requirements.memoryTypeBits, properties);
        DH_VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &memory), "vkAllocateMemory");
        vkBindBufferMemory(device, buffer, memory, 0);
    }

    VulkanBuffer::VulkanBuffer(const BufferDesc& desc, const void* initialData)
      : mDesc(desc)
    {
        Allocate(initialData);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Release();
    }

    void VulkanBuffer::Allocate(const void* data)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();

        if (mDesc.memory == BufferMemory::Dynamic)
        {
            CreateRaw(mDesc.size,
                      ToUsageFlags(mDesc.usage),
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      mBuffer,
                      mMemory);
            vkMapMemory(ctx->GetDevice(), mMemory, 0, VK_WHOLE_SIZE, 0, &mMapped);
            if (data != nullptr)
                std::memcpy(mMapped, data, mDesc.size);
        }
        else
        {
            CreateRaw(mDesc.size,
                      ToUsageFlags(mDesc.usage),
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      mBuffer,
                      mMemory);
            if (data != nullptr)
                Update(data, mDesc.size, 0);
        }
    }

    void VulkanBuffer::Release()
    {
        if (mBuffer == VK_NULL_HANDLE)
            return;

        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        ctx->DeferDestroy(
          [device = ctx->GetDevice(), buffer = mBuffer, memory = mMemory]()
          {
              vkDestroyBuffer(device, buffer, nullptr);
              vkFreeMemory(device, memory, nullptr); // Also unmaps
          });
        mBuffer = VK_NULL_HANDLE;
        mMemory = VK_NULL_HANDLE;
        mMapped = nullptr;
    }

    void VulkanBuffer::Update(const void* data, size_t size, size_t offset)
    {
        if (offset + size > mDesc.size)
            throw std::out_of_range("VulkanBuffer::Update writes past the end of the buffer");
        if (size == 0)
            return;

        auto* ctx = GPUContext::Get<VulkanGPUContext>();

        if (mMapped != nullptr)
        {
            // Frames in flight may still read this memory
            vkQueueWaitIdle(ctx->GetGraphicsQueue());
            std::memcpy(static_cast<uint8_t*>(mMapped) + offset, data, size);
            return;
        }

        VkBuffer staging = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        CreateRaw(size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging,
                  stagingMemory);

        void* mapped = nullptr;
        vkMapMemory(ctx->GetDevice(), stagingMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
        std::memcpy(mapped, data, size);
        vkUnmapMemory(ctx->GetDevice(), stagingMemory);

        ctx->SubmitImmediate(
          [&](VkCommandBuffer cmd)
          {
              VkBufferCopy region{ 0, offset, size };
              vkCmdCopyBuffer(cmd, staging, mBuffer, 1, &region);
          });

        vkDestroyBuffer(ctx->GetDevice(), staging, nullptr);
        vkFreeMemory(ctx->GetDevice(), stagingMemory, nullptr);
    }

    void VulkanBuffer::Resize(size_t newSize, const void* data)
    {
        Release();
        mDesc.size = newSize;
        Allocate(data);
    }
}
