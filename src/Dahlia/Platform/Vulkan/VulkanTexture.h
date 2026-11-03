#pragma once

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Texture.h"

namespace DH
{
    class DH_API VulkanTexture2D : public Texture2D
    {
      public:
        // Sampled texture. Allocated with a full mip chain when the format supports linear
        // blits; the chain is generated on creation and by GenerateMipMaps.
        VulkanTexture2D(const Dimension2D& dimensions, const byte* data, Format format);
        // Render target attachment (sampled too unless it is a depth format)
        VulkanTexture2D(const Dimension2D& dimensions, Format format, bool renderTarget);
        VulkanTexture2D(const VulkanTexture2D&) = delete;
        ~VulkanTexture2D() override;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] void* GetNativeHandle() const override { return mView; }
        [[nodiscard]] uint64_t GetImGuiID() const override;

        [[nodiscard]] VkImage GetImage() const { return mImage; }
        [[nodiscard]] VkImageView GetView() const { return mView; }
        [[nodiscard]] VkFormat GetVkFormat() const { return mVkFormat; }
        [[nodiscard]] VkImageAspectFlags GetAspect() const { return mAspect; }

        // Layout tracked on the CPU in recording order
        [[nodiscard]] VkImageLayout GetLayout() const { return mLayout; }
        void TransitionTo(VkCommandBuffer cmd, VkImageLayout layout);

      private:
        void CreateImage(VkImageUsageFlags usage);
        void UploadBaseLevel(Dimension2D offset, Dimension2D size, const byte* data);
        void RecordMipGeneration(VkCommandBuffer cmd);

        Dimension2D mDimensions;
        Format mFormat;
        VkFormat mVkFormat = VK_FORMAT_UNDEFINED;
        VkImageAspectFlags mAspect = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t mMipLevels = 1;

        VkImage mImage = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;
        VkImageView mView = VK_NULL_HANDLE;
        VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        mutable VkDescriptorSet mImGuiSet = VK_NULL_HANDLE;
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
