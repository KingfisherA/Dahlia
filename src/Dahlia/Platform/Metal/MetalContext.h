#pragma once

#include "Dahlia/Platform/GPUContext.h"

namespace MTL
{
    class Device;
}

namespace DH
{
    //
    class DH_API MetalGPUContext : public GPUContext
    {
      public:
        MetalGPUContext();
        MetalGPUContext(const MetalGPUContext&) = delete;
        MetalGPUContext(MetalGPUContext&&) noexcept = delete;

        virtual GraphicsAPI GetContextAPI() const override { return GraphicsAPI::Metal; }
        virtual void Reset() override {}

        virtual void* GetDevice() override { return reinterpret_cast<void*>(mtlDevice); }
        MTL::Device* GetMetalDevice() { return mtlDevice; }

        virtual void Clear(const glm::vec4& clearColour) override;

        virtual std::string GetVersionStr() override { return {}; }
        virtual std::string GetVendorStr() override { return {}; }
        virtual std::string GetShadingVersionStr() override { return {}; }

        virtual void SetFaceCulling(bool activate) override {}
        virtual void SetDepthTest(bool activate) override {}
        virtual void SetPolygonMode(const PolygonMode mode) override {}
        virtual void SetBlending(bool activate, const Blending mode = Blending::Alpha) override {}

        virtual void SetViewport(const glm::ivec4& bounds) override {}

        virtual void ActivateDefaultFramebuffer() override {}

        virtual void DestroyContext() override;

      private:
        MTL::Device* mtlDevice = nullptr;
    };
    //
} // namespace DH