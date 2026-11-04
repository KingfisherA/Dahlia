#include <stdexcept>

#include "Dahlia/Platform/Metal/MetalFramebuffer.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"

namespace DH
{
    MetalFramebuffer::MetalFramebuffer(const FramebufferSpecification& spec)
      : mSpecification(Prepare(spec))
    {
        if (mSpecification.samples > 1)
            throw std::invalid_argument(
              "Multisampled framebuffers are not supported by the Metal backend yet");
        CreateAttachments();
    }

    void MetalFramebuffer::CreateAttachments()
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
                attachment.owner = spec.texture;
                attachment.texture = AsMetalTexture(*spec.texture).GetMetalTexture();
                attachment.level = spec.mipLevel;
                // Metal addresses 3D slices as depth planes (array slices would use setSlice)
                attachment.depthPlane = spec.layer;
                mOwned.push_back(nullptr);
            }
            else
            {
                auto texture = make<MetalTexture2D>(size, spec.format, true);
                attachment.owner = texture;
                attachment.texture = texture->GetMetalTexture();
                mOwned.push_back(texture);
            }

            if (IsDepthFormat(spec.format))
                mDepth = attachment;
            else
                mColourAttachments.push_back(attachment);
        }
    }

    void MetalFramebuffer::Resize(size_t newWidth, size_t newHeight)
    {
        if (HasExistingAttachments())
            throw std::logic_error(
              "Framebuffers with existing attachments cannot be resized; create a new one");
        mSpecification.width = newWidth;
        mSpecification.height = newHeight;
        CreateAttachments();
    }

    sRef<Texture2D> MetalFramebuffer::GetAttachmentTexture(size_t index) const
    {
        if (index >= mOwned.size() || mSpecification.attachments[index].renderbuffer)
            return nullptr;
        if (mOwned[index])
            return mOwned[index];
        return GetExistingTexture2D(index);
    }
}
