#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Platform/GPUContext.h"

namespace DH
{
    // Requires Vulkan 1.3 (dynamic rendering) and VK_EXT_depth_clip_control, which lets pipelines
    // use OpenGL's [-1, 1] clip space depth. Works with MoltenVK on macOS.
    //
    // The instance is created with the context; the device is created when the first display
    // provides a surface (so a queue with present support can be chosen).
    class DH_API VulkanGPUContext : public GPUContext
    {
      public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        VulkanGPUContext(uint32_t major, uint32_t minor);
        VulkanGPUContext(const VulkanGPUContext&) = delete;

        // Points GLFW at the Vulkan loader Dahlia links against, so it does not have to find one
        // on the library search path (Homebrew's loader is not on it). Call before glfwInit.
        static void PrepareGLFW();

        void CreateDevice(VkSurfaceKHR surface);

        [[nodiscard]] GraphicsAPI GetContextAPI() const override { return GraphicsAPI::Vulkan; }
        [[nodiscard]] void* GetNativeDevice() override { return mDevice; }
        [[nodiscard]] const GPULimits& GetLimits() const override { return mLimits; }

        void WaitIdle() override;

        std::string GetVersionStr() override;
        std::string GetVendorStr() override;
        std::string GetShadingVersionStr() override;

        void DestroyContext() override;

        [[nodiscard]] uint32_t GetApiVersion() const { return mApiVersion; }
        [[nodiscard]] VkInstance GetInstance() const { return mInstance; }
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
        [[nodiscard]] VkDevice GetDevice() const { return mDevice; }
        [[nodiscard]] uint32_t GetGraphicsQueueFamily() const { return mQueueFamily; }
        [[nodiscard]] VkQueue GetGraphicsQueue() const { return mQueue; }
        [[nodiscard]] VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }
        // Linear, clamp to edge. Used to show render target textures in ImGui.
        [[nodiscard]] VkSampler GetDefaultSampler() const { return mDefaultSampler; }
        [[nodiscard]] bool SupportsNonSolidFill() const { return mFillModeNonSolid; }
        // Different blend state per colour attachment
        [[nodiscard]] bool SupportsIndependentBlend() const { return mIndependentBlend; }
        // Rendering into a slice of a 3D image through a 2D view. Core Vulkan; optional on
        // portability subset devices (MoltenVK)
        [[nodiscard]] bool SupportsImageView2DOn3D() const { return mImageView2DOn3D; }

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        // Format a texture of `format` is stored in on this device: 3 component formats become 4
        // component and D24S8 is substituted when unsupported (e.g. on Apple GPUs)
        [[nodiscard]] VkFormat ToTextureFormat(Format format) const;
        [[nodiscard]] bool SupportsLinearBlit(VkFormat format) const;
        [[nodiscard]] bool SupportsLinearFilter(VkFormat format) const;

        // Records commands into a one-off command buffer, submits it and waits for the queue to
        // go idle. A full barrier is recorded first so earlier submissions are complete.
        void SubmitImmediate(const std::function<void(VkCommandBuffer)>& record);

        // Destroys Vulkan objects once no frame in flight can still be using them
        void DeferDestroy(std::function<void()> destroy);
        // Called by the display after waiting for a frame slot's fence
        void CollectGarbage();
        // Called by the display after each frame is submitted
        void OnFrameSubmitted() { mFrameCounter++; }

      private:
        void CreateInstance(uint32_t major, uint32_t minor);
        void SelectPhysicalDevice(VkSurfaceKHR surface);
        void CreateLogicalDevice();
        void CreateDeviceObjects();
        void FlushAllGarbage();

        uint32_t mApiVersion = 0;
        bool mValidationEnabled = false;
        bool mPortabilitySubset = false;
        bool mFillModeNonSolid = false;
        bool mIndependentBlend = false;
        bool mImageView2DOn3D = true;

        VkInstance mInstance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties mDeviceProperties{};
        VkDevice mDevice = VK_NULL_HANDLE;
        uint32_t mQueueFamily = 0;
        VkQueue mQueue = VK_NULL_HANDLE;

        VkCommandPool mImmediatePool = VK_NULL_HANDLE;
        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        VkSampler mDefaultSampler = VK_NULL_HANDLE;
        VkFormat mDepthStencilFormat = VK_FORMAT_UNDEFINED;

        GPULimits mLimits;

        uint64_t mFrameCounter = 0;
        std::vector<std::pair<uint64_t, std::function<void()>>> mGarbage;
    };
}
