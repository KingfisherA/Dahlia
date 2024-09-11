#include "VulkanSwapChain.h"

#include <algorithm>

#include "VulkanDisplay.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanGPUContext.h"

namespace GEng {
    //
    //
    VulkanSwapChain::VulkanSwapChain(VulkanGPUContext *ctx, VulkanDisplay *display): mDisplay(display) {
        CreateSwapChain();
        CreateRenderPass();
        CreateFramebuffers();
    }

    //
    VulkanSwapChain::~VulkanSwapChain() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        vkDestroyRenderPass(vkCtx->GetDevice(), mRenderPass, nullptr);
        DestroySwapChain();
    }

    //
    SwapChainDetails VulkanSwapChain::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    //
    void VulkanSwapChain::RecreateSwapChain() {
        DestroySwapChain();
        CreateSwapChain();
        CreateFramebuffers();
    }

    //
    VkFormat VulkanSwapChain::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                                  VkFormatFeatureFlags features) {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(vkCtx->GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find any supported formats");
    }

    //
    VkFormat VulkanSwapChain::FindDepthFormat() {
        return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    //
    VkSurfaceFormatKHR
    VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    //
    VkPresentModeKHR
    VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const VkPresentModeKHR &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    //
    VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(mDisplay->GetWidth()),
                static_cast<uint32_t>(mDisplay->GetHeight())
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    //
    void VulkanSwapChain::CreateRenderPass() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = mSwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //Needs to be changed for MSAA
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Needs to be changed for MSAA
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // VkAttachmentDescription colorAttachmentResolve{};
        // colorAttachmentResolve.format = mSwapChainImageFormat;
        // colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        // colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        // colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // VkAttachmentReference colourAttachmentResolveRef{};
        // colourAttachmentResolveRef.attachment = 2;
        // colourAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = nullptr; // &colourAttachmentResolveRef;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        //, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(vkCtx->GetDevice(), &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass \n");
    }

    //
    void VulkanSwapChain::CreateSwapChain() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        mDetails = QuerySwapChainSupport(vkCtx->GetPhysicalDevice(), mDisplay->GetSurface());

#pragma region Swap Chain Creation

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(mDetails.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(mDetails.presentModes);
        VkExtent2D extent = ChooseSwapExtent(mDetails.capabilities);

        uint32_t imageCount = mDetails.capabilities.minImageCount + 1;

        if (mDetails.capabilities.maxImageCount > 0 && imageCount > mDetails.capabilities.maxImageCount)
            imageCount = mDetails.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mDisplay->GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = vkCtx->GetQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = mDetails.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(vkCtx->GetDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain. \n");


        vkGetSwapchainImagesKHR(vkCtx->GetDevice(), mSwapChain, &imageCount, nullptr);
        mSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vkCtx->GetDevice(), mSwapChain, &imageCount, mSwapChainImages.data());

        mSwapChainImageFormat = surfaceFormat.format;
        mSwapChainExtent = extent;

#pragma endregion
#pragma region Swap Chain Image Views

        auto createSwapChainView = [this, vkCtx](VkImage image, VkImageView *imageView) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = image;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = mSwapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.layerCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;

            if (vkCreateImageView(vkCtx->GetDevice(), &createInfo, nullptr, imageView) != VK_SUCCESS)
                throw std::runtime_error("Failed to create image view \n");
        };

        mSwapChainImageViews.resize(mSwapChainImages.size());

        for (size_t i = 0; i < mSwapChainImages.size(); i++)
            createSwapChainView(mSwapChainImages[i], &mSwapChainImageViews[i]);

#pragma endregion

        CreateDepthTexture();
    }

    //
    void VulkanSwapChain::CreateFramebuffers() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        mSwapChainFramebuffers.resize(mSwapChainImageViews.size());
        for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                mSwapChainImageViews[i],
                mDepthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = mSwapChainExtent.width;
            framebufferInfo.height = mSwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vkCtx->GetDevice(), &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]))
                throw std::runtime_error("Failed to create framebuffers \n");
        }
    }

    //
    void VulkanSwapChain::CreateDepthTexture() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        VkFormat depthFormat = FindDepthFormat();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = mSwapChainExtent.width;
        imageInfo.extent.height = mSwapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(vkCtx->GetDevice(), &imageInfo, nullptr, &mDepthImage) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image \n");

        vkCtx->AllocateImageMemory(mDepthImage, &mDepthImageMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = mDepthImage;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = depthFormat;

        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;

        if (vkCreateImageView(vkCtx->GetDevice(), &imageViewInfo, nullptr, &mDepthImageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create depth image view \n");

        vkCtx->TransitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }

    //
    void VulkanSwapChain::DestroySwapChain() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = vkCtx->GetDevice();

        vkDeviceWaitIdle(device);

        // vkDestroyImageView(mDevice, mMultisampledImageView, nullptr);
        // vkDestroyImage(mDevice, mMultisampledImage, nullptr);
        // vkFreeMemory(mDevice, mMultisampledImageMem, nullptr);

        vkDestroyImageView(device, mDepthImageView, nullptr);
        vkDestroyImage(device, mDepthImage, nullptr);
        vkFreeMemory(device, mDepthImageMem, nullptr);

        for (VkFramebuffer framebuffer: mSwapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (VkImageView imageView: mSwapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, mSwapChain, nullptr);
    }

    //
    //
} // GEng
