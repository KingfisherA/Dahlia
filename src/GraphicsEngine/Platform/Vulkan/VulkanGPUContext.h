#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include<unordered_set>
#include "GraphicsEngine/Platform/GPUContext.h"
#include<optional>


namespace GEng {
    class VulkanCommandPool;

    struct QueueFamilyIndices {
        ///Index of queue which corresponds to the graphics family on the current physical device
        std::optional<uint32_t> graphicsFamily;
        ///Index of the queue corresponding to the presentation (surface drawing) family of the physical device
        std::optional<uint32_t> presentFamily;

        ///Returns whether the logical device possesses a valid graphics and presentation queue
        [[nodiscard]] bool IsComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class VulkanGPUContext : public GPUContext {
    public:
        VulkanGPUContext();

        constexpr static std::array<const char *, 1> VALIDATION_LAYERS = {
            "VK_LAYER_KHRONOS_validation"
        };
        const static std::vector<const char *> DEFAULT_REQ_EXTENSIONS;
        //
        std::vector<const char *> GetRequiredExtensions() const;
        bool CheckValidationLayerSupport();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void BindDebugCallback();
        //
        //
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<std::string> &extensions);
        static QueueFamilyIndices QueryQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
        void CreateDevice(VkSurfaceKHR surface,
                          const std::vector<const char *> &requiredExtensions = DEFAULT_REQ_EXTENSIONS);

    private:
        void SelectPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char *> &requiredExtensions);
        void CreateLogicalDevice(VkSurfaceKHR surface, const std::vector<const char *> &requiredExtensions);

    public:
        //
        //
        [[nodiscard]] VkInstance GetInstance() const { return mInstance; }
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
        [[nodiscard]] VkDevice GetDevice() const { return mDevice; }
        [[nodiscard]] const QueueFamilyIndices &GetQueueFamilyIndices() const { return mQueueFamilyIndices; }
        [[nodiscard]] VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
        [[nodiscard]] VkQueue GetPresentQueue() const { return mPresentQueue; }
        [[nodiscard]] VulkanCommandPool* GetCommandPool() const { return mCommandPool; }
        //
        //
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        //
        //
        VkSampleCountFlagBits GetMaxMSAASamples();
        //
        //
        void AllocateImageMemory(VkImage image, VkDeviceMemory *imageMemory, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageAspectFlags aspectMask,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout, uint32_t mipLevels);
        //
        //
        bool IsDepthEnabled() const { return false; }
        //
        //
        [[nodiscard]] API GetContextAPI() const override { return API::Vulkan; }
        void Reset() override;
        void Clear(const glm::vec4 &clearColour) override;
        std::string GetVersionStr() override;
        std::string GetVendorStr() override;
        std::string GetShadingVersionStr() override;
        void SetFaceCulling(bool activate) override;
        void SetDepthTest(bool activate) override;
        void SetPolygonMode(const PolygonMode mode) override;
        void SetBlending(bool activate, const Blending mode) override;
        void SetViewport(const glm::ivec4 &bounds) override;
        void ActivateDefaultFramebuffer() override;
        //

        void DestroyContext() override;

    private:
        bool mValidationLayersEnabled = true;
        VkDebugUtilsMessengerEXT mDebugMessenger = nullptr;

        VkInstance mInstance = nullptr;

        VkPhysicalDevice mPhysicalDevice = nullptr;
        VkDevice mDevice = nullptr;
        VkQueue mGraphicsQueue = nullptr, mPresentQueue = nullptr;
        QueueFamilyIndices mQueueFamilyIndices;

        VulkanCommandPool *mCommandPool = nullptr;
    };
} // GEng
