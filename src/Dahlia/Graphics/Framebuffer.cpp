#include <stdexcept>

#include "Dahlia/Graphics/Framebuffer.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLFramebuffer.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanFramebuffer.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalFramebuffer.h"
#endif

namespace DH
{
    RenderTargetLayout Framebuffer::GetLayout() const
    {
        const FramebufferSpecification& spec = GetSpecification();

        RenderTargetLayout layout;
        layout.samples = (uint32_t)spec.samples;
        for (const FramebufferAttachment& attachment : spec.attachments)
        {
            if (IsDepthFormat(attachment.format))
                layout.depthFormat = attachment.format;
            else
                layout.colourFormats.push_back(attachment.format);
        }
        return layout;
    }

    sRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLFramebuffer>(spec);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanFramebuffer>(spec);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalFramebuffer>(spec);
#endif
            default:
                throw std::runtime_error("Framebuffer is not implemented for this graphics API yet");
        }
    }
}
