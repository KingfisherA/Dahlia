#pragma once

#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Framebuffer.h"

namespace DH
{
    class VulkanTexture2D;

    // Attachments are VulkanTexture2D render targets. Colour attachments are left in
    // SHADER_READ_ONLY_OPTIMAL between render passes so they can be sampled.
    class DH_API VulkanFramebuffer : public Framebuffer
    {
      public:
        explicit VulkanFramebuffer(const FramebufferSpecification& spec);

        void Resize(size_t newWidth, size_t newHeight) override;

        [[nodiscard]] sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const override;
        [[nodiscard]] const FramebufferSpecification& GetSpecification() const override
        {
            return mSpecification;
        }

        [[nodiscard]] const std::vector<sRef<VulkanTexture2D>>& GetColourAttachments() const
        {
            return mColourAttachments;
        }
        [[nodiscard]] const sRef<VulkanTexture2D>& GetDepthAttachment() const { return mDepth; }

      private:
        void CreateAttachments();

        FramebufferSpecification mSpecification;
        std::vector<sRef<VulkanTexture2D>> mAttachments; // Indexed like the specification
        std::vector<sRef<VulkanTexture2D>> mColourAttachments;
        sRef<VulkanTexture2D> mDepth;
    };
}
