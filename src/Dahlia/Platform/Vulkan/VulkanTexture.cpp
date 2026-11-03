#include <algorithm>
#include <cmath>
#include <cstring>

#include <backends/imgui_impl_vulkan.h>

#include "Dahlia/Graphics/FormatConversion.h"
#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanImGuiRenderer.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    VulkanTexture2D::VulkanTexture2D(const Dimension2D& dimensions, const byte* data, Format format)
      : mDimensions(dimensions)
      , mFormat(format)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        mVkFormat = ctx->ToTextureFormat(format);
        mAspect = VK::ToAspect(format);

        if (ctx->SupportsLinearBlit(mVkFormat))
            mMipLevels =
              (uint32_t)std::floor(std::log2((double)std::max(dimensions.x, dimensions.y))) + 1;

        CreateImage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        if (data != nullptr)
            UploadBaseLevel({ 0, 0 }, dimensions, data);
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
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        mVkFormat = ctx->ToTextureFormat(format);
        mAspect = VK::ToAspect(format);

        const bool depth = IsDepthFormat(format);
        CreateImage(depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                          : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

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
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        ctx->DeferDestroy(
          [device = ctx->GetDevice(),
           image = mImage,
           memory = mMemory,
           view = mView,
           imguiSet = mImGuiSet]()
          {
              // ImGui frees its descriptor pool (and these sets) when it shuts down
              if (imguiSet != VK_NULL_HANDLE && VulkanImGuiRenderer::IsActive())
                  ImGui_ImplVulkan_RemoveTexture(imguiSet);
              vkDestroyImageView(device, view, nullptr);
              vkDestroyImage(device, image, nullptr);
              vkFreeMemory(device, memory, nullptr);
          });
    }

    void VulkanTexture2D::CreateImage(VkImageUsageFlags usage)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { (uint32_t)mDimensions.x, (uint32_t)mDimensions.y, 1 };
        imageInfo.mipLevels = mMipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = mVkFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
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
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = mVkFormat;
        // Sampling views only see depth; the stencil aspect is used through the attachment
        viewInfo.subresourceRange = { IsDepthFormat(mFormat) ? (VkImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT
                                                             : mAspect,
                                      0,
                                      mMipLevels,
                                      0,
                                      1 };
        DH_VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &mView), "vkCreateImageView");
    }

    void VulkanTexture2D::TransitionTo(VkCommandBuffer cmd, VkImageLayout layout)
    {
        if (mLayout == layout)
            return;
        VK::TransitionImage(cmd, mImage, mAspect, mLayout, layout);
        mLayout = layout;
    }

    void VulkanTexture2D::UploadBaseLevel(Dimension2D offset, Dimension2D size, const byte* data)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const size_t pixels = size.x * size.y;

        // RGB data is stored as RGBA
        const std::vector<uint8_t> expanded = ExpandToStorageFormat(data, pixels, mFormat);
        const void* source = expanded.empty() ? data : expanded.data();
        const size_t byteSize = pixels * FormatByteSize(TextureStorageFormat(mFormat));

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
              region.imageOffset = { (int32_t)offset.x, (int32_t)offset.y, 0 };
              region.imageExtent = { (uint32_t)size.x, (uint32_t)size.y, 1 };
              vkCmdCopyBufferToImage(
                cmd, staging, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

              TransitionTo(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          });

        vkDestroyBuffer(ctx->GetDevice(), staging, nullptr);
        vkFreeMemory(ctx->GetDevice(), stagingMemory, nullptr);
    }

    void VulkanTexture2D::UpdateData(Dimension2D offset, Dimension2D size, const byte* data)
    {
        UploadBaseLevel(offset, size, data);
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
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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
