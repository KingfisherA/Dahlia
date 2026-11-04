#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/CommandList.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"

namespace DH
{
    class VulkanDisplay;
    class VulkanGraphicsPipeline;
    class VulkanImage;

    // Records into the display's per-frame command buffer using dynamic rendering.
    //
    // OpenGL conventions: the viewport is flipped (negative height) so NDC +y points up, and
    // viewport/scissor rectangles are given with a top left origin like every backend.
    class DH_API VulkanCommandList : public CommandList
    {
      public:
        explicit VulkanCommandList(VulkanDisplay& display);

        // Called by the display when a frame starts
        void Begin(VkCommandBuffer cmd);
        [[nodiscard]] VkCommandBuffer GetCommandBuffer() const { return mCmd; }

        void BeginRenderPass(const RenderPassDesc& desc) override;
        void EndRenderPass() override;

        void SetViewport(const glm::vec4& viewport) override;
        void SetScissor(const glm::ivec4& scissor) override;

        void BindPipeline(const GraphicsPipeline& pipeline) override;
        void BindVertexBuffer(uint32_t binding, const Buffer& buffer, size_t offset = 0) override;
        void BindIndexBuffer(const Buffer& buffer, IndexType type, size_t offset = 0) override;
        void BindGroup(uint32_t set, const DH::BindGroup& group) override;
        void PushConstants(const void* data, uint32_t size) override;

        void Draw(uint32_t vertexCount,
                  uint32_t instanceCount = 1,
                  uint32_t firstVertex = 0,
                  uint32_t firstInstance = 0) override;
        void DrawIndexed(uint32_t indexCount,
                         uint32_t instanceCount = 1,
                         uint32_t firstIndex = 0,
                         int32_t vertexOffset = 0,
                         uint32_t firstInstance = 0) override;

        using CommandList::PushConstants;

      private:
        void FlushDrawState();
        void AssertInPass(const char* command) const;

        VulkanDisplay& mDisplay;
        VkCommandBuffer mCmd = VK_NULL_HANDLE;

        bool mInPass = false;
        VkExtent2D mTargetExtent{};
        std::vector<VulkanImage*> mPassColourTargets; // Offscreen targets to make sampleable

        const VulkanGraphicsPipeline* mPipeline = nullptr;
        std::array<const DH::BindGroup*, MAX_BIND_GROUPS> mBindGroups{};
        bool mBindGroupsDirty = false;
        std::array<uint8_t, MAX_PUSH_CONSTANT_SIZE> mPushConstantData{};
        bool mPushConstantsDirty = false;
    };
}
