#pragma once

#include <cstdint>
#include <string_view>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class DH_API TextureLoader
    {
      public:
        using byte = unsigned char;

        // desiredChannels: 0 keeps the file's channel count; 1-4 converts on load (4 gives RGBA8,
        // avoiding the RGB -> RGBA expansion Vulkan and Metal otherwise do on upload)
        explicit TextureLoader(std::string_view filePath, int desiredChannels = 0);
        TextureLoader(const TextureLoader&) = delete;
        ~TextureLoader();

        [[nodiscard]] byte* GetData() { return mData; }
        [[nodiscard]] const byte* GetData() const { return mData; }
        [[nodiscard]] const Dimension2D& GetDimensions() const { return mImageDimensions; }
        [[nodiscard]] Format GetFormat() const { return mImageFormat; }

      private:
        byte* mData = nullptr;
        Dimension2D mImageDimensions{};
        Format mImageFormat = Format::None;
    };

    // Filtering/addressing state is kept in a separate Sampler object (as in Vulkan, Metal and
    // OpenGL 3.3+ sampler objects) and paired with a texture in a BindGroup.
    struct DH_API SamplerDesc
    {
        Filter minFilter = Filter::Linear;
        Filter magFilter = Filter::Linear;
        MipmapMode mipmapMode = MipmapMode::None;
        AddressMode addressU = AddressMode::ClampToEdge;
        AddressMode addressV = AddressMode::ClampToEdge;
        AddressMode addressW = AddressMode::ClampToEdge; // 3D textures
        // Used by AddressMode::ClampToBorder
        BorderColour borderColour = BorderColour::TransparentBlack;
    };

    class DH_API Sampler
    {
      public:
        virtual ~Sampler() = default;

        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;

        static sRef<Sampler> Create(const SamplerDesc& desc = {});
    };

    // Common base of every texture type. Framebuffer attachments and bind group entries take a
    // Texture so 2D and 3D textures can be used interchangeably.
    class DH_API Texture
    {
      public:
        using byte = unsigned char;

        virtual ~Texture() = default;

        [[nodiscard]] virtual TextureType GetType() const = 0;
        [[nodiscard]] virtual Format GetFormat() const = 0;
        [[nodiscard]] virtual TextureUsage GetUsage() const = 0;
        [[nodiscard]] virtual uint32_t GetMipLevelCount() const = 0;
        // {width, height, depth}; depth is 1 for 2D textures
        [[nodiscard]] virtual Dimension3D GetExtent() const = 0;

        // Size of mip level `level` (each dimension halves, down to 1)
        [[nodiscard]] Dimension3D GetMipExtent(uint32_t level) const;

        // OpenGL: texture name, Vulkan: VkImageView, Metal: MTL::Texture*
        [[nodiscard]] virtual void* GetNativeHandle() const = 0;
    };

    class DH_API Texture2D : public Texture
    {
      public:
        // Generate the mip chain from the base level. Sample it with a Sampler whose mipmapMode is
        // not None.
        virtual void GenerateMipMaps() = 0;

        // Upload a sub-region of the base level. `data` must be in the texture's format.
        virtual void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) = 0;

        [[nodiscard]] virtual const Dimension2D& GetDimensions() const = 0;

        // Value to pass to ImGui::Image for this texture with the active ImGui renderer backend
        [[nodiscard]] virtual uint64_t GetImGuiID() const = 0;

        [[nodiscard]] TextureType GetType() const final { return TextureType::Texture2D; }
        [[nodiscard]] Dimension3D GetExtent() const final
        {
            return { GetDimensions().x, GetDimensions().y, 1 };
        }

        [[nodiscard]] size_t GetWidth() const { return GetDimensions().x; }
        [[nodiscard]] size_t GetHeight() const { return GetDimensions().y; }
        [[nodiscard]] double GetAspect() const
        {
            return (double)GetDimensions().x / (double)GetDimensions().y;
        }
        [[nodiscard]] double GetInvAspect() const
        {
            return (double)GetDimensions().y / (double)GetDimensions().x;
        }

        // desiredChannels as for TextureLoader
        static sRef<Texture2D> Create(std::string_view filePath, int desiredChannels = 0);
        static sRef<Texture2D> Create(const TextureLoader& imageData);
        // `data` may be nullptr (contents undefined). A full mip chain is allocated where the
        // backend can generate one for the format (see GetMipLevelCount); it is filled from
        // `data` on creation, by GenerateMipMaps, or by rendering into each level.
        static sRef<Texture2D> Create(const Dimension2D& dimensions,
                                      const byte* data,
                                      Format format,
                                      TextureUsage usage = TextureUsage::Sampled);
    };

    // A volume texture sampled with sampler3D (ShaderResourceType::CombinedTextureSampler3D).
    // It has a single mip level. With TextureUsage::RenderTarget each depth slice can be rendered
    // into with FramebufferAttachment{ texture, 0, slice }.
    class DH_API Texture3D : public Texture
    {
      public:
        // Upload a sub-volume. `data` is tightly packed in the texture's format, x fastest.
        virtual void UpdateData(Dimension3D offset, Dimension3D size, const byte* data) = 0;

        [[nodiscard]] virtual const Dimension3D& GetDimensions() const = 0;

        [[nodiscard]] TextureType GetType() const final { return TextureType::Texture3D; }
        [[nodiscard]] Dimension3D GetExtent() const final { return GetDimensions(); }
        [[nodiscard]] uint32_t GetMipLevelCount() const final { return 1; }

        [[nodiscard]] size_t GetWidth() const { return GetDimensions().x; }
        [[nodiscard]] size_t GetHeight() const { return GetDimensions().y; }
        [[nodiscard]] size_t GetDepth() const { return GetDimensions().z; }

        // `data` may be nullptr (contents undefined)
        static sRef<Texture3D> Create(const Dimension3D& dimensions,
                                      const byte* data,
                                      Format format,
                                      TextureUsage usage = TextureUsage::Sampled);
    };
}
