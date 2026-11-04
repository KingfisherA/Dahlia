#include <stdexcept>

#include "Dahlia/Platform/Vulkan/VulkanFramebuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
      : mSpecification(Prepare(spec))
    {
        if (mSpecification.samples > 1)
            throw std::invalid_argument(
              "Multisampled framebuffers are not supported by the Vulkan backend yet");
        CreateAttachments();
    }

    void VulkanFramebuffer::CreateAttachments()
    {
        mOwned.clear();
        mColourAttachments.clear();
        mDepth.reset();

        const Dimension2D size{ mSpecification.width, mSpecification.height };
        for (const FramebufferAttachment& spec : mSpecification.attachments)
        {
            Attachment attachment;
            attachment.format = spec.format;

            if (spec.IsExisting())
            {
                attachment.texture = spec.texture;
                attachment.image = &AsVulkanImage(*spec.texture);
                attachment.view = attachment.image->GetAttachmentView(spec.mipLevel, spec.layer);
                mOwned.push_back(nullptr);
            }
            else
            {
                auto texture = make<VulkanTexture2D>(size, spec.format, true);
                attachment.texture = texture;
                attachment.image = texture.get();
                attachment.view = texture->GetAttachmentView(0, 0);
                mOwned.push_back(texture);
            }

            if (IsDepthFormat(spec.format))
                mDepth = attachment;
            else
                mColourAttachments.push_back(attachment);
        }
    }

    void VulkanFramebuffer::Resize(size_t newWidth, size_t newHeight)
    {
        if (HasExistingAttachments())
            throw std::logic_error(
              "Framebuffers with existing attachments cannot be resized; create a new one");
        mSpecification.width = newWidth;
        mSpecification.height = newHeight;
        CreateAttachments(); // Old textures are destroyed once no frame in flight uses them
    }

    sRef<Texture2D> VulkanFramebuffer::GetAttachmentTexture(size_t index) const
    {
        if (index >= mOwned.size() || mSpecification.attachments[index].renderbuffer)
            return nullptr;
        if (mOwned[index])
            return mOwned[index];
        return GetExistingTexture2D(index);
    }
}
