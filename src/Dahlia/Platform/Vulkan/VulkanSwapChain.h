#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class VulkanTexture2D;

    // Swapchain images plus a matching depth buffer for the display render pass
    class VulkanSwapChain
    {
      public:
        static constexpr Format DEPTH_FORMAT = Format::D32F;

        VulkanSwapChain(VkSurfaceKHR surface, VkExtent2D desiredExtent);
        VulkanSwapChain(const VulkanSwapChain&) = delete;
        ~VulkanSwapChain();

        // Waits for the device to go idle first
        void Recreate(VkExtent2D desiredExtent);

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const { return mSwapChain; }
        [[nodiscard]] VkFormat GetImageFormat() const { return mImageFormat; }
        [[nodiscard]] Format GetFormat() const;
        [[nodiscard]] VkExtent2D GetExtent() const { return mExtent; }
        [[nodiscard]] uint32_t GetImageCount() const { return (uint32_t)mImages.size(); }
        [[nodiscard]] uint32_t GetMinImageCount() const { return mMinImageCount; }

        [[nodiscard]] VkImage GetImage(uint32_t index) const { return mImages[index]; }
        [[nodiscard]] VkImageView GetView(uint32_t index) const { return mViews[index]; }
        // Layout of each image, tracked in recording order
        [[nodiscard]] VkImageLayout& ImageLayout(uint32_t index) { return mLayouts[index]; }
        [[nodiscard]] VulkanTexture2D& GetDepth() const { return *mDepth; }

      private:
        void Create(VkExtent2D desiredExtent, VkSwapchainKHR oldSwapChain);
        void DestroyViews();

        VkSurfaceKHR mSurface;
        VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
        VkFormat mImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D mExtent{};
        uint32_t mMinImageCount = 2;

        std::vector<VkImage> mImages;
        std::vector<VkImageView> mViews;
        std::vector<VkImageLayout> mLayouts;
        sRef<VulkanTexture2D> mDepth;
    };
}
