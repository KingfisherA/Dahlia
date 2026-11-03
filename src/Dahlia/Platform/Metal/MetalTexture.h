#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Texture.h"

namespace MTL
{
    class Texture;
    class SamplerState;
}

namespace DH
{
    class DH_API MetalTexture2D : public Texture2D
    {
      public:
        // Sampled texture with a full mip chain for 8 bit formats (generated on creation and by
        // GenerateMipMaps)
        MetalTexture2D(const Dimension2D& dimensions, const byte* data, Format format);
        // Render target attachment (private storage)
        MetalTexture2D(const Dimension2D& dimensions, Format format, bool renderTarget);
        MetalTexture2D(const MetalTexture2D&) = delete;
        ~MetalTexture2D() override;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] void* GetNativeHandle() const override { return mTexture; }
        // ImGui's Metal backend takes the id<MTLTexture> directly
        [[nodiscard]] uint64_t GetImGuiID() const override { return (uint64_t)mTexture; }

        [[nodiscard]] MTL::Texture* GetMetalTexture() const { return mTexture; }

      private:
        Dimension2D mDimensions;
        Format mFormat;
        MTL::Texture* mTexture = nullptr;
    };

    class DH_API MetalSampler : public Sampler
    {
      public:
        explicit MetalSampler(const SamplerDesc& desc);
        MetalSampler(const MetalSampler&) = delete;
        ~MetalSampler() override;

        [[nodiscard]] const SamplerDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] MTL::SamplerState* GetSamplerState() const { return mSampler; }

      private:
        SamplerDesc mDesc;
        MTL::SamplerState* mSampler = nullptr;
    };
}
