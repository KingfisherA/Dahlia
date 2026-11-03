#pragma once

#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Framebuffer.h"

namespace DH
{
    class MetalTexture2D;

    class DH_API MetalFramebuffer : public Framebuffer
    {
      public:
        explicit MetalFramebuffer(const FramebufferSpecification& spec);

        void Resize(size_t newWidth, size_t newHeight) override;

        [[nodiscard]] sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const override;
        [[nodiscard]] const FramebufferSpecification& GetSpecification() const override
        {
            return mSpecification;
        }

        [[nodiscard]] const std::vector<sRef<MetalTexture2D>>& GetColourAttachments() const
        {
            return mColourAttachments;
        }
        [[nodiscard]] const sRef<MetalTexture2D>& GetDepthAttachment() const { return mDepth; }

      private:
        void CreateAttachments();

        FramebufferSpecification mSpecification;
        std::vector<sRef<MetalTexture2D>> mAttachments;
        std::vector<sRef<MetalTexture2D>> mColourAttachments;
        sRef<MetalTexture2D> mDepth;
    };
}
