#include <cstring>
#include <stdexcept>
#include <string>

#include <Metal/Metal.hpp>

#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Platform/Metal/MetalBuffer.h"
#include "Dahlia/Platform/Metal/MetalCommandList.h"
#include "Dahlia/Platform/Metal/MetalCommon.h"
#include "Dahlia/Platform/Metal/MetalFramebuffer.h"
#include "Dahlia/Platform/Metal/MetalGraphicsPipeline.h"
#include "Dahlia/Platform/Metal/MetalShaderModule.h"
#include "Dahlia/Platform/Metal/MetalTexture.h"

namespace DH
{
    void MetalCommandList::Begin(MTL::CommandBuffer* commandBuffer,
                                 MTL::Texture* backBuffer,
                                 MTL::Texture* backDepth)
    {
        mCommandBuffer = commandBuffer;
        mBackBuffer = backBuffer;
        mBackDepth = backDepth;
        mEncoder = nullptr;
    }

    void MetalCommandList::AssertInPass(const char* command) const
    {
        if (mEncoder == nullptr)
            throw std::logic_error(std::string(command) + " must be called inside a render pass");
    }

    void MetalCommandList::BeginRenderPass(const RenderPassDesc& desc)
    {
        if (mEncoder != nullptr)
            throw std::logic_error("BeginRenderPass called inside another render pass");

        // Autoreleased; lives until the frame's autorelease pool is drained
        mPassDesc = MTL::RenderPassDescriptor::renderPassDescriptor();

        auto setColour = [&](uint32_t index, MTL::Texture* texture)
        {
            MTL::RenderPassColorAttachmentDescriptor* colour = mPassDesc->colorAttachments()->object(index);
            colour->setTexture(texture);
            colour->setLoadAction(MTLConv::ToLoadAction(desc.colourLoadOp));
            colour->setStoreAction(MTL::StoreActionStore);
            colour->setClearColor(MTL::ClearColor::Make(
              desc.clearColour.r, desc.clearColour.g, desc.clearColour.b, desc.clearColour.a));
        };
        auto setDepth = [&](MTL::Texture* texture, bool stencil)
        {
            MTL::RenderPassDepthAttachmentDescriptor* depth = mPassDesc->depthAttachment();
            depth->setTexture(texture);
            depth->setLoadAction(MTLConv::ToLoadAction(desc.depthLoadOp));
            depth->setStoreAction(MTL::StoreActionStore);
            depth->setClearDepth(desc.clearDepth);
            if (stencil)
            {
                MTL::RenderPassStencilAttachmentDescriptor* st = mPassDesc->stencilAttachment();
                st->setTexture(texture);
                st->setLoadAction(MTLConv::ToLoadAction(desc.depthLoadOp));
                st->setStoreAction(MTL::StoreActionStore);
                st->setClearStencil(desc.clearStencil);
            }
        };

        if (desc.target != nullptr)
        {
            auto* framebuffer = static_cast<MetalFramebuffer*>(desc.target);
            mTargetWidth = (uint32_t)framebuffer->GetWidth();
            mTargetHeight = (uint32_t)framebuffer->GetHeight();
            uint32_t index = 0;
            for (const sRef<MetalTexture2D>& texture : framebuffer->GetColourAttachments())
                setColour(index++, texture->GetMetalTexture());
            if (const sRef<MetalTexture2D>& depth = framebuffer->GetDepthAttachment())
                setDepth(depth->GetMetalTexture(), HasStencil(depth->GetFormat()));
        }
        else
        {
            mTargetWidth = (uint32_t)mBackBuffer->width();
            mTargetHeight = (uint32_t)mBackBuffer->height();
            setColour(0, mBackBuffer);
            setDepth(mBackDepth, false);
        }

        mEncoder = mCommandBuffer->renderCommandEncoder(mPassDesc);
        mPipeline = nullptr;
        mIndexBuffer = nullptr;
        mBindGroups.fill(nullptr);
        mBindGroupsDirty = false;

        SetViewport({ 0.0f, 0.0f, (float)mTargetWidth, (float)mTargetHeight });
        SetScissor({ 0, 0, (int)mTargetWidth, (int)mTargetHeight });
    }

    void MetalCommandList::EndRenderPass()
    {
        AssertInPass("EndRenderPass");
        mEncoder->endEncoding();
        mEncoder = nullptr;
    }

    void MetalCommandList::SetViewport(const glm::vec4& viewport)
    {
        AssertInPass("SetViewport");
        // Metal viewports already use a top left origin
        mEncoder->setViewport(MTL::Viewport{ viewport.x, viewport.y, viewport.z, viewport.w, 0.0, 1.0 });
    }

    void MetalCommandList::SetScissor(const glm::ivec4& scissor)
    {
        AssertInPass("SetScissor");
        mEncoder->setScissorRect(
          MTL::ScissorRect{ (NS::UInteger)scissor.x, (NS::UInteger)scissor.y, (NS::UInteger)scissor.z, (NS::UInteger)scissor.w });
    }

    void MetalCommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        AssertInPass("BindPipeline");
        const auto& mtlPipeline = static_cast<const MetalGraphicsPipeline&>(pipeline);
        if (mPipeline == &mtlPipeline)
            return;

