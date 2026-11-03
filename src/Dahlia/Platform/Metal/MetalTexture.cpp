#include <algorithm>
#include <cmath>

#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"

namespace DH
{
    static bool CanGenerateMips(Format format)
    {
        // Filterable, colour renderable 8 bit formats on every Mac GPU
        const Format storage = TextureStorageFormat(format);
        return storage == Format::R8 || storage == Format::RG8 || storage == Format::RGBA8 ||
               storage == Format::BGRA8;
    }

    MetalTexture2D::MetalTexture2D(const Dimension2D& dimensions, const byte* data, Format format)
      : mDimensions(dimensions)
      , mFormat(format)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        const bool mipmapped = CanGenerateMips(format);

        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
          MTLConv::ToPixelFormat(format, device), dimensions.x, dimensions.y, mipmapped);
        desc->setUsage(MTL::TextureUsageShaderRead);
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
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
          MTLConv::ToPixelFormat(format, device), dimensions.x, dimensions.y, false);
        desc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
        desc->setStorageMode(MTL::StorageModePrivate);
        mTexture = device->newTexture(desc);
    }

    MetalTexture2D::~MetalTexture2D()
    {
        mTexture->release();
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

        mSampler =
          GPUContext::Get<MetalGPUContext>()->GetMetalDevice()->newSamplerState(samplerDesc);
        samplerDesc->release();
    }

    MetalSampler::~MetalSampler()
    {
        mSampler->release();
    }
}
