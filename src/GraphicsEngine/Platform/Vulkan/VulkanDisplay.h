#pragma once

#include<vulkan/vulkan.h>
#include "GraphicsEngine/Platform/GLFW/GLFWDisplayBase.h"

namespace GEng {
    class GPUContext;
    class VulkanSwapChain;

    class VulkanDisplay : public GLFWDisplayBase {
    public:
        VulkanDisplay(GPUContext *ctx, const DisplayProps &props);
        ~VulkanDisplay() override;
        //
        //
        [[nodiscard]] VkSurfaceKHR GetSurface() const { return mSurface; }
        //
        //
    private:

        VkSurfaceKHR mSurface = nullptr;
        VulkanSwapChain *mSwapChain = nullptr;
    };
} // GEng
