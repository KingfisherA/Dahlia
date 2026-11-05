#pragma once

#include <optional>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Framebuffer.h"

namespace MTL
{
    class Texture;
}

namespace DH
{
    class MetalTexture2D;

    // Attachments are MetalTexture2D render targets the framebuffer creates, or existing textures
    // (one mip level, or one depth plane of a 3D texture)
    class DH_API MetalFramebuffer : public Framebuffer
    {
      public:
        struct Attachment
        {
            sRef<Texture> owner; // Keeps the texture alive
            MTL::Texture* texture = nullptr;
            uint32_t level = 0;
            uint32_t depthPlane = 0;
            Format format = Format::None;
        };

        explicit MetalFramebuffer(const FramebufferSpecification& spec);

        void Resize(size_t newWidth, size_t newHeight) override;

        [[nodiscard]] sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const override;
        [[nodiscard]] const FramebufferSpecification& GetSpecification() const override
        {
            return mSpecification;
        }

        [[nodiscard]] const std::vector<Attachment>& GetColourAttachments() const
        {
            return mColourAttachments;
        }
        [[nodiscard]] const std::optional<Attachment>& GetDepthAttachment() const { return mDepth; }

      private:
        void CreateAttachments();

        FramebufferSpecification mSpecification;
        std::vector<sRef<MetalTexture2D>> mOwned; // Indexed like the specification (null if existing)
        std::vector<Attachment> mColourAttachments;
        std::optional<Attachment> mDepth;
    };
}
