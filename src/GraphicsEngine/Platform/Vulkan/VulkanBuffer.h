#pragma once
#include<vulkan/vulkan.h>

#include "GraphicsEngine/3D/stb/stb_image.h"

namespace GEng {
    class VulkanStagingBuffer;

    class VulkanBuffer {
    public:
        enum class BufferType {
            VertexBuffer = 0,
            IndexBuffer,
            StagingBuffer,
            UniformBuffer
        };

        enum class DataType {
            Void = 0,
            Float32,
            Float64,
            UInt16,
            UInt32
        };

        VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties, DataType dataType);
        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer(VulkanBuffer &&) = delete;

        virtual ~VulkanBuffer();

        [[nodiscard]] VkBuffer GetBuffer() const { return mBuffer; }
        [[nodiscard]] VkDeviceSize GetBufferSize() const { return mBufferSize; }
        [[nodiscard]] VkDeviceMemory GetBufferMemory() const { return mBufferMemory; }
        [[nodiscard]] virtual BufferType GetBufferType() const =0;
        [[nodiscard]] DataType GetDataType() const { return mDataType; }

        void Copy(VulkanBuffer *destinationBuffer);
        void Copy(VulkanBuffer *destinationBuffer, VkDeviceSize byteSize, VkDeviceSize offset = 0);

        virtual void UpdateData(VulkanStagingBuffer *srcBuf, VkDeviceSize size = 0);

    protected:
        static void CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                       VkBuffer &buffer, VkDeviceMemory &bufferMemory);

        VkDeviceSize mBufferSize = 0;
        VkBuffer mBuffer = nullptr;
        VkDeviceMemory mBufferMemory = nullptr;
        DataType mDataType;
    };

    class VulkanBufferMappable : public VulkanBuffer {
    public:
        using VulkanBuffer::VulkanBuffer;

        void Map(void **hostMem, VkDeviceSize offset = 0);
        void Unmap();

        void StoreData(const void *data, size_t byteSize, VkDeviceSize offset = 0);
    };

    class VulkanStagingBuffer : public VulkanBufferMappable {
    public:
        explicit VulkanStagingBuffer(VkDeviceSize size, DataType dataType = DataType::Void);
        VulkanStagingBuffer(const VulkanStagingBuffer &) = delete;
        VulkanStagingBuffer(VulkanStagingBuffer &&) = delete;

        [[nodiscard]] BufferType GetBufferType() const override { return BufferType::StagingBuffer; }
    };

    class VulkanUniformBuffer : public VulkanBufferMappable {
    public:
        explicit VulkanUniformBuffer(VkDeviceSize size);
        VulkanUniformBuffer(const void *data, VkDeviceSize size);
        VulkanUniformBuffer(const VulkanUniformBuffer &) = delete;
        VulkanUniformBuffer(VulkanUniformBuffer &&) = delete;

        [[nodiscard]] BufferType GetBufferType() const override { return BufferType::UniformBuffer; }
    };

    class VulkanVertexBuffer : public VulkanBuffer {
    public:
        explicit VulkanVertexBuffer(VkDeviceSize size, DataType dataType = DataType::Float32);
        VulkanVertexBuffer(const void *data, VkDeviceSize size, DataType dataType = DataType::Float32);
        VulkanVertexBuffer(const VulkanVertexBuffer &) = delete;
        VulkanVertexBuffer(VulkanVertexBuffer &&) = delete;

        [[nodiscard]] BufferType GetBufferType() const override { return BufferType::VertexBuffer; }
    };

    class VulkanIndexBuffer : public VulkanBuffer {
    public:
        explicit VulkanIndexBuffer(VkDeviceSize size, DataType dataType = DataType::UInt16);
        VulkanIndexBuffer(const void *data, VkDeviceSize size, DataType dataType = DataType::UInt16);
        VulkanIndexBuffer(const VulkanIndexBuffer &) = delete;
        VulkanIndexBuffer(VulkanIndexBuffer &&) = delete;

        [[nodiscard]] BufferType GetBufferType() const override { return BufferType::IndexBuffer; }

        void UpdateData(VulkanStagingBuffer *srcBuf, VkDeviceSize size = 0) override;

        size_t GetElementCount() const { return mElementCount; }
        VkIndexType GetIndexType() const {
            switch (mDataType) {
                default:
                case DataType::UInt16:
                    return VK_INDEX_TYPE_UINT16;


                case DataType::UInt32:
                    return VK_INDEX_TYPE_UINT32;
            }
        }

    private:
        size_t mElementCount = 0;
    };
} // GEng
