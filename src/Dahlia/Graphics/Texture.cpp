#include <algorithm>
#include <stdexcept>

#include "Dahlia/Graphics/Texture.h"
#include "Dahlia/Graphics/stb/stb_image.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLTexture.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalTexture.h"
#endif

namespace DH
{
#pragma region Texture Loader

    TextureLoader::TextureLoader(const std::string_view filePath, const int desiredChannels)
    {
        if (desiredChannels < 0 || desiredChannels > 4)
            throw std::invalid_argument("TextureLoader desiredChannels must be between 0 and 4");

        stbi_set_flip_vertically_on_load(0);
        int width = 0, height = 0, channels = 0;
        const std::string path(filePath);
        mData = (byte*)stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
        // stbi reports the file's channel count; the data has the requested count
        if (desiredChannels != 0)
            channels = desiredChannels;

        if (mData == nullptr)
            throw std::runtime_error("Image file cannot be loaded into memory: " + path);

        mImageDimensions = { (size_t)width, (size_t)height };
        mImageFormat = [channels]()
        {
            switch (channels)
            {
                case 1:
                    return Format::R8;
                case 2:
                    return Format::RG8;
                case 3:
                    return Format::RGB8;
                case 4:
                    return Format::RGBA8;
                default:
                    return Format::None;
            }
        }();
    }

    TextureLoader::~TextureLoader()
    {
        stbi_image_free(mData);
        mData = nullptr;
    }

#pragma endregion

#pragma region Texture2D

    Dimension3D Texture::GetMipExtent(uint32_t level) const
    {
        const Dimension3D extent = GetExtent();
        return { std::max<size_t>(extent.x >> level, 1),
                 std::max<size_t>(extent.y >> level, 1),
                 std::max<size_t>(extent.z >> level, 1) };
    }

    sRef<Texture2D> Texture2D::Create(const std::string_view filePath, const int desiredChannels)
    {
        TextureLoader imageData(filePath, desiredChannels);
        return Create(imageData);
    }

    sRef<Texture2D> Texture2D::Create(const TextureLoader& imageData)
    {
        return Create(imageData.GetDimensions(), imageData.GetData(), imageData.GetFormat());
    }

    static void ValidateTextureArgs(Format format, TextureUsage usage)
    {
        if (format == Format::None)
            throw std::invalid_argument("A texture needs a format");
        if (!(usage & TextureUsage::Sampled))
            throw std::invalid_argument("Textures must include TextureUsage::Sampled");
        if ((usage & TextureUsage::RenderTarget) &&
            (format == Format::RGB8 || format == Format::RGB32F))
            throw std::invalid_argument(
              "3 component formats cannot be render targets; use a 4 component format");
    }

    sRef<Texture2D> Texture2D::Create(const Dimension2D& dimensions,
                                      const byte* data,
                                      const Format format,
                                      const TextureUsage usage)
    {
        ValidateTextureArgs(format, usage);
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLTexture2D>(dimensions, data, format, usage);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanTexture2D>(dimensions, data, format, usage);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalTexture2D>(dimensions, data, format, usage);
#endif
            default:
                throw std::runtime_error("Texture2D is not implemented for this graphics API yet");
        }
    }

#pragma endregion

#pragma region Texture3D

    sRef<Texture3D> Texture3D::Create(const Dimension3D& dimensions,
                                      const byte* data,
                                      const Format format,
                                      const TextureUsage usage)
    {
        ValidateTextureArgs(format, usage);
        if (IsDepthFormat(format))
            throw std::invalid_argument("3D textures cannot have a depth format");
        if (dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0)
            throw std::invalid_argument("3D texture dimensions must be non-zero");

        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLTexture3D>(dimensions, data, format, usage);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanTexture3D>(dimensions, data, format, usage);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalTexture3D>(dimensions, data, format, usage);
#endif
            default:
                throw std::runtime_error("Texture3D is not implemented for this graphics API yet");
        }
    }

#pragma endregion

#pragma region Sampler

    sRef<Sampler> Sampler::Create(const SamplerDesc& desc)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLSampler>(desc);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanSampler>(desc);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalSampler>(desc);
#endif
            default:
                throw std::runtime_error("Sampler is not implemented for this graphics API yet");
        }
    }

#pragma endregion
}
