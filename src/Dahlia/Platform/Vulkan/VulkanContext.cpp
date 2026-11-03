#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <string_view>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Dahlia/Graphics/FormatConversion.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    namespace
    {
        constexpr const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";
        constexpr const char* PORTABILITY_SUBSET_EXTENSION = "VK_KHR_portability_subset";

        bool HasExtension(const std::vector<VkExtensionProperties>& available, const char* name)
        {
            return std::any_of(available.begin(),
                               available.end(),
                               [name](const VkExtensionProperties& ext)
                               { return std::strcmp(ext.extensionName, name) == 0; });
        }

        std::vector<VkExtensionProperties> DeviceExtensions(VkPhysicalDevice device)
        {
            uint32_t count = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
            return extensions;
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* data,
                                                     void*)
        {
            const char* id = data->pMessageIdName != nullptr ? data->pMessageIdName : "";
            if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                DH_FATAL(Vulkan Validation, "[error] " << id << ": " << data->pMessage);
            }
            else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                DH_WARN(Vulkan Validation, "[warning] " << id << ": " << data->pMessage);
            }
            return VK_FALSE;
        }
    }

    void VulkanGPUContext::PrepareGLFW()
    {
        glfwInitVulkanLoader(vkGetInstanceProcAddr);
    }

    VulkanGPUContext::VulkanGPUContext(uint32_t major, uint32_t minor)
    {
        CreateInstance(major, minor);
    }

    void VulkanGPUContext::CreateInstance(uint32_t major, uint32_t minor)
    {
        if (major < 1 || (major == 1 && minor < 3))
            throw std::runtime_error("The Vulkan backend requires Vulkan 1.3 or newer");

        uint32_t loaderVersion = VK_API_VERSION_1_0;
        vkEnumerateInstanceVersion(&loaderVersion);
        mApiVersion = VK_MAKE_API_VERSION(0, major, minor, 0);
        if (loaderVersion < mApiVersion)
            throw std::runtime_error("The installed Vulkan loader does not support the requested "
                                     "Vulkan version");

        uint32_t extCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> available(extCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, available.data());

        uint32_t glfwCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwCount);
        if (glfwExtensions == nullptr)
            throw std::runtime_error("GLFW could not find a Vulkan loader with surface support");
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwCount);

        VkInstanceCreateFlags flags = 0;
        // MoltenVK is a portability driver and is only enumerated when asked for
        if (HasExtension(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }

#ifdef CONFIG_DEBUG
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        mValidationEnabled =
          std::any_of(layers.begin(),
                      layers.end(),
                      [](const VkLayerProperties& layer)
                      { return std::strcmp(layer.layerName, VALIDATION_LAYER) == 0; }) &&
          HasExtension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (mValidationEnabled)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        else
        {
            DH_WARN(DH::VulkanGPUContext,
                    "Vulkan validation layers are not installed; running without validation");
        }
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Dahlia Application";
        appInfo.pEngineName = "Dahlia";
        appInfo.apiVersion = mApiVersion;

        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugInfo.pfnUserCallback = DebugCallback;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = flags;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (uint32_t)extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        if (mValidationEnabled)
        {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &VALIDATION_LAYER;
            createInfo.pNext = &debugInfo;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
        if (result == VK_ERROR_LAYER_NOT_PRESENT && mValidationEnabled)
        {
            // The layer's manifest was found but its library could not be loaded. Homebrew's
            // manifest names the dylib without a path, and /opt/homebrew/lib is not searched.
            DH_WARN(DH::VulkanGPUContext,
                    "The Vulkan validation layer is installed but could not be loaded; running "
                    "without validation. On macOS with Homebrew, launch with "
                    "DYLD_FALLBACK_LIBRARY_PATH=/opt/homebrew/lib to enable it.");
            mValidationEnabled = false;
            extensions.erase(std::remove(extensions.begin(),
                                         extensions.end(),
                                         std::string_view(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)),
                             extensions.end());
            createInfo.enabledExtensionCount = (uint32_t)extensions.size();
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pNext = nullptr;
            result = vkCreateInstance(&createInfo, nullptr, &mInstance);
        }
        DH_VK_CHECK(result, "vkCreateInstance");

        if (mValidationEnabled)
        {
            auto createMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
              mInstance, "vkCreateDebugUtilsMessengerEXT");
            if (createMessenger != nullptr)
                createMessenger(mInstance, &debugInfo, nullptr, &mDebugMessenger);
        }
    }

    void VulkanGPUContext::CreateDevice(VkSurfaceKHR surface)
    {
        if (mDevice != VK_NULL_HANDLE)
            return;

        SelectPhysicalDevice(surface);
        CreateLogicalDevice();
        CreateDeviceObjects();
    }

    void VulkanGPUContext::SelectPhysicalDevice(VkSurfaceKHR surface)
    {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
        if (count == 0)
            throw std::runtime_error("No GPUs with Vulkan support are available");
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(mInstance, &count, devices.data());

        int bestScore = -1;
        for (VkPhysicalDevice device : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            if (props.apiVersion < mApiVersion)
                continue;

            const std::vector<VkExtensionProperties> extensions = DeviceExtensions(device);
            if (!HasExtension(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME) ||
                !HasExtension(extensions, VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME))
                continue;

            VkPhysicalDeviceDepthClipControlFeaturesEXT clipControl{};
            clipControl.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
            VkPhysicalDeviceVulkan13Features features13{};
            features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            features13.pNext = &clipControl;
            VkPhysicalDeviceFeatures2 features{};
            features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features.pNext = &features13;
            vkGetPhysicalDeviceFeatures2(device, &features);
            if (!features13.dynamicRendering || !clipControl.depthClipControl)
                continue;

            // One queue family that can both draw and present keeps synchronisation simple
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

            std::optional<uint32_t> family;
            for (uint32_t i = 0; i < familyCount; i++)
            {
                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
                if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present)
                {
                    family = i;
                    break;
                }
            }
            if (!family)
                continue;

            int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 1000
                        : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 100
                                                                                     : 1;
            if (score > bestScore)
            {
                bestScore = score;
                mPhysicalDevice = device;
                mQueueFamily = *family;
                mDeviceProperties = props;
                mFillModeNonSolid = features.features.fillModeNonSolid == VK_TRUE;
                mPortabilitySubset = HasExtension(extensions, PORTABILITY_SUBSET_EXTENSION);
            }
        }

        if (mPhysicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error(
              "No suitable Vulkan GPU found. Dahlia needs Vulkan 1.3 with dynamic rendering, "
              "VK_EXT_depth_clip_control and a queue that supports graphics and present.");

        DH_LOG(Green, DH::VulkanGPUContext, "Selected " << mDeviceProperties.deviceName);
    }

    void VulkanGPUContext::CreateLogicalDevice()
    {
        const float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = mQueueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME };
        if (mPortabilitySubset)
            extensions.push_back(PORTABILITY_SUBSET_EXTENSION);

        VkPhysicalDeviceDepthClipControlFeaturesEXT clipControl{};
        clipControl.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
        clipControl.depthClipControl = VK_TRUE;
        VkPhysicalDeviceVulkan13Features features13{};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.pNext = &clipControl;
        features13.dynamicRendering = VK_TRUE;
        VkPhysicalDeviceFeatures2 features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &features13;
        features.features.fillModeNonSolid = mFillModeNonSolid ? VK_TRUE : VK_FALSE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueInfo;
        createInfo.enabledExtensionCount = (uint32_t)extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        DH_VK_CHECK(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice),
                    "vkCreateDevice");
        vkGetDeviceQueue(mDevice, mQueueFamily, 0, &mQueue);
    }

    void VulkanGPUContext::CreateDeviceObjects()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex = mQueueFamily;
        DH_VK_CHECK(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mImmediatePool),
                    "vkCreateCommandPool");

        const VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
        };
        VkDescriptorPoolCreateInfo descriptorInfo{};
        descriptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptorInfo.maxSets = 1024;
        descriptorInfo.poolSizeCount = 2;
        descriptorInfo.pPoolSizes = poolSizes;
        DH_VK_CHECK(vkCreateDescriptorPool(mDevice, &descriptorInfo, nullptr, &mDescriptorPool),
                    "vkCreateDescriptorPool");

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        DH_VK_CHECK(vkCreateSampler(mDevice, &samplerInfo, nullptr, &mDefaultSampler),
                    "vkCreateSampler");

        // Apple GPUs (MoltenVK) have no D24S8
        for (VkFormat candidate : { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT })
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, candidate, &props);
            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                mDepthStencilFormat = candidate;
                break;
            }
        }

        mLimits.maxTextureSize = mDeviceProperties.limits.maxImageDimension2D;
        mLimits.maxColourAttachments = mDeviceProperties.limits.maxColorAttachments;
        mLimits.maxBindGroups = std::min(MAX_BIND_GROUPS, mDeviceProperties.limits.maxBoundDescriptorSets);
        mLimits.maxPushConstantSize =
          std::min(MAX_PUSH_CONSTANT_SIZE, mDeviceProperties.limits.maxPushConstantsSize);
        // The flipped viewport stores row 0 of a render target at the top
        mLimits.textureOriginBottomLeft = false;
    }

    uint32_t VulkanGPUContext::FindMemoryType(uint32_t typeFilter,
                                              VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1u << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }
        throw std::runtime_error("Failed to find a suitable Vulkan memory type");
    }

    VkFormat VulkanGPUContext::ToTextureFormat(Format format) const
    {
        switch (TextureStorageFormat(format))
        {
            case Format::R8:
                return VK_FORMAT_R8_UNORM;
            case Format::RG8:
                return VK_FORMAT_R8G8_UNORM;
            case Format::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case Format::BGRA8:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case Format::R32F:
                return VK_FORMAT_R32_SFLOAT;
            case Format::RG32F:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case Format::D24S8:
                return mDepthStencilFormat;
            case Format::D32F:
                return VK_FORMAT_D32_SFLOAT;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    bool VulkanGPUContext::SupportsLinearBlit(VkFormat format) const
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);
        const VkFormatFeatureFlags needed = VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                                            VK_FORMAT_FEATURE_BLIT_DST_BIT |
                                            VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        return (props.optimalTilingFeatures & needed) == needed;
    }

    void VulkanGPUContext::SubmitImmediate(const std::function<void(VkCommandBuffer)>& record)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mImmediatePool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        DH_VK_CHECK(vkAllocateCommandBuffers(mDevice, &allocInfo, &cmd), "vkAllocateCommandBuffers");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        // Order after everything already submitted (e.g. frames still reading a resource)
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             0,
                             1,
                             &barrier,
                             0,
                             nullptr,
                             0,
                             nullptr);

        record(cmd);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        DH_VK_CHECK(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE), "vkQueueSubmit");
        vkQueueWaitIdle(mQueue);

        vkFreeCommandBuffers(mDevice, mImmediatePool, 1, &cmd);
    }

    void VulkanGPUContext::DeferDestroy(std::function<void()> destroy)
    {
        mGarbage.emplace_back(mFrameCounter, std::move(destroy));
    }

    void VulkanGPUContext::CollectGarbage()
    {
        // Something released while frame N was being recorded is safe once frame N's fence has
        // been waited on, which is guaranteed MAX_FRAMES_IN_FLIGHT frames later
        auto ready = [this](const auto& item)
        { return item.first + MAX_FRAMES_IN_FLIGHT <= mFrameCounter; };

        std::vector<std::function<void()>> destroyNow;
        for (auto& item : mGarbage)
        {
            if (ready(item))
                destroyNow.push_back(std::move(item.second));
        }
        mGarbage.erase(std::remove_if(mGarbage.begin(), mGarbage.end(), ready), mGarbage.end());

        for (auto& destroy : destroyNow)
            destroy();
    }

    void VulkanGPUContext::FlushAllGarbage()
    {
        // Destroy callbacks may release more resources, so loop until empty
        while (!mGarbage.empty())
        {
            auto garbage = std::move(mGarbage);
            mGarbage.clear();
            for (auto& item : garbage)
                item.second();
        }
    }

    void VulkanGPUContext::WaitIdle()
    {
        if (mDevice == VK_NULL_HANDLE)
            return;
        vkDeviceWaitIdle(mDevice);
        FlushAllGarbage();
    }

    std::string VulkanGPUContext::GetVersionStr()
    {
        const uint32_t v = mDeviceProperties.apiVersion;
        return std::to_string(VK_API_VERSION_MAJOR(v)) + "." + std::to_string(VK_API_VERSION_MINOR(v)) +
               "." + std::to_string(VK_API_VERSION_PATCH(v));
    }

    std::string VulkanGPUContext::GetVendorStr()
    {
        return mDeviceProperties.deviceName;
    }

    std::string VulkanGPUContext::GetShadingVersionStr()
    {
        return "SPIR-V (Vulkan GLSL 450)";
    }

    void VulkanGPUContext::DestroyContext()
    {
        WaitIdle();

        if (mDevice != VK_NULL_HANDLE)
        {
            vkDestroySampler(mDevice, mDefaultSampler, nullptr);
            vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
            vkDestroyCommandPool(mDevice, mImmediatePool, nullptr);
            vkDestroyDevice(mDevice, nullptr);
        }

        if (mDebugMessenger != VK_NULL_HANDLE)
        {
            auto destroyMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
              mInstance, "vkDestroyDebugUtilsMessengerEXT");
            if (destroyMessenger != nullptr)
                destroyMessenger(mInstance, mDebugMessenger, nullptr);
        }
        vkDestroyInstance(mInstance, nullptr);

        glfwTerminate();
        delete this;
    }
}
