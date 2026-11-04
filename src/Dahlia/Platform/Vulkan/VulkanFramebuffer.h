#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Framebuffer.h"

namespace DH
{
    class VulkanImage;
    class VulkanTexture2D;

    // Attachments are either VulkanTexture2D render targets the framebuffer creates, or existing
    // textures (one mip level / 3D slice through a 2D view). Colour attachments are left in
    // SHADER_READ_ONLY_OPTIMAL between render passes so they can be sampled.
    class DH_API VulkanFramebuffer : public Framebuffer
    {
      public:
        struct Attachment
        {
            sRef<Texture> texture; // Keeps the image alive
            VulkanImage* image = nullptr;
            VkImageView view = VK_NULL_HANDLE;
            Format format = Format::None;
        };

        explicit VulkanFramebuffer(const FramebufferSpecification& spec);

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
        std::vector<sRef<VulkanTexture2D>> mOwned; // Indexed like the specification (null if existing)
        std::vector<Attachment> mColourAttachments;
        std::optional<Attachment> mDepth;
    };
}
