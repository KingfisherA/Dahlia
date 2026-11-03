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

    TextureLoader::TextureLoader(const std::string_view filePath)
    {
        stbi_set_flip_vertically_on_load(0);
        int width = 0, height = 0, channels = 0;
        const std::string path(filePath);
        mData = (byte*)stbi_load(path.c_str(), &width, &height, &channels, 0);

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

    sRef<Texture2D> Texture2D::Create(const std::string_view filePath)
    {
        TextureLoader imageData(filePath);
        return Create(imageData);
    }

    sRef<Texture2D> Texture2D::Create(const TextureLoader& imageData)
    {
        return Create(imageData.GetDimensions(), imageData.GetData(), imageData.GetFormat());
    }

    sRef<Texture2D> Texture2D::Create(const Dimension2D& dimensions,
                                      const byte* data,
                                      const Format format)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLTexture2D>(dimensions, data, format);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanTexture2D>(dimensions, data, format);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalTexture2D>(dimensions, data, format);
#endif
            default:
                throw std::runtime_error("Texture2D is not implemented for this graphics API yet");
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
