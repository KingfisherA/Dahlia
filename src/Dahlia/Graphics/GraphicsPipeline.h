#pragma once

#include <cstdint>
#include <vector>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class ShaderModule;
    class BindGroupLayout;

    // One vertex buffer slot. Buffers are attached to slots at draw time with
    // CommandList::BindVertexBuffer.
    struct DH_API VertexBinding
    {
        uint32_t binding = 0;
        uint32_t stride = 0; // In bytes
        bool perInstance = false;
    };

    // One shader input (`layout(location = N) in ...`) read from a vertex buffer slot
    struct DH_API VertexAttribute
    {
        uint32_t location = 0;
        uint32_t binding = 0;
        Format format = Format::RGB32F;
        uint32_t offset = 0; // In bytes, from the start of the element
    };

    struct DH_API RasterState
    {
        CullMode cullMode = CullMode::None;
        FrontFace frontFace = FrontFace::CounterClockwise;
        PolygonMode polygonMode = PolygonMode::Solid;
    };

    struct DH_API DepthState
    {
        bool testEnabled = true;
        bool writeEnabled = true;
        CompareOp compareOp = CompareOp::Less;
    };

    constexpr uint32_t MAX_BIND_GROUPS = 4;
    constexpr uint32_t MAX_PUSH_CONSTANT_SIZE = 128;

    // All fixed function state needed to draw, baked into one immutable object.
    //   OpenGL: a linked program + VAO; binding it issues the glEnable/glBlendFunc/glPolygonMode...
    //           calls that differ from the current GL state
    //   Vulkan: VkPipeline + VkPipelineLayout (dynamic rendering, dynamic viewport/scissor)
    //   Metal:  MTLRenderPipelineState + MTLDepthStencilState + encoder raster state
    struct DH_API GraphicsPipelineDesc
    {
        sRef<ShaderModule> vertexShader;
        sRef<ShaderModule> fragmentShader;

        std::vector<VertexBinding> vertexBindings;
        std::vector<VertexAttribute> vertexAttributes;

        PrimitiveTopology topology = PrimitiveTopology::Triangles;
        RasterState raster;
        DepthState depth;
        BlendPreset blend = BlendPreset::None;

        RenderTargetLayout targetLayout;

        // Index in this vector is the shader `set` number
        std::vector<sRef<BindGroupLayout>> bindGroupLayouts;
        // Visible to all graphics stages. At most MAX_PUSH_CONSTANT_SIZE bytes
        uint32_t pushConstantSize = 0;
    };

    class DH_API GraphicsPipeline
    {
      public:
        virtual ~GraphicsPipeline() = default;

        [[nodiscard]] const GraphicsPipelineDesc& GetDesc() const { return mDesc; }

        static sRef<GraphicsPipeline> Create(const GraphicsPipelineDesc& desc);

      protected:
        // Validates the description against the shaders' reflection (throws on hard errors, warns
        // about resources missing from the bind group layouts)
        explicit GraphicsPipeline(const GraphicsPipelineDesc& desc);

        GraphicsPipelineDesc mDesc;

      private:
        void Validate() const;
    };
}
