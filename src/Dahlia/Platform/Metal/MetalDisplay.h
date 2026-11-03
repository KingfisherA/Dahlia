#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"
#include "Dahlia/Utilities/Memory.h"

namespace CA
{
    class MetalLayer;
    class MetalDrawable;
}

namespace MTL
{
    class CommandBuffer;
}

namespace NS
{
    class AutoreleasePool;
}

namespace DH
{
    class GPUContext;
    class MetalCommandList;
    class MetalTexture2D;

    // GLFW window backed by a CAMetalLayer. Each frame takes the next drawable, records into a
    // new command buffer and presents it.
    class DH_API MetalDisplay : public GLFWDisplayBase
    {
      public:
        static constexpr Format COLOUR_FORMAT = Format::BGRA8;
        static constexpr Format DEPTH_FORMAT = Format::D32F;

        MetalDisplay(GPUContext* ctx, const DisplayProps& props);
        ~MetalDisplay() override;

        [[nodiscard]] CommandList* BeginFrame() override;
        void EndFrame() override;

        [[nodiscard]] RenderTargetLayout GetRenderTargetLayout() const override
        {
            return { { COLOUR_FORMAT }, DEPTH_FORMAT, 1 };
        }

        [[nodiscard]] CA::MetalLayer* GetMetalLayer() const { return mLayer; }

      private:
        CA::MetalLayer* mLayer = nullptr;
        sRef<MetalTexture2D> mDepth;

        NS::AutoreleasePool* mFramePool = nullptr;
        CA::MetalDrawable* mDrawable = nullptr;
        MTL::CommandBuffer* mCommandBuffer = nullptr;

        uRef<MetalCommandList> mCommandList;
    };
}
