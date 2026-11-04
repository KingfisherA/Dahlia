#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/CommandList.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    class Display;
    class OpenGLGraphicsPipeline;

    // Immediate mode: every call is executed on the GL context straight away. Vertex attribute
    // pointers, the index buffer and bind group resources depend on the bound pipeline, so they
    // are applied lazily by the next Draw.
    class DH_API OpenGLCommandList : public CommandList
    {
      public:
        explicit OpenGLCommandList(const Display& display);

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
        static constexpr uint32_t MAX_VERTEX_BINDINGS = 16;

        struct VertexBufferBinding
        {
            GLuint buffer = 0;
            size_t offset = 0;
        };

        // Last fixed function state set on the context. Invalidated at the start of each render
        // pass because code outside the command list (e.g. the ImGui renderer) changes GL state.
        struct StateCache
        {
            bool valid = false;
            RasterState raster;
            DepthState depth;
            BlendDesc blend;
        };

        void ResetDrawState();
        void ApplyPipelineState(const GraphicsPipelineDesc& desc);
        void FlushDrawState(uint32_t firstInstance);
        void AssertInPass(const char* command) const;

        const Display& mDisplay;

        bool mInPass = false;
        Dimension2D mTargetSize{ 0, 0 };
        StateCache mState;

        const OpenGLGraphicsPipeline* mPipeline = nullptr;

        std::array<VertexBufferBinding, MAX_VERTEX_BINDINGS> mVertexBuffers{};
        bool mVertexStateDirty = true;
        uint32_t mAppliedFirstInstance = 0;

        GLuint mIndexBuffer = 0;
        size_t mIndexOffset = 0;
        IndexType mIndexType = IndexType::UInt32;

        std::array<const DH::BindGroup*, MAX_BIND_GROUPS> mBindGroups{};
        bool mBindGroupsDirty = true;

        std::array<uint8_t, MAX_PUSH_CONSTANT_SIZE> mPushConstantData{};
    };
}
