#pragma once

#include <cstdint>
#include <initializer_list>
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

    // Blend state for a pipeline's colour attachments, indexed like
    // RenderTargetLayout::colourFormats:
    //   empty      blending disabled for every attachment
    //   1 entry    applied to every attachment
    //   N entries  one per colour attachment (N must equal the attachment count)
    // Assign a single preset/state or a list:
    //   desc.blend = BlendPreset::Alpha;
    //   desc.blend = { BlendPreset::None, BlendPreset::AdditiveOne };
    class DH_API BlendDesc
    {
      public:
        BlendDesc() = default;
        BlendDesc(BlendPreset preset) // NOLINT(google-explicit-constructor)
        {
            if (preset != BlendPreset::None)
                mAttachments.emplace_back(preset);
        }
        BlendDesc(const BlendState& state) // NOLINT(google-explicit-constructor)
          : mAttachments{ state }
        {
        }
        BlendDesc(std::initializer_list<BlendState> attachments)
          : mAttachments(attachments)
        {
        }
        BlendDesc(std::vector<BlendState> attachments) // NOLINT(google-explicit-constructor)
          : mAttachments(std::move(attachments))
        {
        }

        // Blend state of colour attachment `index`
        [[nodiscard]] BlendState Get(size_t index) const
        {
            if (mAttachments.empty())
                return {};
            return mAttachments.size() == 1 ? mAttachments[0] : mAttachments.at(index);
        }

        [[nodiscard]] size_t size() const { return mAttachments.size(); }
        [[nodiscard]] bool empty() const { return mAttachments.empty(); }
        [[nodiscard]] const BlendState& operator[](size_t index) const { return mAttachments[index]; }
        [[nodiscard]] auto begin() const { return mAttachments.begin(); }
        [[nodiscard]] auto end() const { return mAttachments.end(); }
        [[nodiscard]] const std::vector<BlendState>& GetAttachments() const { return mAttachments; }

        bool operator==(const BlendDesc&) const = default;

      private:
        std::vector<BlendState> mAttachments;
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

        // A preset, or one blend state per colour attachment (see BlendDesc)
        BlendDesc blend;

        RenderTargetLayout targetLayout;

        // Index in this vector is the shader `set` number
        std::vector<sRef<BindGroupLayout>> bindGroupLayouts;
        // Visible to all graphics stages. At most MAX_PUSH_CONSTANT_SIZE bytes
        uint32_t pushConstantSize = 0;

        // Blend state of colour attachment `index`
        [[nodiscard]] BlendState GetBlend(size_t index) const { return blend.Get(index); }
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
