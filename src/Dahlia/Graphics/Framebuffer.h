#pragma once

#include <concepts>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class Texture;
    class Texture2D;

    // Either a new attachment the framebuffer creates and owns (from a format), or an existing
    // texture created with TextureUsage::RenderTarget. An existing attachment renders into one
    // mip level of a Texture2D, or one depth slice (`layer`) of a Texture3D, and the texture
    // keeps its contents after the framebuffer is destroyed. One texture can be attached to any
    // number of framebuffers, so long-lived textures can be rendered with different MRT sets.
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
        template<std::derived_from<Texture> T>
        FramebufferAttachment(const sRef<T>& texture, // NOLINT(google-explicit-constructor)
                              uint32_t mipLevel = 0,
                              uint32_t layer = 0)
          : texture(texture)
          , mipLevel(mipLevel)
          , layer(layer)
        {
        }

        // New attachment; ignored when `texture` is set (the texture's format is used)
        Format format = Format::None;
        bool renderbuffer = false;

        // Existing attachment
        sRef<Texture> texture;
        uint32_t mipLevel = 0;
        uint32_t layer = 0; // Depth slice of a Texture3D; must be 0 for a Texture2D

        [[nodiscard]] bool IsExisting() const { return texture != nullptr; }
    };

    struct DH_API FramebufferSpecification
    {
      public:
        // May be left 0 when an existing attachment is present: the size of its mip level is used.
        // Every attachment must have the framebuffer's size.
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
        // Throws for framebuffers with existing attachments (create a new framebuffer instead).
        virtual void Resize(size_t newWidth, size_t newHeight) = 0;

        // Texture for attachment `index` (index into the specification's attachments), or nullptr
        // if that attachment is a renderbuffer or an existing Texture3D
        [[nodiscard]] virtual sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const = 0;

        [[nodiscard]] virtual const FramebufferSpecification& GetSpecification() const = 0;

        [[nodiscard]] size_t GetWidth() const { return GetSpecification().width; }
        [[nodiscard]] size_t GetHeight() const { return GetSpecification().height; }

        // Attachment formats, for GraphicsPipelineDesc::targetLayout
        [[nodiscard]] RenderTargetLayout GetLayout() const;

        static sRef<Framebuffer> Create(const FramebufferSpecification& spec);

      protected:
        // Checks existing attachments, fills in their formats and derives the size when it is 0.
        // Called by every backend's constructor.
        static FramebufferSpecification Prepare(const FramebufferSpecification& spec);
        [[nodiscard]] bool HasExistingAttachments() const;
        // Attachment `index` as a Texture2D if it is an existing 2D texture, else nullptr
        [[nodiscard]] sRef<Texture2D> GetExistingTexture2D(size_t index) const;
    };
}