        mPipeline = &mtlPipeline;
        const GraphicsPipelineDesc& desc = mtlPipeline.GetDesc();
        mEncoder->setRenderPipelineState(mtlPipeline.GetPipelineState());
        mEncoder->setDepthStencilState(mtlPipeline.GetDepthStencilState());
        mEncoder->setCullMode(MTLConv::ToCullMode(desc.raster.cullMode));
        mEncoder->setFrontFacingWinding(MTLConv::ToWinding(desc.raster.frontFace));
        mEncoder->setTriangleFillMode(mtlPipeline.GetEffectivePolygonMode() == PolygonMode::Lines
                                        ? MTL::TriangleFillModeLines
                                        : MTL::TriangleFillModeFill);
        mBindGroupsDirty = true;
        mPushConstantsDirty = true;
    }

    void MetalCommandList::BindVertexBuffer(uint32_t binding, const Buffer& buffer, size_t offset)
    {
        AssertInPass("BindVertexBuffer");
        mEncoder->setVertexBuffer(static_cast<const MetalBuffer&>(buffer).GetMetalBuffer(),
                                  offset,
                                  MetalShaderModule::VERTEX_BUFFER_BASE + binding);
    }

    void MetalCommandList::BindIndexBuffer(const Buffer& buffer, IndexType type, size_t offset)
    {
        AssertInPass("BindIndexBuffer");
        mIndexBuffer = static_cast<const MetalBuffer&>(buffer).GetMetalBuffer();
        mIndexType = type;
        mIndexOffset = offset;
    }

    void MetalCommandList::BindGroup(uint32_t set, const DH::BindGroup& group)
    {
        AssertInPass("BindGroup");
        if (set >= MAX_BIND_GROUPS)
            throw std::out_of_range("Bind group set index out of range");
        mBindGroups[set] = &group;
        mBindGroupsDirty = true;
    }

    void MetalCommandList::PushConstants(const void* data, uint32_t size)
    {
        AssertInPass("PushConstants");
        if (size > MAX_PUSH_CONSTANT_SIZE)
            throw std::out_of_range("Push constants exceed MAX_PUSH_CONSTANT_SIZE");
        std::memcpy(mPushConstantData.data(), data, size);
        mPushConstantsDirty = true;
    }

    void MetalCommandList::FlushDrawState()
    {
        if (mPipeline == nullptr)
            throw std::logic_error("Draw called without a bound pipeline");

        const MetalShaderModule& vertex = mPipeline->GetVertexModule();
        const MetalShaderModule& fragment = mPipeline->GetFragmentModule();

        if (mBindGroupsDirty)
        {
            for (uint32_t set = 0; set < MAX_BIND_GROUPS; set++)
            {
                const DH::BindGroup* group = mBindGroups[set];
                if (group == nullptr)
                    continue;

                for (const BindGroupEntry& entry : group->GetEntries())
                {
                    for (const MetalShaderModule* module : { &vertex, &fragment })
                    {
                        const MetalShaderModule::ArgumentSlot* slot = module->FindSlot(set, entry.binding);
                        if (slot == nullptr)
                            continue;
                        const bool isVertex = module == &vertex;

                        if (slot->type == ShaderResourceType::UniformBuffer && entry.buffer)
                        {
                            MTL::Buffer* buffer = static_cast<const MetalBuffer&>(*entry.buffer).GetMetalBuffer();
                            if (isVertex)
                                mEncoder->setVertexBuffer(buffer, entry.offset, slot->index);
                            else
                                mEncoder->setFragmentBuffer(buffer, entry.offset, slot->index);
                        }
                        else if (slot->type == ShaderResourceType::CombinedTextureSampler && entry.texture)
                        {
                            MTL::Texture* texture =
                              static_cast<const MetalTexture2D&>(*entry.texture).GetMetalTexture();
                            MTL::SamplerState* sampler =
                              static_cast<const MetalSampler&>(*entry.sampler).GetSamplerState();
                            if (isVertex)
                            {
                                mEncoder->setVertexTexture(texture, slot->index);
                                mEncoder->setVertexSamplerState(sampler, slot->index);
                            }
                            else
                            {
                                mEncoder->setFragmentTexture(texture, slot->index);
                                mEncoder->setFragmentSamplerState(sampler, slot->index);
                            }
                        }
                    }
                }
            }
            mBindGroupsDirty = false;
        }

        const uint32_t pushSize = mPipeline->GetDesc().pushConstantSize;
        if (mPushConstantsDirty && pushSize > 0)
        {
            if (vertex.UsesPushConstants())
                mEncoder->setVertexBytes(mPushConstantData.data(), pushSize, MetalShaderModule::PUSH_CONSTANT_BUFFER_INDEX);
            if (fragment.UsesPushConstants())
                mEncoder->setFragmentBytes(mPushConstantData.data(), pushSize, MetalShaderModule::PUSH_CONSTANT_BUFFER_INDEX);
            mPushConstantsDirty = false;
        }
    }

    void MetalCommandList::Draw(uint32_t vertexCount,
                                uint32_t instanceCount,
                                uint32_t firstVertex,
                                uint32_t firstInstance)
    {
        AssertInPass("Draw");
        FlushDrawState();
        mEncoder->drawPrimitives(MTLConv::ToPrimitiveType(mPipeline->GetDesc().topology),
                                 firstVertex,
                                 vertexCount,
                                 instanceCount,
                                 firstInstance);
    }

    void MetalCommandList::DrawIndexed(uint32_t indexCount,
                                       uint32_t instanceCount,
                                       uint32_t firstIndex,
                                       int32_t vertexOffset,
                                       uint32_t firstInstance)
    {
        AssertInPass("DrawIndexed");
        if (mIndexBuffer == nullptr)
            throw std::logic_error("DrawIndexed called without a bound index buffer");
        FlushDrawState();

        const size_t indexSize = mIndexType == IndexType::UInt16 ? 2 : 4;
        mEncoder->drawIndexedPrimitives(MTLConv::ToPrimitiveType(mPipeline->GetDesc().topology),
                                        indexCount,
                                        mIndexType == IndexType::UInt16 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32,
                                        mIndexBuffer,
                                        mIndexOffset + firstIndex * indexSize,
                                        instanceCount,
                                        vertexOffset,
                                        firstInstance);
    }
}
