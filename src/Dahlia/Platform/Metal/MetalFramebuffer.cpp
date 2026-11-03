#include <stdexcept>

#include "Dahlia/Platform/Metal/MetalFramebuffer.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"

namespace DH
{
    MetalFramebuffer::MetalFramebuffer(const FramebufferSpecification& spec)
      : mSpecification(spec)
    {
        if (spec.samples > 1)
            throw std::invalid_argument(
              "Multisampled framebuffers are not supported by the Metal backend yet");
        CreateAttachments();
    }

    void MetalFramebuffer::CreateAttachments()
    {
        mAttachments.clear();
        mColourAttachments.clear();
        mDepth.reset();

        const Dimension2D size{ mSpecification.width, mSpecification.height };
        for (const FramebufferAttachment& attachment : mSpecification.attachments)
        {
            auto texture = make<MetalTexture2D>(size, attachment.format, true);
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

    void MetalFramebuffer::Resize(size_t newWidth, size_t newHeight)
    {
        mSpecification.width = newWidth;
        mSpecification.height = newHeight;
        CreateAttachments();
    }

    sRef<Texture2D> MetalFramebuffer::GetAttachmentTexture(size_t index) const
    {
        if (index >= mAttachments.size() || mSpecification.attachments[index].renderbuffer)
            return nullptr;
        return mAttachments[index];
    }
}
