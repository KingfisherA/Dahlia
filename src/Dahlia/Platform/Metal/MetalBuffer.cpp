// Metal Buffer.cpp

#include "MetalBuffer.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Metal/MTLBuffer.hpp"
#include "Metal/MTLResource.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

#include <Metal/Metal.hpp>

namespace DH
{
    MTL::ResourceOptions ToMTLResourceStorageMode(BufferStorageMode mode)
    {
        switch (mode)
        {
            case BufferStorageMode::Shared:
                return MTL::ResourceStorageModeShared;
        }
    }
    //
    MetalBuffer::MetalBuffer(GPUContext* ctx)
    {
        if (ctx->GetContextAPI() != GraphicsAPI::Metal)
            throw std::runtime_error("a");

        MetalGPUContext* mtlCtx = reinterpret_cast<MetalGPUContext*>(ctx);
        mDevice = mtlCtx->GetMetalDevice();
    }
    //
    MetalBuffer::~MetalBuffer()
    {
        Destroy();
    }
    //
    void MetalBuffer::Store(const void* data, std::size_t length, BufferStorageMode mode)
    {
        if (mtlBuffer != nullptr)
            mtlBuffer->release();
        mtlBuffer = mDevice->newBuffer(data, length, ToMTLResourceStorageMode(mode));
        mBufferSize = length;
    }
    //
    void MetalBuffer::Update(const void* data, std::size_t length, std::size_t offset)
    {
        if (length + offset > mBufferSize)
            throw std::runtime_error("Update would exceed length of metal buffer. Resize first");
        memcpy(reinterpret_cast<char*>(mtlBuffer->contents()) + offset, data, length);
    }
    //
    void MetalBuffer::Resize(std::size_t length, BufferStorageMode mode)
    {
        if (mtlBuffer == nullptr)
            throw std::runtime_error("Cannot resize a null metal buffer");

        MTL::Buffer* newBuffer = mDevice->newBuffer(length, ToMTLResourceStorageMode(mode));

        if (length >= mBufferSize)
            memcpy(newBuffer->contents(), mtlBuffer->contents(), mBufferSize);

        mtlBuffer->release();
        mtlBuffer = newBuffer;
        mBufferSize = length;
    }
    //
    void MetalBuffer::Destroy()
    {
        if (mtlBuffer)
            mtlBuffer->release();
        mtlBuffer = nullptr;
        mBufferSize = 0;
    }

}