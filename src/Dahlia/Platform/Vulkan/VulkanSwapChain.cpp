#include <algorithm>
#include <limits>

#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanSwapChain.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    VulkanSwapChain::VulkanSwapChain(VkSurfaceKHR surface, VkExtent2D desiredExtent)
      : mSurface(surface)
    {
        Create(desiredExtent, VK_NULL_HANDLE);
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        vkDeviceWaitIdle(ctx->GetDevice());
        DestroyViews();
        mDepth.reset();
        vkDestroySwapchainKHR(ctx->GetDevice(), mSwapChain, nullptr);
    }

    Format VulkanSwapChain::GetFormat() const
    {
        return mImageFormat == VK_FORMAT_B8G8R8A8_UNORM ? Format::BGRA8 : Format::RGBA8;
    }

    void VulkanSwapChain::Create(VkExtent2D desiredExtent, VkSwapchainKHR oldSwapChain)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkPhysicalDevice physicalDevice = ctx->GetPhysicalDevice();

        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &caps);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &formatCount, formats.data());

        // UNORM (not sRGB) matches OpenGL's default framebuffer
        VkSurfaceFormatKHR chosen = formats.front();
        for (VkFormat preferred : { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM })
        {
            auto it = std::find_if(formats.begin(),
                                   formats.end(),
                                   [preferred](const VkSurfaceFormatKHR& f)
                                   {
                                       return f.format == preferred &&
                                              f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                                   });
            if (it != formats.end())
            {
                chosen = *it;
                break;
            }
        }
        mImageFormat = chosen.format;

        if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
            mExtent = caps.currentExtent;
        else
            mExtent = { std::clamp(desiredExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
                        std::clamp(desiredExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height) };

        mMinImageCount = std::max(caps.minImageCount, 2u);
        uint32_t imageCount = mMinImageCount + 1;
        if (caps.maxImageCount > 0)
            imageCount = std::min(imageCount, caps.maxImageCount);

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = mSurface;
        info.minImageCount = imageCount;
        info.imageFormat = chosen.format;
        info.imageColorSpace = chosen.colorSpace;
        info.imageExtent = mExtent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-sync, always available
        info.clipped = VK_TRUE;
        info.oldSwapchain = oldSwapChain;

        DH_VK_CHECK(vkCreateSwapchainKHR(ctx->GetDevice(), &info, nullptr, &mSwapChain),
                    "vkCreateSwapchainKHR");
        if (oldSwapChain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(ctx->GetDevice(), oldSwapChain, nullptr);

        uint32_t count = 0;
        vkGetSwapchainImagesKHR(ctx->GetDevice(), mSwapChain, &count, nullptr);
        mImages.resize(count);
        vkGetSwapchainImagesKHR(ctx->GetDevice(), mSwapChain, &count, mImages.data());
        mLayouts.assign(count, VK_IMAGE_LAYOUT_UNDEFINED);

        mViews.resize(count);
        for (uint32_t i = 0; i < count; i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = mImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = mImageFormat;
            viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            DH_VK_CHECK(vkCreateImageView(ctx->GetDevice(), &viewInfo, nullptr, &mViews[i]),
                        "vkCreateImageView");
        }

        mDepth = make<VulkanTexture2D>(Dimension2D{ mExtent.width, mExtent.height }, DEPTH_FORMAT, true);
    }

    void VulkanSwapChain::DestroyViews()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        for (VkImageView view : mViews)
            vkDestroyImageView(ctx->GetDevice(), view, nullptr);
        mViews.clear();
        mImages.clear();
        mLayouts.clear();
    }

    void VulkanSwapChain::Recreate(VkExtent2D desiredExtent)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        vkDeviceWaitIdle(ctx->GetDevice());
        DestroyViews();
        Create(desiredExtent, mSwapChain);
    }
}
