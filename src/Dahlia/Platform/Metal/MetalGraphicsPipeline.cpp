#include <algorithm>
#include <stdexcept>

#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalGraphicsPipeline.h"
#include "Dahlia/Platform/Metal/MetalShaderModule.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    MetalGraphicsPipeline::MetalGraphicsPipeline(const GraphicsPipelineDesc& desc)
      : GraphicsPipeline(desc)
    {
        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();

        MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
        pipelineDesc->setVertexFunction(GetVertexModule().GetFunction());
        pipelineDesc->setFragmentFunction(GetFragmentModule().GetFunction());

        MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::vertexDescriptor();
        for (const VertexBinding& binding : mDesc.vertexBindings)
        {
            MTL::VertexBufferLayoutDescriptor* layout =
              vertexDesc->layouts()->object(MetalShaderModule::VERTEX_BUFFER_BASE + binding.binding);
            layout->setStride(binding.stride);
            layout->setStepFunction(binding.perInstance ? MTL::VertexStepFunctionPerInstance
                                                        : MTL::VertexStepFunctionPerVertex);
            layout->setStepRate(1);
        }
        for (const VertexAttribute& attribute : mDesc.vertexAttributes)
        {
            MTL::VertexAttributeDescriptor* attr = vertexDesc->attributes()->object(attribute.location);
            attr->setFormat(MTLConv::ToVertexFormat(attribute.format));
            attr->setOffset(attribute.offset);
            attr->setBufferIndex(MetalShaderModule::VERTEX_BUFFER_BASE + attribute.binding);
        }
        pipelineDesc->setVertexDescriptor(vertexDesc);

        const RenderTargetLayout& target = mDesc.targetLayout;
        for (size_t i = 0; i < target.colourFormats.size(); i++)
        {
            MTL::RenderPipelineColorAttachmentDescriptor* colour =
              pipelineDesc->colorAttachments()->object(i);
            colour->setPixelFormat(MTLConv::ToPixelFormat(target.colourFormats[i], device));

            const BlendState blend = mDesc.GetBlend(i);
            if (blend.enabled)
            {
                colour->setBlendingEnabled(true);
                colour->setRgbBlendOperation(MTLConv::ToBlendOperation(blend.colourOp));
                colour->setAlphaBlendOperation(MTLConv::ToBlendOperation(blend.alphaOp));
                colour->setSourceRGBBlendFactor(MTLConv::ToBlendFactor(blend.srcColour));
                colour->setDestinationRGBBlendFactor(MTLConv::ToBlendFactor(blend.dstColour));
                colour->setSourceAlphaBlendFactor(MTLConv::ToBlendFactor(blend.srcAlpha));
                colour->setDestinationAlphaBlendFactor(MTLConv::ToBlendFactor(blend.dstAlpha));
            }
        }
        if (IsDepthFormat(target.depthFormat))
        {
            const MTL::PixelFormat depthFormat = MTLConv::ToPixelFormat(target.depthFormat, device);
            pipelineDesc->setDepthAttachmentPixelFormat(depthFormat);
            if (HasStencil(target.depthFormat))
                pipelineDesc->setStencilAttachmentPixelFormat(depthFormat);
        }
        pipelineDesc->setRasterSampleCount(std::max<uint32_t>(target.samples, 1));

        NS::Error* error = nullptr;
        mPipeline = device->newRenderPipelineState(pipelineDesc, &error);
        pipelineDesc->release();
        if (mPipeline == nullptr)
            throw std::runtime_error(std::string("Failed to create Metal pipeline: ") +
                                     (error ? error->localizedDescription()->utf8String() : "unknown"));

        // As in OpenGL, disabling the depth test also disables depth writes
        MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
        depthDesc->setDepthCompareFunction(mDesc.depth.testEnabled
                                             ? MTLConv::ToCompareFunction(mDesc.depth.compareOp)
                                             : MTL::CompareFunctionAlways);
        depthDesc->setDepthWriteEnabled(mDesc.depth.testEnabled && mDesc.depth.writeEnabled);
        mDepthStencil = device->newDepthStencilState(depthDesc);
        depthDesc->release();

        mPolygonMode = mDesc.raster.polygonMode;
        if (mPolygonMode == PolygonMode::Points)
        {
            DH_WARN(DH::MetalGraphicsPipeline,
                    "Metal has no point fill mode; drawing with PolygonMode::Solid instead");
            mPolygonMode = PolygonMode::Solid;
        }
    }

    MetalGraphicsPipeline::~MetalGraphicsPipeline()
    {
        mPipeline->release();
        mDepthStencil->release();
    }

    const MetalShaderModule& MetalGraphicsPipeline::GetVertexModule() const
    {
        return static_cast<const MetalShaderModule&>(*mDesc.vertexShader);
    }

    const MetalShaderModule& MetalGraphicsPipeline::GetFragmentModule() const
    {
        return static_cast<const MetalShaderModule&>(*mDesc.fragmentShader);
    }
}
