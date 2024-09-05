#pragma once
#include <vulkan/vulkan.h>
#include<vector>

namespace GEng {
    class VulkanGPUContext;
    class VulkanDisplay;

    struct SwapChainDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanSwapChain {
    public:
        VulkanSwapChain(VulkanGPUContext *ctx, VulkanDisplay *display);
        ~VulkanSwapChain();

        const SwapChainDetails &GetDetails() const { return mDetails; }

        static SwapChainDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        void RecreateSwapChain();

        static VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                            VkFormatFeatureFlags features);
        static VkFormat FindDepthFormat();
        static bool HasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const {
            return mSwapChain;
        }
        [[nodiscard]] VkFormat GetImageFormat() const {
            return mSwapChainImageFormat;
        }
        [[nodiscard]] VkExtent2D GetSwapChainExtent() const {
            return mSwapChainExtent;
        }
        [[nodiscard]] VkImage GetSwapChainImage(size_t idx) const {
            return mSwapChainImages[idx];
        }
        [[nodiscard]] VkImageView GetSwapChainImageView(size_t idx) const {
            return mSwapChainImageViews[idx];
        }
        [[nodiscard]] VkFramebuffer GetSwapChainFramebuffer(size_t idx) const {
            return mSwapChainFramebuffers[idx];
        }
        [[nodiscard]] VkImage GetDepthImage() const {
            return mDepthImage;
        }
        [[nodiscard]] VkImageView GetDepthImageView() const {
            return mDepthImageView;
        }
        [[nodiscard]] VkRenderPass GetRenderPass() const {
            return mRenderPass;
        }

    private:
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
        void CreateRenderPass();
        void CreateSwapChain();
        void CreateFramebuffers();
        void CreateDepthTexture();
        void DestroySwapChain();

        SwapChainDetails mDetails;

        VulkanDisplay *mDisplay = nullptr;

        VkSwapchainKHR mSwapChain = nullptr;
        VkFormat mSwapChainImageFormat;
        VkExtent2D mSwapChainExtent;
        std::vector<VkImage> mSwapChainImages;
        std::vector<VkImageView> mSwapChainImageViews;
        std::vector<VkFramebuffer> mSwapChainFramebuffers;

        VkImage mDepthImage = nullptr;
        VkDeviceMemory mDepthImageMem = nullptr;
        VkImageView mDepthImageView = nullptr;

        VkRenderPass mRenderPass = nullptr;
    };
}
