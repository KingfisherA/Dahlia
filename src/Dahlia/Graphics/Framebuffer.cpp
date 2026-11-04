#include <stdexcept>
#include <string>

#include "Dahlia/Graphics/Texture.h"

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

    FramebufferSpecification Framebuffer::Prepare(const FramebufferSpecification& spec)
    {
        FramebufferSpecification out = spec;
        bool sizeFromTexture = out.width == 0 && out.height == 0;
        bool hasDepth = false;

        for (size_t i = 0; i < out.attachments.size(); i++)
        {
            FramebufferAttachment& attachment = out.attachments[i];
            const std::string where = "Framebuffer attachment " + std::to_string(i);

            if (attachment.IsExisting())
            {
                const Texture& texture = *attachment.texture;
                if (!(texture.GetUsage() & TextureUsage::RenderTarget))
                    throw std::invalid_argument(where +
                                                ": the texture was not created with "
                                                "TextureUsage::RenderTarget");
                if (out.samples > 1)
                    throw std::invalid_argument(where +
                                                ": existing textures cannot be attached to a "
                                                "multisampled framebuffer");
                if (attachment.mipLevel >= texture.GetMipLevelCount())
                    throw std::invalid_argument(where + ": mip level out of range");

                const Dimension3D extent = texture.GetMipExtent(attachment.mipLevel);
                if (texture.GetType() == TextureType::Texture3D ? attachment.layer >= extent.z
                                                                : attachment.layer != 0)
                    throw std::invalid_argument(where + ": layer out of range");

                attachment.format = texture.GetFormat();
                attachment.renderbuffer = false;

                if (sizeFromTexture)
                {
                    out.width = extent.x;
                    out.height = extent.y;
                    sizeFromTexture = false;
                }
                else if (extent.x != out.width || extent.y != out.height)
                {
                    throw std::invalid_argument(where + " is " + std::to_string(extent.x) + "x" +
                                                std::to_string(extent.y) +
                                                " but the framebuffer is " +
                                                std::to_string(out.width) + "x" +
                                                std::to_string(out.height));
                }
            }

            if (IsDepthFormat(attachment.format))
            {
                if (hasDepth)
                    throw std::invalid_argument("A framebuffer can only have one depth attachment");
                hasDepth = true;
            }
        }

        if (out.width == 0 || out.height == 0)
            throw std::invalid_argument("Framebuffer size must be non-zero");
        return out;
    }

    bool Framebuffer::HasExistingAttachments() const
    {
        for (const FramebufferAttachment& attachment : GetSpecification().attachments)
        {
            if (attachment.IsExisting())
                return true;
        }
        return false;
    }

    sRef<Texture2D> Framebuffer::GetExistingTexture2D(size_t index) const
    {
        const FramebufferSpecification& spec = GetSpecification();
        if (index >= spec.attachments.size())
            return nullptr;
        const sRef<Texture>& texture = spec.attachments[index].texture;
        if (texture == nullptr || texture->GetType() != TextureType::Texture2D)
            return nullptr;
        return std::static_pointer_cast<Texture2D>(texture);
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
