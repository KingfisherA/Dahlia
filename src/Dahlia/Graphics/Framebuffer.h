#pragma once

#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class Texture2D;

    struct DH_API FramebufferAttachment
    {
      public:
        FramebufferAttachment() = default;
        explicit FramebufferAttachment(Format format)
          : format(format)
        {
        }
        // A renderbuffer attachment cannot be sampled afterwards (OpenGL only; other backends
        // always create textures)
        FramebufferAttachment(Format format, bool isRenderbuffer)
          : format(format)
          , renderbuffer(isRenderbuffer)
        {
        }

        Format format = Format::None;
        bool renderbuffer = false;
    };

    struct DH_API FramebufferSpecification
    {
      public:
        size_t width = 0, height = 0, samples = 1;
        std::vector<FramebufferAttachment> attachments;
    };

    // An off-screen render target. A framebuffer is rendered into by passing it to
    // CommandList::BeginRenderPass; it has no Bind/Clear of its own.
    class DH_API Framebuffer
    {
      public:
        virtual ~Framebuffer() = default;

        // Recreates every attachment. Any BindGroup referencing an attachment must be recreated.
        virtual void Resize(size_t newWidth, size_t newHeight) = 0;

        // Texture for attachment `index` (index into the specification's attachments), or nullptr
        // if that attachment is a renderbuffer
        [[nodiscard]] virtual sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const = 0;

        [[nodiscard]] virtual const FramebufferSpecification& GetSpecification() const = 0;

        [[nodiscard]] size_t GetWidth() const { return GetSpecification().width; }
        [[nodiscard]] size_t GetHeight() const { return GetSpecification().height; }

        // Attachment formats, for GraphicsPipelineDesc::targetLayout
        [[nodiscard]] RenderTargetLayout GetLayout() const;

        static sRef<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}
