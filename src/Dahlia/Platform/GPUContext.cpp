#include <stdexcept>

#include <GLFW/glfw3.h>

#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalContext.h"
#endif

namespace DH
{
    GPUContext* GPUContext::sCurrentContext = nullptr;

    GPUContext::GPUContext()
    {
        if (glfwInit() != GLFW_TRUE)
            throw std::runtime_error("Failed to initialize GLFW");
    }

    // All backends follow OpenGL's conventions: clip space depth in [-1, 1], NDC y pointing up and
    // counter-clockwise front faces. Vulkan and Metal adapt to them internally.
    GPUContext* GPUContext::Create(const GraphicsAPI api, uint32_t major, uint32_t minor)
    {
        if (sCurrentContext != nullptr)
            throw std::runtime_error("Cannot create two instances of a GPU context");

        switch (api)
        {
            case GraphicsAPI::OpenGL:
                sCurrentContext = new OpenGLContext(major, minor);
                return sCurrentContext;
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                VulkanGPUContext::PrepareGLFW(); // Must precede glfwInit in GPUContext()
                sCurrentContext = new VulkanGPUContext(major, minor);
                return sCurrentContext;
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                sCurrentContext = new MetalGPUContext();
                return sCurrentContext;
#endif
            default:
                throw std::runtime_error(
                  "The requested graphics API is not available in this build of Dahlia");
        }
    }
}
