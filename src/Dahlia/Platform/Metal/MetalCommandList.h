#pragma once

#include <array>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/CommandList.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"

namespace MTL
{
    class Buffer;
    class CommandBuffer;
    class RenderCommandEncoder;
    class RenderPassDescriptor;
    class Texture;
}

namespace DH
{
    class MetalGraphicsPipeline;

    // One MTLRenderCommandEncoder per render pass on the frame's command buffer. Bind groups and
    // push constants are applied at draw time using each shader stage's argument table layout.
    class DH_API MetalCommandList : public CommandList
    {
      public:
        // Called by the display when a frame starts
        void Begin(MTL::CommandBuffer* commandBuffer, MTL::Texture* backBuffer, MTL::Texture* backDepth);

        [[nodiscard]] MTL::CommandBuffer* GetCommandBuffer() const { return mCommandBuffer; }
        [[nodiscard]] MTL::RenderCommandEncoder* GetEncoder() const { return mEncoder; }
        [[nodiscard]] MTL::RenderPassDescriptor* GetRenderPassDescriptor() const { return mPassDesc; }

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

        MTL::CommandBuffer* mCommandBuffer = nullptr;
        MTL::Texture* mBackBuffer = nullptr;
        MTL::Texture* mBackDepth = nullptr;

        MTL::RenderPassDescriptor* mPassDesc = nullptr;
        MTL::RenderCommandEncoder* mEncoder = nullptr;
        uint32_t mTargetWidth = 0, mTargetHeight = 0;

        const MetalGraphicsPipeline* mPipeline = nullptr;
        MTL::Buffer* mIndexBuffer = nullptr;
        size_t mIndexOffset = 0;
        IndexType mIndexType = IndexType::UInt32;

        std::array<const DH::BindGroup*, MAX_BIND_GROUPS> mBindGroups{};
        bool mBindGroupsDirty = false;
        std::array<uint8_t, MAX_PUSH_CONSTANT_SIZE> mPushConstantData{};
        bool mPushConstantsDirty = false;
    };
}
