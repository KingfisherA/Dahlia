#include <stdexcept>

#include "Dahlia/Platform/Vulkan/VulkanFramebuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
      : mSpecification(spec)
    {
        if (spec.samples > 1)
            throw std::invalid_argument(
              "Multisampled framebuffers are not supported by the Vulkan backend yet");
        CreateAttachments();
    }

    void VulkanFramebuffer::CreateAttachments()
    {
        mAttachments.clear();
        mColourAttachments.clear();
        mDepth.reset();

        const Dimension2D size{ mSpecification.width, mSpecification.height };
        for (const FramebufferAttachment& attachment : mSpecification.attachments)
        {
            auto texture = make<VulkanTexture2D>(size, attachment.format, true);
            mAttachments.push_back(texture);

            if (IsDepthFormat(attachment.format))
            {
                if (mDepth)
                    throw std::invalid_argument("A framebuffer can only have one depth attachment");
                mDepth = texture;
            }
            else
            {
                mColourAttachments.push_back(texture);
            }
        }
    }

    void VulkanFramebuffer::Resize(size_t newWidth, size_t newHeight)
    {
        mSpecification.width = newWidth;
        mSpecification.height = newHeight;
        CreateAttachments(); // Old textures are destroyed once no frame in flight uses them
    }

    sRef<Texture2D> VulkanFramebuffer::GetAttachmentTexture(size_t index) const
    {
        if (index >= mAttachments.size() || mSpecification.attachments[index].renderbuffer)
            return nullptr;
        return mAttachments[index];
    }
}
