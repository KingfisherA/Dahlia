#pragma once

#include <cstddef>
namespace MTL
{
    class Device;
    class Buffer;
}

namespace DH
{
    class GPUContext;

    enum class BufferStorageMode
    {
        Shared = 0
    };

    class MetalBuffer
    {
      public:
        MetalBuffer(GPUContext* ctx);
        ~MetalBuffer();

        void Store(const void* data, std::size_t length, BufferStorageMode mode);
        void Update(const void* data, std::size_t length, std::size_t offset);
        void Resize(std::size_t length, BufferStorageMode mode);
        //
        void Destroy();

        const std::size_t GetBufferSize() const { return mBufferSize; }
        MTL::Buffer* GetMetalBuffer() { return mtlBuffer; }

      private:
        MTL::Device* mDevice = nullptr;
        MTL::Buffer* mtlBuffer = nullptr;
        std::size_t mBufferSize = 0;
    };
}