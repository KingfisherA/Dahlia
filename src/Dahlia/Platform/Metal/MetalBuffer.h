#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Buffer.h"

namespace MTL
{
    class Buffer;
}

namespace DH
{
    // CPU writable buffer (shared storage on unified memory, managed otherwise) for both Static
    // and Dynamic memory.
    //
    // Update/Resize wait for submitted GPU work to finish before writing. As with Vulkan, an
    // Update made while a frame is being recorded is seen by the whole frame.
    class DH_API MetalBuffer : public Buffer
    {
      public:
        MetalBuffer(const BufferDesc& desc, const void* initialData);
        MetalBuffer(const MetalBuffer&) = delete;
        ~MetalBuffer() override;

        void Update(const void* data, size_t size, size_t offset = 0) override;
        void Resize(size_t newSize, const void* data = nullptr) override;

        [[nodiscard]] size_t GetSize() const override { return mDesc.size; }
        [[nodiscard]] const BufferDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] void* GetNativeHandle() const override { return mBuffer; }

        [[nodiscard]] MTL::Buffer* GetMetalBuffer() const { return mBuffer; }

      private:
        void Allocate(const void* data);

        BufferDesc mDesc;
        MTL::Buffer* mBuffer = nullptr;
    };
}
