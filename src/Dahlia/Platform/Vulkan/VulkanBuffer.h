#pragma once

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Buffer.h"

namespace DH
{
    // Static buffers live in device local memory and are written through a staging buffer;
    // Dynamic buffers are host visible and persistently mapped.
    //
    // Update/Resize wait for the GPU to finish all submitted work before writing, so they are
    // safe at any time but are not meant for per-draw data (use push constants). An Update made
    // while a frame is being recorded is seen by that whole frame, unlike OpenGL where it only
    // affects draws issued after it.
    class DH_API VulkanBuffer : public Buffer
    {
      public:
        VulkanBuffer(const BufferDesc& desc, const void* initialData);
        VulkanBuffer(const VulkanBuffer&) = delete;
        ~VulkanBuffer() override;

        void Update(const void* data, size_t size, size_t offset = 0) override;
        void Resize(size_t newSize, const void* data = nullptr) override;

        [[nodiscard]] size_t GetSize() const override { return mDesc.size; }
        [[nodiscard]] const BufferDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] void* GetNativeHandle() const override { return mBuffer; }

        [[nodiscard]] VkBuffer GetBuffer() const { return mBuffer; }

        // Creates a buffer + memory (shared with textures for staging uploads)
        static void CreateRaw(VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkBuffer& buffer,
                              VkDeviceMemory& memory);

      private:
        void Allocate(const void* data);
        void Release();

        BufferDesc mDesc;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
        void* mMapped = nullptr;
    };
}
