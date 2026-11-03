#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "Dahlia/Platform/Metal/MetalBuffer.h"
#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalContext.h"

namespace DH
{
    MetalBuffer::MetalBuffer(const BufferDesc& desc, const void* initialData)
      : mDesc(desc)
    {
        Allocate(initialData);
    }

    MetalBuffer::~MetalBuffer()
    {
        if (mBuffer != nullptr)
            mBuffer->release();
    }

    void MetalBuffer::Allocate(const void* data)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        const size_t size = std::max<size_t>(mDesc.size, 4);
        const MTL::ResourceOptions options = MTLConv::CPUWriteStorage(device);

        mBuffer = data != nullptr ? device->newBuffer(data, size, options)
                                  : device->newBuffer(size, options);
    }

    void MetalBuffer::Update(const void* data, size_t size, size_t offset)
    {
        if (offset + size > mDesc.size)
            throw std::out_of_range("MetalBuffer::Update writes past the end of the buffer");

        // Frames in flight may still read this memory
        GPUContext::Get<MetalGPUContext>()->WaitIdle();
        std::memcpy(static_cast<uint8_t*>(mBuffer->contents()) + offset, data, size);
        if (mBuffer->storageMode() == MTL::StorageModeManaged)
            mBuffer->didModifyRange(NS::Range::Make(offset, size));
    }

    void MetalBuffer::Resize(size_t newSize, const void* data)
    {
        mBuffer->release();
        mDesc.size = newSize;
        Allocate(data);
    }
}
