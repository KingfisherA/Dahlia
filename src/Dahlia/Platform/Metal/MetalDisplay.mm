// MetalDisplay.cpp

#include <Metal/Metal.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "MetalDisplay.h"
#include "QuartzCore/CAMetalLayer.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace DH
{
    MetalDisplay::MetalDisplay(GPUContext* ctx, const DisplayProps& props)
      : GLFWDisplayBase()
    {
        if (ctx->GetContextAPI() != GraphicsAPI::Metal)
            throw std::runtime_error("Cannot create a Metal display in a non-Metal GPU Context");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWDisplayBase::CreateDisplay(props);

        MetalGPUContext* mtlCtx = reinterpret_cast<MetalGPUContext*>(ctx);

        mtlLayer = CA::MetalLayer::layer();
        mtlLayer->setDevice(mtlCtx->GetMetalDevice());
        mtlLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        mtlLayer->setDrawableSize(CGSize{ (float)mWidth, (float)mHeight });

        // Bind layer to window surface
        NSWindow* cocoa_window = glfwGetCocoaWindow(mWindow);
        CAMetalLayer* native_layer = (__bridge CAMetalLayer*)mtlLayer;
        [[cocoa_window contentView] setLayer:native_layer];
        [native_layer setMaximumDrawableCount:2];
        [[cocoa_window contentView] setWantsLayer:YES];
        [[cocoa_window contentView] setNeedsLayout:YES];
    }
}