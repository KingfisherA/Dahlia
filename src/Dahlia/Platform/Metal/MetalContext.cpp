#include <stdexcept>

#include <GLFW/glfw3.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Platform/Metal/MetalContext.h"

namespace DH
{
    MetalGPUContext::MetalGPUContext()
    {
        mDevice = MTL::CreateSystemDefaultDevice();
        if (mDevice == nullptr)
            throw std::runtime_error("No Metal device is available");
        mQueue = mDevice->newCommandQueue();

        mLimits.maxTextureSize = 16384; // All Mac GPU families
        mLimits.maxColourAttachments = 8;
        mLimits.maxBindGroups = MAX_BIND_GROUPS;
        mLimits.maxPushConstantSize = MAX_PUSH_CONSTANT_SIZE;
        mLimits.textureOriginBottomLeft = false;
    }

    void MetalGPUContext::SetLastCommandBuffer(MTL::CommandBuffer* commandBuffer)
    {
        commandBuffer->retain();
        if (mLastCommandBuffer != nullptr)
            mLastCommandBuffer->release();
        mLastCommandBuffer = commandBuffer;
    }

    void MetalGPUContext::WaitIdle()
    {
        if (mLastCommandBuffer != nullptr)
            mLastCommandBuffer->waitUntilCompleted();
    }

    void MetalGPUContext::SubmitBlit(const std::function<void(MTL::BlitCommandEncoder*)>& encode)
    {
        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        MTL::CommandBuffer* commandBuffer = mQueue->commandBuffer();
        MTL::BlitCommandEncoder* blit = commandBuffer->blitCommandEncoder();
        encode(blit);
        blit->endEncoding();
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();
        pool->release();
    }

    std::string MetalGPUContext::GetVersionStr()
    {
        return mDevice->supportsFamily(MTL::GPUFamilyMetal3) ? "3" : "2";
    }

    std::string MetalGPUContext::GetVendorStr()
    {
        return mDevice->name()->utf8String();
    }

    std::string MetalGPUContext::GetShadingVersionStr()
    {
        return "MSL 2.4 (SPIR-V via SPIRV-Cross)";
    }

    void MetalGPUContext::DestroyContext()
    {
        WaitIdle();
        if (mLastCommandBuffer != nullptr)
            mLastCommandBuffer->release();
        mQueue->release();
        mDevice->release();

        glfwTerminate();
        delete this;
    }
}
