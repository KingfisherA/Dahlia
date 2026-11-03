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

        explicit TextureLoader(std::string_view filePath);
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
    };

    class DH_API Sampler
    {
      public:
        virtual ~Sampler() = default;

        [[nodiscard]] virtual const SamplerDesc& GetDesc() const = 0;

        static sRef<Sampler> Create(const SamplerDesc& desc = {});
    };

    class DH_API Texture2D
    {
      public:
        using byte = unsigned char;

        virtual ~Texture2D() = default;

        // Generate the mip chain from the base level. Sample it with a Sampler whose mipmapMode is
        // not None.
        virtual void GenerateMipMaps() = 0;

        // Upload a sub-region of the base level. `data` must be in the texture's format.
        virtual void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) = 0;

        [[nodiscard]] virtual const Dimension2D& GetDimensions() const = 0;
        [[nodiscard]] virtual Format GetFormat() const = 0;

        // OpenGL: texture name, Vulkan: VkImageView, Metal: MTL::Texture*
        [[nodiscard]] virtual void* GetNativeHandle() const = 0;

        // Value to pass to ImGui::Image for this texture with the active ImGui renderer backend
        [[nodiscard]] virtual uint64_t GetImGuiID() const = 0;

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

        static sRef<Texture2D> Create(std::string_view filePath);
        static sRef<Texture2D> Create(const TextureLoader& imageData);
        static sRef<Texture2D> Create(const Dimension2D& dimensions, const byte* data, Format format);
    };
}
