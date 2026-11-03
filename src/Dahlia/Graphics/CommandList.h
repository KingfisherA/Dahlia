#pragma once

#include <cstdint>

#include <glm/vec4.hpp>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"

namespace DH
{
    class Buffer;
    class BindGroup;
    class Framebuffer;
    class GraphicsPipeline;

    struct DH_API RenderPassDesc
    {
        // nullptr renders to the display's current back buffer
        Framebuffer* target = nullptr;

        LoadOp colourLoadOp = LoadOp::Clear;
        glm::vec4 clearColour{ 0.0f, 0.0f, 0.0f, 1.0f }; // Applied to every colour attachment

        LoadOp depthLoadOp = LoadOp::Clear;
        float clearDepth = 1.0f;
        uint32_t clearStencil = 0;
    };

    // Records rendering commands for one frame. Obtained from Display::BeginFrame.
    //   OpenGL: executes each call immediately on the context (nothing is recorded)
    //   Vulkan: records into the frame's VkCommandBuffer (dynamic rendering)
    //   Metal:  MTLCommandBuffer, one MTLRenderCommandEncoder per render pass
    //
    // All draw state (pipeline, vertex/index buffers, bind groups, push constants) must be set
    // inside a render pass and is reset when the pass ends.
    class DH_API CommandList
    {
      public:
        virtual ~CommandList() = default;

        // Also sets the viewport and scissor to cover the whole target
        virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
        virtual void EndRenderPass() = 0;

        // {x, y, width, height} in pixels, origin at the top left of the target
        virtual void SetViewport(const glm::vec4& viewport) = 0;
        virtual void SetScissor(const glm::ivec4& scissor) = 0;

        virtual void BindPipeline(const GraphicsPipeline& pipeline) = 0;
        virtual void BindVertexBuffer(uint32_t binding, const Buffer& buffer, size_t offset = 0) = 0;
        virtual void BindIndexBuffer(const Buffer& buffer, IndexType type, size_t offset = 0) = 0;
        virtual void BindGroup(uint32_t set, const DH::BindGroup& group) = 0;
        // Overwrites bytes [0, size) of the bound pipeline's push constant block
        virtual void PushConstants(const void* data, uint32_t size) = 0;

        virtual void Draw(uint32_t vertexCount,
                          uint32_t instanceCount = 1,
                          uint32_t firstVertex = 0,
                          uint32_t firstInstance = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount,
                                 uint32_t instanceCount = 1,
                                 uint32_t firstIndex = 0,
                                 int32_t vertexOffset = 0,
                                 uint32_t firstInstance = 0) = 0;

        template<typename T>
        void PushConstants(const T& data)
        {
            static_assert(sizeof(T) <= 128, "Push constants are limited to 128 bytes");
            PushConstants(&data, (uint32_t)sizeof(T));
        }
    };
}
