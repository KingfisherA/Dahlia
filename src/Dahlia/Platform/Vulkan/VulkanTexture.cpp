#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <backends/imgui_impl_vulkan.h>

#include "Dahlia/Graphics/FormatConversion.h"
#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanImGuiRenderer.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    static void WarnIfNotFilterable(VulkanGPUContext* ctx, VkFormat vkFormat, Format format)
    {
        static bool warned[256] = {};
        if (IsDepthFormat(format) || warned[(uint8_t)format] || ctx->SupportsLinearFilter(vkFormat))
            return;
        warned[(uint8_t)format] = true;
        DH_WARN(DH::VulkanTexture,
                "This device cannot linearly filter the texture's format (Format "
                  << (int)format
                  << "); linear samplers will not interpolate it. Format::RGBA16F is filterable "
                     "everywhere.");
    }

    //
    VulkanImage& AsVulkanImage(Texture& texture)
    {
        if (texture.GetType() == TextureType::Texture3D)
            return static_cast<VulkanTexture3D&>(texture);
        return static_cast<VulkanTexture2D&>(texture);
    }

    const VulkanImage& AsVulkanImage(const Texture& texture)
    {
        if (texture.GetType() == TextureType::Texture3D)
            return static_cast<const VulkanTexture3D&>(texture);
        return static_cast<const VulkanTexture2D&>(texture);
    }

    VulkanImage::~VulkanImage()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        std::vector<VkImageView> views;
        views.reserve(mAttachmentViews.size() + 1);
        views.push_back(mView);
        for (const auto& [key, view] : mAttachmentViews)
            views.push_back(view);

        ctx->DeferDestroy(
          [device = ctx->GetDevice(), image = mImage, memory = mMemory, views = std::move(views)]()
          {
              for (VkImageView view : views)
                  vkDestroyImageView(device, view, nullptr);
              vkDestroyImage(device, image, nullptr);
              vkFreeMemory(device, memory, nullptr);
          });
    }

    void VulkanImage::CreateImage(Format format, const ImageInfo& info)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        mVkFormat = ctx->ToTextureFormat(format);
        mAspect = VK::ToAspect(format);
        mImageType = info.type;
        mMipLevels = info.mipLevels;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = info.flags;
        imageInfo.imageType = info.type;
        imageInfo.extent = info.extent;
        imageInfo.mipLevels = info.mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = mVkFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = info.usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        DH_VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &mImage), "vkCreateImage");

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device, mImage, &requirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex =
          ctx->FindMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        DH_VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &mMemory), "vkAllocateMemory");
        vkBindImageMemory(device, mImage, mMemory, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = mImage;
        viewInfo.viewType = info.viewType;
        viewInfo.format = mVkFormat;
        // Sampling views only see depth; the stencil aspect is used through attachment views
        viewInfo.subresourceRange = { IsDepthFormat(format) ? (VkImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT
                                                            : mAspect,
                                      0,
                                      mMipLevels,
                                      0,
                                      1 };
        DH_VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &mView), "vkCreateImageView");
    }

    VkImageView VulkanImage::GetAttachmentView(uint32_t mipLevel, uint32_t layer)
    {
        const uint64_t key = ((uint64_t)mipLevel << 32) | layer;
        if (auto it = mAttachmentViews.find(key); it != mAttachmentViews.end())
            return it->second;

        // A 2D view of a 3D image addresses its depth slices as array layers
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = mImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = mVkFormat;
        viewInfo.subresourceRange = { mAspect, mipLevel, 1, layer, 1 };

        VkImageView view = VK_NULL_HANDLE;
        DH_VK_CHECK(vkCreateImageView(
                      GPUContext::Get<VulkanGPUContext>()->GetDevice(), &viewInfo, nullptr, &view),
                    "vkCreateImageView (attachment)");
        mAttachmentViews.emplace(key, view);
        return view;
    }

    void VulkanImage::TransitionTo(VkCommandBuffer cmd, VkImageLayout layout)
    {
        if (mLayout == layout)
            return;
        VK::TransitionImage(cmd, mImage, mAspect, mLayout, layout);
        mLayout = layout;
    }

    void VulkanImage::UploadRegion(Format format, VkOffset3D offset, VkExtent3D extent, const void* data)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const size_t pixels = (size_t)extent.width * extent.height * extent.depth;

        // RGB data is stored as RGBA
        const std::vector<uint8_t> expanded = ExpandToStorageFormat(data, pixels, format);
        const void* source = expanded.empty() ? data : expanded.data();
        const size_t byteSize = pixels * FormatByteSize(TextureStorageFormat(format));

        VkBuffer staging = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        VulkanBuffer::CreateRaw(byteSize,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                staging,
                                stagingMemory);
        void* mapped = nullptr;
        vkMapMemory(ctx->GetDevice(), stagingMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
        std::memcpy(mapped, source, byteSize);
        vkUnmapMemory(ctx->GetDevice(), stagingMemory);

        ctx->SubmitImmediate(
          [&](VkCommandBuffer cmd)
          {
              TransitionTo(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

              VkBufferImageCopy region{};
              region.imageSubresource = { mAspect, 0, 0, 1 };
              region.imageOffset = offset;
              region.imageExtent = extent;
              vkCmdCopyBufferToImage(
                cmd, staging, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

              TransitionTo(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          });

        vkDestroyBuffer(ctx->GetDevice(), staging, nullptr);
        vkFreeMemory(ctx->GetDevice(), stagingMemory, nullptr);
    }

    //
    VulkanTexture2D::VulkanTexture2D(const Dimension2D& dimensions,
                                     const byte* data,
                                     Format format,
                                     TextureUsage usage)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const VkFormat vkFormat = ctx->ToTextureFormat(format);
        WarnIfNotFilterable(ctx, vkFormat, format);

        ImageInfo info;
        info.extent = { (uint32_t)dimensions.x, (uint32_t)dimensions.y, 1 };
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (usage & TextureUsage::RenderTarget)
            info.usage |= IsDepthFormat(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (ctx->SupportsLinearBlit(vkFormat))
            info.mipLevels =
              (uint32_t)std::floor(std::log2((double)std::max(dimensions.x, dimensions.y))) + 1;
        CreateImage(format, info);

        if (data != nullptr)
            UploadRegion(format,
                         { 0, 0, 0 },
                         { (uint32_t)dimensions.x, (uint32_t)dimensions.y, 1 },
                         data);
        else
            ctx->SubmitImmediate(
              [this](VkCommandBuffer cmd)
              { TransitionTo(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); });

        if (data != nullptr && mMipLevels > 1)
            GenerateMipMaps();
    }

    VulkanTexture2D::VulkanTexture2D(const Dimension2D& dimensions, Format format, bool)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(TextureUsage::SampledRenderTarget)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const bool depth = IsDepthFormat(format);

        ImageInfo info;
        info.extent = { (uint32_t)dimensions.x, (uint32_t)dimensions.y, 1 };
        info.usage = depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                           : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        CreateImage(format, info);

        // Colour targets rest in SHADER_READ_ONLY between passes so they can always be sampled
        ctx->SubmitImmediate(
          [this, depth](VkCommandBuffer cmd)
          {
              TransitionTo(cmd,
                           depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                 : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          });
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
        if (mImGuiSet == VK_NULL_HANDLE)
            return;
        // Queued before the image is destroyed (by ~VulkanImage). ImGui frees its descriptor pool
        // (and these sets) when it shuts down.
        GPUContext::Get<VulkanGPUContext>()->DeferDestroy(
          [imguiSet = mImGuiSet]()
          {
              if (VulkanImGuiRenderer::IsActive())
                  ImGui_ImplVulkan_RemoveTexture(imguiSet);
          });
    }

    void VulkanTexture2D::UpdateData(Dimension2D offset, Dimension2D size, const byte* data)
    {
        UploadRegion(mFormat,
                     { (int32_t)offset.x, (int32_t)offset.y, 0 },
                     { (uint32_t)size.x, (uint32_t)size.y, 1 },
                     data);
    }

    void VulkanTexture2D::RecordMipGeneration(VkCommandBuffer cmd)
    {
        TransitionTo(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        int32_t width = (int32_t)mDimensions.x, height = (int32_t)mDimensions.y;
        for (uint32_t level = 1; level < mMipLevels; level++)
        {
            // Previous level becomes the blit source
            VK::TransitionImage(cmd,
                                mImage,
                                mAspect,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                level - 1,
                                1);

            const int32_t nextWidth = std::max(width / 2, 1), nextHeight = std::max(height / 2, 1);
            VkImageBlit blit{};
            blit.srcSubresource = { mAspect, level - 1, 0, 1 };
            blit.srcOffsets[1] = { width, height, 1 };
            blit.dstSubresource = { mAspect, level, 0, 1 };
            blit.dstOffsets[1] = { nextWidth, nextHeight, 1 };
            vkCmdBlitImage(cmd,
                           mImage,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           mImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);

            VK::TransitionImage(cmd,
                                mImage,
                                mAspect,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                level - 1,
                                1);
            width = nextWidth;
            height = nextHeight;
        }

        VK::TransitionImage(cmd,
                            mImage,
                            mAspect,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            mMipLevels - 1,
                            1);
        mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    void VulkanTexture2D::GenerateMipMaps()
    {
        if (mMipLevels <= 1)
            return;
        GPUContext::Get<VulkanGPUContext>()->SubmitImmediate([this](VkCommandBuffer cmd)
                                                             { RecordMipGeneration(cmd); });
    }

    uint64_t VulkanTexture2D::GetImGuiID() const
    {
        if (mImGuiSet == VK_NULL_HANDLE)
        {
            mImGuiSet = ImGui_ImplVulkan_AddTexture(GPUContext::Get<VulkanGPUContext>()->GetDefaultSampler(),
                                                    mView,
                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        return (uint64_t)mImGuiSet;
    }

    //
    VulkanTexture3D::VulkanTexture3D(const Dimension3D& dimensions,
                                     const byte* data,
                                     Format format,
                                     TextureUsage usage)
      : mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        WarnIfNotFilterable(ctx, ctx->ToTextureFormat(format), format);

        ImageInfo info;
        info.type = VK_IMAGE_TYPE_3D;
        info.viewType = VK_IMAGE_VIEW_TYPE_3D;
        info.extent = { (uint32_t)dimensions.x, (uint32_t)dimensions.y, (uint32_t)dimensions.z };
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (usage & TextureUsage::RenderTarget)
        {
            if (!ctx->SupportsImageView2DOn3D())
                throw std::runtime_error(
                  "This Vulkan device cannot render into 3D textures (portability subset without "
                  "imageView2DOn3DImage)");
            info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            info.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        }
        CreateImage(format, info);

        if (data != nullptr)
            UploadRegion(format, { 0, 0, 0 }, info.extent, data);
        else
            ctx->SubmitImmediate(
              [this](VkCommandBuffer cmd)
              { TransitionTo(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); });
    }

    void VulkanTexture3D::UpdateData(Dimension3D offset, Dimension3D size, const byte* data)
    {
        UploadRegion(mFormat,
                     { (int32_t)offset.x, (int32_t)offset.y, (int32_t)offset.z },
                     { (uint32_t)size.x, (uint32_t)size.y, (uint32_t)size.z },
                     data);
    }

    //
    VulkanSampler::VulkanSampler(const SamplerDesc& desc)
      : mDesc(desc)
    {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK::ToFilter(desc.magFilter);
        info.minFilter = VK::ToFilter(desc.minFilter);
        info.mipmapMode = desc.mipmapMode == MipmapMode::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                                                                : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        info.addressModeU = VK::ToAddressMode(desc.addressU);
        info.addressModeV = VK::ToAddressMode(desc.addressV);
        info.addressModeW = VK::ToAddressMode(desc.addressW);
        info.borderColor = VK::ToBorderColor(desc.borderColour);
        info.minLod = 0.0f;
        // MipmapMode::None samples only the base level, as in OpenGL
        info.maxLod = desc.mipmapMode == MipmapMode::None ? 0.0f : VK_LOD_CLAMP_NONE;

        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        DH_VK_CHECK(vkCreateSampler(ctx->GetDevice(), &info, nullptr, &mSampler), "vkCreateSampler");
    }

    VulkanSampler::~VulkanSampler()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        ctx->DeferDestroy([device = ctx->GetDevice(), sampler = mSampler]()
                          { vkDestroySampler(device, sampler, nullptr); });
    }
}
