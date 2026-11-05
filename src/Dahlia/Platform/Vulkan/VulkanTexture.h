#pragma once

#include <cstdint>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Texture.h"

namespace DH
{
    // VkImage, its memory, the sampling view and the CPU tracked layout. Shared by every Vulkan
    // texture type so framebuffers and bind groups can treat them alike.
    class DH_API VulkanImage
    {
      public:
        VulkanImage(const VulkanImage&) = delete;

        [[nodiscard]] VkImage GetImage() const { return mImage; }
        // View used for sampling: every mip level (and every slice of a 3D image)
        [[nodiscard]] VkImageView GetView() const { return mView; }
        [[nodiscard]] VkFormat GetVkFormat() const { return mVkFormat; }
        [[nodiscard]] VkImageAspectFlags GetAspect() const { return mAspect; }
        [[nodiscard]] uint32_t GetVkMipLevels() const { return mMipLevels; }

        // 2D view of one mip level and layer (depth slice for 3D images) for use as a render pass
        // attachment. Created on first use and destroyed with the image.
        [[nodiscard]] VkImageView GetAttachmentView(uint32_t mipLevel, uint32_t layer);

        // Layout tracked on the CPU in recording order. Every subresource shares one layout.
        [[nodiscard]] VkImageLayout GetLayout() const { return mLayout; }
        void TransitionTo(VkCommandBuffer cmd, VkImageLayout layout);

      protected:
        struct ImageInfo
        {
            VkImageType type = VK_IMAGE_TYPE_2D;
            VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
            VkExtent3D extent{ 1, 1, 1 };
            uint32_t mipLevels = 1;
            VkImageUsageFlags usage = 0;
            VkImageCreateFlags flags = 0;
        };

        VulkanImage() = default;
        ~VulkanImage();

        void CreateImage(Format format, const ImageInfo& info);
        // Copies tightly packed `data` (in `format`) into mip 0. Leaves the image in
        // SHADER_READ_ONLY_OPTIMAL.
        void UploadRegion(Format format, VkOffset3D offset, VkExtent3D extent, const void* data);

        VkImage mImage = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
        VkImageView mView = VK_NULL_HANDLE;
        VkFormat mVkFormat = VK_FORMAT_UNDEFINED;
        VkImageAspectFlags mAspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageType mImageType = VK_IMAGE_TYPE_2D;
        uint32_t mMipLevels = 1;
        VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        std::unordered_map<uint64_t, VkImageView> mAttachmentViews; // Key: mip << 32 | layer
    };

    // The Vulkan part of any texture created by the Vulkan backend
    [[nodiscard]] DH_API VulkanImage& AsVulkanImage(Texture& texture);
    [[nodiscard]] DH_API const VulkanImage& AsVulkanImage(const Texture& texture);

    class DH_API VulkanTexture2D : public Texture2D, public VulkanImage
    {
      public:
        // Allocated with a full mip chain when the format supports linear blits; the chain is
        // generated on creation (when data is given) and by GenerateMipMaps.
        VulkanTexture2D(const Dimension2D& dimensions,
                        const byte* data,
                        Format format,
                        TextureUsage usage = TextureUsage::Sampled);
        // Framebuffer-owned attachment (sampled too unless it is a depth format)
        VulkanTexture2D(const Dimension2D& dimensions, Format format, bool renderTarget);
        VulkanTexture2D(const VulkanTexture2D&) = delete;
        ~VulkanTexture2D() override;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] uint32_t GetMipLevelCount() const override { return mMipLevels; }
        [[nodiscard]] void* GetNativeHandle() const override { return mView; }
        [[nodiscard]] uint64_t GetImGuiID() const override;

      private:
        void RecordMipGeneration(VkCommandBuffer cmd);

        Dimension2D mDimensions;
        Format mFormat;
        TextureUsage mUsage;

        mutable VkDescriptorSet mImGuiSet = VK_NULL_HANDLE;
    };

    // VK_IMAGE_TYPE_3D. Render target 3D images are created 2D-array compatible so each depth
    // slice can be rendered through a 2D view (needs imageView2DOn3DImage on portability devices
    // such as MoltenVK).
    class DH_API VulkanTexture3D : public Texture3D, public VulkanImage
    {
      public:
        VulkanTexture3D(const Dimension3D& dimensions,
                        const byte* data,
                        Format format,
                        TextureUsage usage = TextureUsage::Sampled);
        VulkanTexture3D(const VulkanTexture3D&) = delete;
        ~VulkanTexture3D() override = default;

        void UpdateData(Dimension3D offset, Dimension3D size, const byte* data) override;

        [[nodiscard]] const Dimension3D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] void* GetNativeHandle() const override { return mView; }

      private:
        Dimension3D mDimensions;
        Format mFormat;
        TextureUsage mUsage;
    };

    class DH_API VulkanSampler : public Sampler
    {
      public:
        explicit VulkanSampler(const SamplerDesc& desc);
        VulkanSampler(const VulkanSampler&) = delete;
        ~VulkanSampler() override;

        [[nodiscard]] const SamplerDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] VkSampler GetSampler() const { return mSampler; }

      private:
        SamplerDesc mDesc;
        VkSampler mSampler = VK_NULL_HANDLE;
    };
}
