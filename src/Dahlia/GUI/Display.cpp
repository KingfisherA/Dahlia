#include <stdexcept>

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/OpenGL/OpenGLDisplay.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanDisplay.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalDisplay.h"
#endif

namespace DH
{
    Display* Display::Create(GPUContext* ctx, const DisplayProps& properties)
    {
        switch (ctx->GetContextAPI())
        {
            case GraphicsAPI::OpenGL:
                return new OpenGLDisplay(ctx, properties);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return new VulkanDisplay(ctx, properties);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return new MetalDisplay(ctx, properties);
#endif
            default:
                throw std::runtime_error("No display implementation for this graphics API");
        }
    }
}
