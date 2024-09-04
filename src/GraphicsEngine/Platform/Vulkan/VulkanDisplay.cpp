#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "GraphicsEngine/Platform/Vulkan/VulkanGPUContext.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanDisplay.h"

#include "GraphicsEngine/Events/WindowEvents.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanSwapChain.h"


namespace GEng {
    //
    VulkanDisplay::VulkanDisplay(GPUContext *ctx, const DisplayProps &props) {
        if (ctx->GetContextAPI() != API::Vulkan)
            throw std::runtime_error("Cannot create a Vulkan display in a non-Vulkan GPU Context");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWDisplayBase::CreateDisplay(props);

        VulkanGPUContext *vkCtx = reinterpret_cast<VulkanGPUContext *>(ctx);
        if (glfwCreateWindowSurface(vkCtx->GetInstance(), mWindow, nullptr, &mSurface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan window surface");

        vkCtx->CreateDevice(mSurface);
        mSwapChain = new VulkanSwapChain(vkCtx, this);

        //To-Do
       //  glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *window, int width, int height) {
       //     GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);
       //     data.mWidth = width;
       //     data.mHeight = height;
       //
       //     WindowResizeEvent resizeEvt(width, height);
       //     data.mEventCallback(resizeEvt);
       // });
    }
    //
    VulkanDisplay::~VulkanDisplay() {
        if (GPUContext::Get()->GetContextAPI() != API::Vulkan)
            return;

        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        delete mSwapChain;
        vkDestroySurfaceKHR(vkCtx->GetInstance(), mSurface, nullptr);
    }
    //
    //
} // GEng
