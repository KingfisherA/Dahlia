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
    // The MTLTexture shared by every Metal texture type
    class DH_API MetalTextureBase
    {
      public:
        [[nodiscard]] MTL::Texture* GetMetalTexture() const { return mTexture; }

      protected:
        MetalTextureBase() = default;
        ~MetalTextureBase();

        MTL::Texture* mTexture = nullptr;
    };

    // The Metal part of any texture created by the Metal backend
    [[nodiscard]] DH_API const MetalTextureBase& AsMetalTexture(const Texture& texture);

    class DH_API MetalTexture2D : public Texture2D, public MetalTextureBase
    {
      public:
        // Full mip chain for 8 bit formats (generated on creation and by GenerateMipMaps)
        MetalTexture2D(const Dimension2D& dimensions,
                       const byte* data,
                       Format format,
                       TextureUsage usage = TextureUsage::Sampled);
        // Framebuffer-owned attachment (private storage)
        MetalTexture2D(const Dimension2D& dimensions, Format format, bool renderTarget);
        MetalTexture2D(const MetalTexture2D&) = delete;
        ~MetalTexture2D() override = default;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] uint32_t GetMipLevelCount() const override;
        [[nodiscard]] void* GetNativeHandle() const override { return mTexture; }
        // ImGui's Metal backend takes the id<MTLTexture> directly
        [[nodiscard]] uint64_t GetImGuiID() const override { return (uint64_t)mTexture; }

      private:
        Dimension2D mDimensions;
        Format mFormat;
        TextureUsage mUsage;
    };

    // MTLTextureType3D. Render targets draw into one depth plane per framebuffer attachment.
    class DH_API MetalTexture3D : public Texture3D, public MetalTextureBase
    {
      public:
        MetalTexture3D(const Dimension3D& dimensions,
                       const byte* data,
                       Format format,
                       TextureUsage usage = TextureUsage::Sampled);
        MetalTexture3D(const MetalTexture3D&) = delete;
        ~MetalTexture3D() override = default;

        void UpdateData(Dimension3D offset, Dimension3D size, const byte* data) override;

        [[nodiscard]] const Dimension3D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] void* GetNativeHandle() const override { return mTexture; }

      private:
        Dimension3D mDimensions;
        Format mFormat;
        TextureUsage mUsage;
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
