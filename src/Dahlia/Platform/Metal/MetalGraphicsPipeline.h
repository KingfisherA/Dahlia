#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"

namespace MTL
{
    class RenderPipelineState;
    class DepthStencilState;
}

namespace DH
{
    class MetalShaderModule;

    // MTLRenderPipelineState (shaders, vertex layout, attachment formats, blending) plus an
    // MTLDepthStencilState. Cull mode, winding and fill mode are encoder state, set by
    // MetalCommandList::BindPipeline.
    class DH_API MetalGraphicsPipeline : public GraphicsPipeline
    {
      public:
        explicit MetalGraphicsPipeline(const GraphicsPipelineDesc& desc);
        MetalGraphicsPipeline(const MetalGraphicsPipeline&) = delete;
        ~MetalGraphicsPipeline() override;

        [[nodiscard]] MTL::RenderPipelineState* GetPipelineState() const { return mPipeline; }
        [[nodiscard]] MTL::DepthStencilState* GetDepthStencilState() const { return mDepthStencil; }
        // Metal has no point fill mode; Points falls back to Solid
        [[nodiscard]] PolygonMode GetEffectivePolygonMode() const { return mPolygonMode; }

        [[nodiscard]] const MetalShaderModule& GetVertexModule() const;
        [[nodiscard]] const MetalShaderModule& GetFragmentModule() const;

      private:
        MTL::RenderPipelineState* mPipeline = nullptr;
        MTL::DepthStencilState* mDepthStencil = nullptr;
        PolygonMode mPolygonMode = PolygonMode::Solid;
    };
}
