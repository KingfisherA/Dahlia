#include <cstring>
#include <stdexcept>
#include <string>

#include "Dahlia/Platform/Vulkan/VulkanBindGroup.h"
#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanCommandList.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanDisplay.h"
#include "Dahlia/Platform/Vulkan/VulkanFramebuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Dahlia/Platform/Vulkan/VulkanSwapChain.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    VulkanCommandList::VulkanCommandList(VulkanDisplay& display)
      : mDisplay(display)
    {
    }

    void VulkanCommandList::Begin(VkCommandBuffer cmd)
    {
        mCmd = cmd;
        mInPass = false;
    }

    void VulkanCommandList::AssertInPass(const char* command) const
    {
        if (!mInPass)
            throw std::logic_error(std::string(command) + " must be called inside a render pass");
    }

    void VulkanCommandList::BeginRenderPass(const RenderPassDesc& desc)
    {
        if (mInPass)
            throw std::logic_error("BeginRenderPass called inside another render pass");

        std::vector<VkRenderingAttachmentInfo> colourAttachments;
        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        bool hasDepth = false, hasStencil = false;

        auto colourInfo = [&desc](VkImageView view)
        {
            VkRenderingAttachmentInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            info.imageView = view;
            info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            info.loadOp = VK::ToLoadOp(desc.colourLoadOp);
            info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            info.clearValue.color = { { desc.clearColour.r,
                                        desc.clearColour.g,
                                        desc.clearColour.b,
                                        desc.clearColour.a } };
            return info;
        };
        auto setDepth = [&](VulkanImage& image, VkImageView view, Format format)
        {
            image.TransitionTo(mCmd, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            depthAttachment.imageView = view;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = VK::ToLoadOp(desc.depthLoadOp);
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.clearValue.depthStencil = { desc.clearDepth, desc.clearStencil };
            hasDepth = true;
            hasStencil = HasStencil(format);
        };

        mPassColourTargets.clear();
        if (desc.target != nullptr)
        {
            auto* framebuffer = static_cast<VulkanFramebuffer*>(desc.target);
            mTargetExtent = { (uint32_t)framebuffer->GetWidth(), (uint32_t)framebuffer->GetHeight() };
            for (const VulkanFramebuffer::Attachment& attachment : framebuffer->GetColourAttachments())
            {
                attachment.image->TransitionTo(mCmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                colourAttachments.push_back(colourInfo(attachment.view));
                mPassColourTargets.push_back(attachment.image);
            }
            if (const auto& depth = framebuffer->GetDepthAttachment())
                setDepth(*depth->image, depth->view, depth->format);
        }
        else
        {
            VulkanSwapChain& swapChain = mDisplay.GetSwapChain();
            const uint32_t image = mDisplay.GetCurrentImageIndex();
            mTargetExtent = swapChain.GetExtent();

            VkImageLayout& layout = swapChain.ImageLayout(image);
            VK::TransitionImage(mCmd,
                                swapChain.GetImage(image),
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                layout,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colourAttachments.push_back(colourInfo(swapChain.GetView(image)));
            VulkanTexture2D& depth = swapChain.GetDepth();
            setDepth(depth, depth.GetAttachmentView(0, 0), depth.GetFormat());
        }

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { { 0, 0 }, mTargetExtent };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = (uint32_t)colourAttachments.size();
        renderingInfo.pColorAttachments = colourAttachments.data();
        renderingInfo.pDepthAttachment = hasDepth ? &depthAttachment : nullptr;
        renderingInfo.pStencilAttachment = hasStencil ? &depthAttachment : nullptr;
        vkCmdBeginRendering(mCmd, &renderingInfo);

        mInPass = true;
        mPipeline = nullptr;
        mBindGroups.fill(nullptr);
        mBindGroupsDirty = false;

        SetViewport({ 0.0f, 0.0f, (float)mTargetExtent.width, (float)mTargetExtent.height });
        SetScissor({ 0, 0, (int)mTargetExtent.width, (int)mTargetExtent.height });
    }

    void VulkanCommandList::EndRenderPass()
    {
        AssertInPass("EndRenderPass");
        vkCmdEndRendering(mCmd);
        mInPass = false;

        // Offscreen colour targets rest in SHADER_READ_ONLY so they can be sampled
        for (VulkanImage* image : mPassColourTargets)
            image->TransitionTo(mCmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        mPassColourTargets.clear();
    }

    void VulkanCommandList::SetViewport(const glm::vec4& viewport)
    {
        AssertInPass("SetViewport");
        // Negative height flips NDC so +y points up, matching OpenGL
        VkViewport vp{ viewport.x, viewport.y + viewport.w, viewport.z, -viewport.w, 0.0f, 1.0f };
        vkCmdSetViewport(mCmd, 0, 1, &vp);
    }

    void VulkanCommandList::SetScissor(const glm::ivec4& scissor)
    {
        AssertInPass("SetScissor");
        VkRect2D rect{ { scissor.x, scissor.y }, { (uint32_t)scissor.z, (uint32_t)scissor.w } };
        vkCmdSetScissor(mCmd, 0, 1, &rect);
    }

    void VulkanCommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        AssertInPass("BindPipeline");
        const auto& vkPipeline = static_cast<const VulkanGraphicsPipeline&>(pipeline);
        if (mPipeline == &vkPipeline)
            return;

        mPipeline = &vkPipeline;
        vkCmdBindPipeline(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline.GetPipeline());
        mBindGroupsDirty = true;
        mPushConstantsDirty = true;
    }

    void VulkanCommandList::BindVertexBuffer(uint32_t binding, const Buffer& buffer, size_t offset)
    {
        AssertInPass("BindVertexBuffer");
        const VkBuffer vkBuffer = static_cast<const VulkanBuffer&>(buffer).GetBuffer();
        const VkDeviceSize vkOffset = offset;
        vkCmdBindVertexBuffers(mCmd, binding, 1, &vkBuffer, &vkOffset);
    }

    void VulkanCommandList::BindIndexBuffer(const Buffer& buffer, IndexType type, size_t offset)
    {
        AssertInPass("BindIndexBuffer");
        vkCmdBindIndexBuffer(
          mCmd, static_cast<const VulkanBuffer&>(buffer).GetBuffer(), offset, VK::ToIndexType(type));
    }

    void VulkanCommandList::BindGroup(uint32_t set, const DH::BindGroup& group)
    {
        AssertInPass("BindGroup");
        if (set >= MAX_BIND_GROUPS)
            throw std::out_of_range("Bind group set index out of range");
        mBindGroups[set] = &group;
        mBindGroupsDirty = true;
    }

    void VulkanCommandList::PushConstants(const void* data, uint32_t size)
    {
        AssertInPass("PushConstants");
        if (size > MAX_PUSH_CONSTANT_SIZE)
            throw std::out_of_range("Push constants exceed MAX_PUSH_CONSTANT_SIZE");
        // Applied at draw time so it works before or after BindPipeline, as in OpenGL
        std::memcpy(mPushConstantData.data(), data, size);
        mPushConstantsDirty = true;
    }

    void VulkanCommandList::FlushDrawState()
    {
        if (mPipeline == nullptr)
            throw std::logic_error("Draw called without a bound pipeline");

        const GraphicsPipelineDesc& desc = mPipeline->GetDesc();
        if (mBindGroupsDirty)
        {
            const uint32_t setCount = (uint32_t)desc.bindGroupLayouts.size();
            for (uint32_t set = 0; set < setCount; set++)
            {
                if (mBindGroups[set] == nullptr)
                    continue;
                const VkDescriptorSet descriptorSet =
                  static_cast<const VulkanBindGroup*>(mBindGroups[set])->GetSet();
                vkCmdBindDescriptorSets(mCmd,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        mPipeline->GetLayout(),
                                        set,
                                        1,
                                        &descriptorSet,
                                        0,
                                        nullptr);
            }
            mBindGroupsDirty = false;
        }

        if (mPushConstantsDirty && desc.pushConstantSize > 0)
        {
            vkCmdPushConstants(mCmd,
                               mPipeline->GetLayout(),
                               VulkanGraphicsPipeline::PUSH_CONSTANT_STAGES,
                               0,
                               desc.pushConstantSize,
                               mPushConstantData.data());
            mPushConstantsDirty = false;
        }
    }

    void VulkanCommandList::Draw(uint32_t vertexCount,
                                 uint32_t instanceCount,
                                 uint32_t firstVertex,
                                 uint32_t firstInstance)
    {
        AssertInPass("Draw");
        FlushDrawState();
        vkCmdDraw(mCmd, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandList::DrawIndexed(uint32_t indexCount,
                                        uint32_t instanceCount,
                                        uint32_t firstIndex,
                                        int32_t vertexOffset,
                                        uint32_t firstInstance)
    {
        AssertInPass("DrawIndexed");
        FlushDrawState();
        vkCmdDrawIndexed(mCmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}
