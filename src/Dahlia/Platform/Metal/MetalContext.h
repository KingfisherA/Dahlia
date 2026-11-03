#pragma once

#include <functional>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Platform/GPUContext.h"

namespace MTL
{
    class Device;
    class CommandQueue;
    class CommandBuffer;
    class BlitCommandEncoder;
}

namespace DH
{
    // Metal retains every resource a command buffer references, so resources can be released at
    // any time without deferred destruction.
    class DH_API MetalGPUContext : public GPUContext
    {
      public:
        MetalGPUContext();
        MetalGPUContext(const MetalGPUContext&) = delete;
        MetalGPUContext(MetalGPUContext&&) noexcept = delete;

        [[nodiscard]] GraphicsAPI GetContextAPI() const override { return GraphicsAPI::Metal; }

        [[nodiscard]] void* GetNativeDevice() override { return reinterpret_cast<void*>(mDevice); }
        [[nodiscard]] MTL::Device* GetMetalDevice() const { return mDevice; }
        [[nodiscard]] MTL::CommandQueue* GetCommandQueue() const { return mQueue; }
        [[nodiscard]] const GPULimits& GetLimits() const override { return mLimits; }

        // Waits for the most recently committed command buffer (and so all earlier ones)
        void WaitIdle() override;

        std::string GetVersionStr() override;
        std::string GetVendorStr() override;
        std::string GetShadingVersionStr() override;

        void DestroyContext() override;

        // Called by the display when a frame's command buffer is committed
        void SetLastCommandBuffer(MTL::CommandBuffer* commandBuffer);

        // Encodes blit work in a one-off command buffer and waits for it to finish
        void SubmitBlit(const std::function<void(MTL::BlitCommandEncoder*)>& encode);

      private:
        MTL::Device* mDevice = nullptr;
        MTL::CommandQueue* mQueue = nullptr;
        MTL::CommandBuffer* mLastCommandBuffer = nullptr;
        GPULimits mLimits;
    };
}
