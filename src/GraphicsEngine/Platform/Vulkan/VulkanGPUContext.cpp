#include<vulkan/vulkan.h>
#include <cstring>
#include <GLFW/glfw3.h>
#include <set>

#include "GraphicsEngine/Platform/Vulkan/VulkanGPUContext.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanSwapChain.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanCommandBuffers.h"


namespace GEng {
    const std::vector<const char *> VulkanGPUContext::DEFAULT_REQ_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    //
    //
    VulkanGPUContext::VulkanGPUContext() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan GPU Context";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (glfwInit() != GLFW_TRUE)
            throw std::runtime_error("Failed to initialize GLFW");
        std::vector<const char *> extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (mValidationLayersEnabled && !CheckValidationLayerSupport()) {
            throw std::runtime_error("Validation layer activation requested but not available in this context");
        }

        createInfo.enabledLayerCount = 0;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (mValidationLayersEnabled) {
            createInfo.enabledLayerCount = (uint32_t) VALIDATION_LAYERS.size();
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        }

        if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create a Vulkan instance");

        BindDebugCallback();
    }
    std::vector<const char *> VulkanGPUContext::GetRequiredExtensions() const {
        unsigned int glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (mValidationLayersEnabled)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }
    bool VulkanGPUContext::CheckValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: VALIDATION_LAYERS) {
            bool layerFound = false;

            for (const VkLayerProperties &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }
    void VulkanGPUContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = [](
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData)-> VkBool32 {
                    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                        GENG_FATAL(GEng::VulkanGPUContext,
                               std::string("Vulkan validation layer has raised an exception:\n       ") + pCallbackData
                               ->pMessage);
                    return VK_FALSE;
                };
        createInfo.pUserData = (void *) this;
    }
    void VulkanGPUContext::BindDebugCallback() {
        if (!mValidationLayersEnabled) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            mInstance, "vkCreateDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            GENG_FATAL(GEng::VulkanGPUContext,
                       "The current Vulkan implementation does not implement vkCreateDebugUtilsMessengerEXT. Debug message callback could thus not be setup.\n");
            return;
        }

        if (vkCreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
            GENG_FATAL(GEng::VulkanGPUContext, "Failed to bind debug messenger\n");
    }
    //
    //
    bool VulkanGPUContext::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                                       const std::vector<std::string> &extensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    QueueFamilyIndices VulkanGPUContext::QueryQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const VkQueueFamilyProperties &queueFamily: queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
                indices.presentFamily = i;

            i++;
        }

        return indices;
    }
    void VulkanGPUContext::CreateDevice(VkSurfaceKHR surface, const std::vector<const char *> &requiredExtensions) {
        if (mPhysicalDevice != nullptr)
            return;
        SelectPhysicalDevice(surface, requiredExtensions);
        CreateLogicalDevice(surface, requiredExtensions);

        mCommandPool = new VulkanCommandPool(mQueueFamilyIndices.graphicsFamily.value());
    }
    void VulkanGPUContext::SelectPhysicalDevice(VkSurfaceKHR surface,
                                                const std::vector<const char *> &requiredExtensions) {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("No GPUs with Vulkan support available.\n");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

        for (const VkPhysicalDevice &device: devices) {
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            QueueFamilyIndices indices = QueryQueueIndices(device, surface);
            bool requiredExtensionsSupported = CheckDeviceExtensionSupport(device, {});

            bool swapChainAdequate = false;
            if (requiredExtensionsSupported) {
                SwapChainDetails swapChainSupport = VulkanSwapChain::QuerySwapChainSupport(device, surface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                && indices.IsComplete() && requiredExtensionsSupported && swapChainAdequate
                && deviceFeatures.samplerAnisotropy) {
                mPhysicalDevice = device;
                mQueueFamilyIndices = indices;
                std::cout << "Selected " << deviceProperties.deviceName << " as device for Vulkan." << "\n";
                break;
            }
        }

        if (mPhysicalDevice == nullptr)
            throw std::runtime_error(
                "No suitable discrete GPUs available. Need to have graphics and present queues available\n");

        //mMSAASamples = GetMaxUsableMSAACount();
    }
    void VulkanGPUContext::CreateLogicalDevice(VkSurfaceKHR surface,
                                               const std::vector<const char *> &requiredExtensions) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            mQueueFamilyIndices.graphicsFamily.value(), mQueueFamilyIndices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if (mValidationLayersEnabled) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
            throw std::runtime_error("Unable to create logical device.\n");

        vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, mQueueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
    }
    //
    //
    uint32_t VulkanGPUContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }
    //
    //
    VkSampleCountFlagBits VulkanGPUContext::GetMaxMSAASamples() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.
                                    limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }
    //
    //
    void VulkanGPUContext::AllocateImageMemory(VkImage image, VkDeviceMemory *imageMemory,
                                               VkMemoryPropertyFlags properties) {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, imageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate image memory \n");

        vkBindImageMemory(mDevice, image, *imageMemory, 0);
    }
    void VulkanGPUContext::TransitionImageLayout(VkImage image, VkFormat format, VkImageAspectFlags aspectMask,
                                                 VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
        VulkanOneTimeCommandBuffer *cBuffer = new VulkanOneTimeCommandBuffer(mCommandPool);
        cBuffer->Begin();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout ==
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("Unsupported texture layout transition");
        }


        vkCmdPipelineBarrier(
            cBuffer->GetBuffer(),
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );


        cBuffer->EndAndSubmit(mGraphicsQueue);
    }
    //
    //
    void VulkanGPUContext::Reset() {
    }
    void VulkanGPUContext::Clear(const glm::vec4 &clearColour) {
    }
    std::string VulkanGPUContext::GetVersionStr() {
        return {""};
    }
    std::string VulkanGPUContext::GetVendorStr() {
        return {""};
    }
    std::string VulkanGPUContext::GetShadingVersionStr() {
        return {""};
    }
    void VulkanGPUContext::SetFaceCulling(bool activate) {
    }
    void VulkanGPUContext::SetDepthTest(bool activate) {
    }
    void VulkanGPUContext::SetPolygonMode(const PolygonMode mode) {
    }
    void VulkanGPUContext::SetBlending(bool activate, const Blending mode) {
    }
    void VulkanGPUContext::SetViewport(const glm::ivec4 &bounds) {
    }
    void VulkanGPUContext::ActivateDefaultFramebuffer() {
    }
    //
    //
    void VulkanGPUContext::DestroyContext() {
        if (mDevice != nullptr) vkDeviceWaitIdle(mDevice);

        delete mCommandPool;

        if (mDevice != nullptr) vkDestroyDevice(mDevice, nullptr);

        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            mInstance, "vkDestroyDebugUtilsMessengerEXT");
        vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        vkDestroyInstance(mInstance, nullptr);

        glfwTerminate();
        delete this;
    }
    //
    //
} // GEng
