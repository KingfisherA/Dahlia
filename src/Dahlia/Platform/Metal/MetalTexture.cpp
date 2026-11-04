#include <algorithm>
#include <cmath>

#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    static bool CanGenerateMips(Format format)
    {
        // Filterable, colour renderable 8 bit formats on every Mac GPU
        const Format storage = TextureStorageFormat(format);
        return storage == Format::R8 || storage == Format::RG8 || storage == Format::RGBA8 ||
               storage == Format::BGRA8;
    }

    static MTL::TextureUsage ToTextureUsage(TextureUsage usage)
    {
        MTL::TextureUsage mtlUsage = MTL::TextureUsageShaderRead;
        if (usage & TextureUsage::RenderTarget)
            mtlUsage |= MTL::TextureUsageRenderTarget;
        return mtlUsage;
    }

    // Apple GPUs before Apple9 (M3) cannot filter 32 bit float textures
    static void WarnIfNotFilterable(MTL::Device* device, Format format)
    {
        static bool warned = false;
        const Format storage = TextureStorageFormat(format);
        const bool float32 = storage == Format::R32F || storage == Format::RG32F ||
                             storage == Format::RGBA32F;
        if (warned || !float32 || device->supports32BitFloatFiltering())
            return;
        warned = true;
        DH_WARN(DH::MetalTexture,
                "This GPU cannot linearly filter 32 bit float textures; linear samplers will not "
                "interpolate them. Format::RGBA16F is filterable everywhere.");
    }

    const MetalTextureBase& AsMetalTexture(const Texture& texture)
    {
        if (texture.GetType() == TextureType::Texture3D)
            return static_cast<const MetalTexture3D&>(texture);
        return static_cast<const MetalTexture2D&>(texture);
    }

    MetalTextureBase::~MetalTextureBase()
    {
        if (mTexture != nullptr)
            mTexture->release();
    }

    //
    MetalTexture2D::MetalTexture2D(const Dimension2D& dimensions,
                                   const byte* data,
                                   Format format,
                                   TextureUsage usage)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        WarnIfNotFilterable(device, format);
        const bool mipmapped = CanGenerateMips(format);

        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
          MTLConv::ToPixelFormat(format, device), dimensions.x, dimensions.y, mipmapped);
        desc->setUsage(ToTextureUsage(usage));
        desc->setStorageMode(device->hasUnifiedMemory() ? MTL::StorageModeShared
                                                        : MTL::StorageModeManaged);
        mTexture = device->newTexture(desc);

        if (data != nullptr)
        {
            UpdateData({ 0, 0 }, dimensions, data);
            if (mipmapped)
                GenerateMipMaps();
        }
    }

    MetalTexture2D::MetalTexture2D(const Dimension2D& dimensions, Format format, bool)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(TextureUsage::SampledRenderTarget)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
          MTLConv::ToPixelFormat(format, device), dimensions.x, dimensions.y, false);
        desc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
        desc->setStorageMode(MTL::StorageModePrivate);
        mTexture = device->newTexture(desc);
    }

    uint32_t MetalTexture2D::GetMipLevelCount() const
    {
        return (uint32_t)mTexture->mipmapLevelCount();
    }

    void MetalTexture2D::UpdateData(Dimension2D offset, Dimension2D size, const byte* data)
    {
        // Frames in flight may still sample this texture
        GPUContext::Get<MetalGPUContext>()->WaitIdle();

        const std::vector<uint8_t> expanded = ExpandToStorageFormat(data, size.x * size.y, mFormat);
        const void* source = expanded.empty() ? data : expanded.data();
        const size_t bytesPerRow = size.x * FormatByteSize(TextureStorageFormat(mFormat));

        mTexture->replaceRegion(
          MTL::Region::Make2D(offset.x, offset.y, size.x, size.y), 0, source, bytesPerRow);
    }

    void MetalTexture2D::GenerateMipMaps()
    {
        if (mTexture->mipmapLevelCount() <= 1)
            return;
        GPUContext::Get<MetalGPUContext>()->SubmitBlit([this](MTL::BlitCommandEncoder* blit)
                                                       { blit->generateMipmaps(mTexture); });
    }

    //
    MetalTexture3D::MetalTexture3D(const Dimension3D& dimensions,
                                   const byte* data,
                                   Format format,
                                   TextureUsage usage)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        WarnIfNotFilterable(device, format);

        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
        desc->setTextureType(MTL::TextureType3D);
        desc->setPixelFormat(MTLConv::ToPixelFormat(format, device));
        desc->setWidth(dimensions.x);
        desc->setHeight(dimensions.y);
        desc->setDepth(dimensions.z);
        desc->setMipmapLevelCount(1);
        desc->setUsage(ToTextureUsage(usage));
        desc->setStorageMode(device->hasUnifiedMemory() ? MTL::StorageModeShared
                                                        : MTL::StorageModeManaged);
        mTexture = device->newTexture(desc);
        desc->release();

        if (mTexture == nullptr)
            throw std::runtime_error("Failed to create Metal 3D texture");

        if (data != nullptr)
            UpdateData({ 0, 0, 0 }, dimensions, data);
    }

    void MetalTexture3D::UpdateData(Dimension3D offset, Dimension3D size, const byte* data)
    {
        GPUContext::Get<MetalGPUContext>()->WaitIdle();

        const std::vector<uint8_t> expanded =
          ExpandToStorageFormat(data, size.x * size.y * size.z, mFormat);
        const void* source = expanded.empty() ? data : expanded.data();
        const size_t bytesPerRow = size.x * FormatByteSize(TextureStorageFormat(mFormat));

        mTexture->replaceRegion(
          MTL::Region::Make3D(offset.x, offset.y, offset.z, size.x, size.y, size.z),
          0,
          0,
          source,
          bytesPerRow,
          bytesPerRow * size.y);
    }

    //
    MetalSampler::MetalSampler(const SamplerDesc& desc)
      : mDesc(desc)
    {
        MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
        samplerDesc->setMinFilter(MTLConv::ToMinMagFilter(desc.minFilter));
        samplerDesc->setMagFilter(MTLConv::ToMinMagFilter(desc.magFilter));
        samplerDesc->setMipFilter(desc.mipmapMode == MipmapMode::None      ? MTL::SamplerMipFilterNotMipmapped
                                  : desc.mipmapMode == MipmapMode::Nearest ? MTL::SamplerMipFilterNearest
                                                                           : MTL::SamplerMipFilterLinear);
        samplerDesc->setSAddressMode(MTLConv::ToAddressMode(desc.addressU));
        samplerDesc->setTAddressMode(MTLConv::ToAddressMode(desc.addressV));
        samplerDesc->setRAddressMode(MTLConv::ToAddressMode(desc.addressW));
        samplerDesc->setBorderColor(MTLConv::ToBorderColor(desc.borderColour));

        mSampler =
          GPUContext::Get<MetalGPUContext>()->GetMetalDevice()->newSamplerState(samplerDesc);
        samplerDesc->release();
    }

    MetalSampler::~MetalSampler()
    {
        mSampler->release();
    }
}
