// MetalDisplay.mm

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include "Dahlia/Platform/Metal/MetalCommandList.h"
#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalDisplay.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"

namespace DH
{
    MetalDisplay::MetalDisplay(GPUContext* ctx, const DisplayProps& props)
    {
        if (ctx->GetContextAPI() != GraphicsAPI::Metal)
            throw std::runtime_error("Cannot create a Metal display in a non-Metal GPU Context");
        auto* mtlCtx = static_cast<MetalGPUContext*>(ctx);

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        CreateDisplay(props);
        if (mWindow == nullptr)
            throw std::runtime_error("Failed to create a window for Metal");

        mLayer = CA::MetalLayer::layer()->retain();
        mLayer->setDevice(mtlCtx->GetMetalDevice());
        mLayer->setPixelFormat(MTLConv::ToPixelFormat(COLOUR_FORMAT, mtlCtx->GetMetalDevice()));
        mLayer->setFramebufferOnly(true);

        // Bind the layer to the window's content view
        NSWindow* cocoaWindow = glfwGetCocoaWindow(mWindow);
        CAMetalLayer* nativeLayer = (__bridge CAMetalLayer*)mLayer;
        nativeLayer.contentsScale = cocoaWindow.backingScaleFactor;
        [nativeLayer setMaximumDrawableCount:3];
        [[cocoaWindow contentView] setLayer:nativeLayer];
        [[cocoaWindow contentView] setWantsLayer:YES];

        mCommandList = umake<MetalCommandList>();

        GLFWDisplayBase::Show();
    }

    MetalDisplay::~MetalDisplay()
    {
        GPUContext::Get<MetalGPUContext>()->WaitIdle();
        mCommandList.reset();
        mDepth.reset();
        mLayer->release();
    }

    CommandList* MetalDisplay::BeginFrame()
    {
        const Dimension2D size = GetFramebufferSize();
        if (size.x == 0 || size.y == 0)
            return nullptr; // Minimised

        // Keep the drawables and depth buffer at the window's pixel size
        const CGSize drawableSize = mLayer->drawableSize();
        if ((size_t)drawableSize.width != size.x || (size_t)drawableSize.height != size.y)
            mLayer->setDrawableSize(CGSize{ (CGFloat)size.x, (CGFloat)size.y });
        if (!mDepth || mDepth->GetDimensions() != size)
            mDepth = make<MetalTexture2D>(size, DEPTH_FORMAT, true);

        mFramePool = NS::AutoreleasePool::alloc()->init();
        mDrawable = mLayer->nextDrawable();
        if (mDrawable == nullptr)
        {
            mFramePool->release();
            mFramePool = nullptr;
            return nullptr;
        }

        mCommandBuffer = GPUContext::Get<MetalGPUContext>()->GetCommandQueue()->commandBuffer();
        mCommandList->Begin(mCommandBuffer, mDrawable->texture(), mDepth->GetMetalTexture());
        return mCommandList.get();
    }

    void MetalDisplay::EndFrame()
    {
        if (mFramePool == nullptr)
            throw std::logic_error("EndFrame called without BeginFrame");

        mCommandBuffer->presentDrawable(mDrawable);
        mCommandBuffer->commit();
        GPUContext::Get<MetalGPUContext>()->SetLastCommandBuffer(mCommandBuffer);

        mDrawable = nullptr;
        mCommandBuffer = nullptr;
        mFramePool->release();
        mFramePool = nullptr;
    }
}
