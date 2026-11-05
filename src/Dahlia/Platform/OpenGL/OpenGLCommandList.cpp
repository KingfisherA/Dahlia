#include <cstring>
#include <stdexcept>
#include <string>

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Platform/OpenGL/OpenGLBuffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLCommandList.h"
#include "Dahlia/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Dahlia/Platform/OpenGL/OpenGLTexture.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    static void SetCapability(GLenum capability, bool enabled)
    {
        if (enabled)
            glEnable(capability);
        else
            glDisable(capability);
    }

    OpenGLCommandList::OpenGLCommandList(const Display& display)
      : mDisplay(display)
    {
    }

    void OpenGLCommandList::AssertInPass(const char* command) const
    {
        if (!mInPass)
            throw std::logic_error(std::string(command) + " must be called inside a render pass");
    }

    void OpenGLCommandList::ResetDrawState()
    {
        mPipeline = nullptr;
        mVertexBuffers.fill({});
        mVertexStateDirty = true;
        mIndexBuffer = 0;
        mIndexOffset = 0;
        mBindGroups.fill(nullptr);
        mBindGroupsDirty = true;
    }

    //
    void OpenGLCommandList::BeginRenderPass(const RenderPassDesc& desc)
    {
        if (mInPass)
            throw std::logic_error("BeginRenderPass called inside another render pass");

        bool hasDepth = true, hasStencil = true; // The default framebuffer is created with D24S8
        if (desc.target != nullptr)
        {
            auto* framebuffer = static_cast<OpenGLFramebuffer*>(desc.target);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->GetID());
            mTargetSize = { framebuffer->GetWidth(), framebuffer->GetHeight() };
            hasDepth = framebuffer->HasDepth();
            hasStencil = framebuffer->HasStencil();
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            mTargetSize = mDisplay.GetFramebufferSize();
        }

        mInPass = true;
        ResetDrawState();

        glViewport(0, 0, (GLsizei)mTargetSize.x, (GLsizei)mTargetSize.y);
        glDisable(GL_SCISSOR_TEST);

        GLbitfield clearBits = 0;
        if (desc.colourLoadOp == LoadOp::Clear)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(desc.clearColour.r, desc.clearColour.g, desc.clearColour.b, desc.clearColour.a);
            clearBits |= GL_COLOR_BUFFER_BIT;
        }
        if (desc.depthLoadOp == LoadOp::Clear && hasDepth)
        {
            // Clears respect the depth/stencil write masks, which a previous pipeline may have
            // disabled
            glDepthMask(GL_TRUE);
            glClearDepth(desc.clearDepth);
            clearBits |= GL_DEPTH_BUFFER_BIT;
            if (hasStencil)
            {
                glStencilMask(0xFF);
                glClearStencil((GLint)desc.clearStencil);
                clearBits |= GL_STENCIL_BUFFER_BIT;
            }
        }
        if (clearBits != 0)
            glClear(clearBits);

        // Anything outside this command list may have changed the fixed function state
        mState.valid = false;
    }

    void OpenGLCommandList::EndRenderPass()
    {
        AssertInPass("EndRenderPass");
        mInPass = false;
        ResetDrawState();
        glBindVertexArray(0);
        glUseProgram(0);

        CheckGLErrors("OpenGLCommandList::EndRenderPass");
    }

    //
    void OpenGLCommandList::SetViewport(const glm::vec4& viewport)
    {
        AssertInPass("SetViewport");
        // The common API puts the origin at the top left; OpenGL's is at the bottom left
        const float y = (float)mTargetSize.y - viewport.y - viewport.w;
        glViewport((GLint)viewport.x, (GLint)y, (GLsizei)viewport.z, (GLsizei)viewport.w);
    }

    void OpenGLCommandList::SetScissor(const glm::ivec4& scissor)
    {
        AssertInPass("SetScissor");
        const GLint y = (GLint)mTargetSize.y - scissor.y - scissor.w;
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor.x, y, scissor.z, scissor.w);
    }

    //
    void OpenGLCommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        AssertInPass("BindPipeline");

        const auto& glPipeline = static_cast<const OpenGLGraphicsPipeline&>(pipeline);
        if (mPipeline == &glPipeline)
            return;

        mPipeline = &glPipeline;
        glUseProgram(glPipeline.GetProgram());
        glBindVertexArray(glPipeline.GetVAO());
        ApplyPipelineState(glPipeline.GetDesc());

        // Attribute pointers and resource slots belong to the new program/VAO
        mVertexStateDirty = true;
        mBindGroupsDirty = true;
    }

    void OpenGLCommandList::ApplyPipelineState(const GraphicsPipelineDesc& desc)
    {
        const bool force = !mState.valid;
        StateCache& s = mState;

        // Rasteriser
        if (force || s.raster.cullMode != desc.raster.cullMode)
        {
            SetCapability(GL_CULL_FACE, desc.raster.cullMode != CullMode::None);
            if (desc.raster.cullMode != CullMode::None)
                glCullFace(desc.raster.cullMode == CullMode::Front ? GL_FRONT : GL_BACK);
        }
        if (force || s.raster.frontFace != desc.raster.frontFace)
            glFrontFace(desc.raster.frontFace == FrontFace::Clockwise ? GL_CW : GL_CCW);
        if (force || s.raster.polygonMode != desc.raster.polygonMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL::ToPolygonMode(desc.raster.polygonMode));

        // Depth
        if (force || s.depth.testEnabled != desc.depth.testEnabled)
            SetCapability(GL_DEPTH_TEST, desc.depth.testEnabled);
        if (force || s.depth.writeEnabled != desc.depth.writeEnabled)
            glDepthMask(desc.depth.writeEnabled ? GL_TRUE : GL_FALSE);
        if (force || s.depth.compareOp != desc.depth.compareOp)
            glDepthFunc(GL::ToCompareFunc(desc.depth.compareOp));

        // Blending. The non-indexed calls set every draw buffer; per-attachment state uses the
        // indexed (GL 4.0) variants.
        if (force || s.blend != desc.blend)
        {
            if (desc.blend.size() <= 1)
            {
                const BlendState blend = desc.GetBlend(0);
                SetCapability(GL_BLEND, blend.enabled);
                if (blend.enabled)
                {
                    glBlendEquationSeparate(GL::ToBlendOp(blend.colourOp),
                                            GL::ToBlendOp(blend.alphaOp));
                    glBlendFuncSeparate(GL::ToBlendFactor(blend.srcColour),
                                        GL::ToBlendFactor(blend.dstColour),
                                        GL::ToBlendFactor(blend.srcAlpha),
                                        GL::ToBlendFactor(blend.dstAlpha));
                }
            }
            else
            {
                for (GLuint i = 0; i < (GLuint)desc.blend.size(); i++)
                {
                    const BlendState& blend = desc.blend[i];
                    if (blend.enabled)
                        glEnablei(GL_BLEND, i);
                    else
                        glDisablei(GL_BLEND, i);
                    glBlendEquationSeparatei(
                      i, GL::ToBlendOp(blend.colourOp), GL::ToBlendOp(blend.alphaOp));
                    glBlendFuncSeparatei(i,
                                         GL::ToBlendFactor(blend.srcColour),
                                         GL::ToBlendFactor(blend.dstColour),
                                         GL::ToBlendFactor(blend.srcAlpha),
                                         GL::ToBlendFactor(blend.dstAlpha));
                }
            }
        }

        s.raster = desc.raster;
        s.depth = desc.depth;
        s.blend = desc.blend;
        s.valid = true;
    }

    //
    void OpenGLCommandList::BindVertexBuffer(uint32_t binding, const Buffer& buffer, size_t offset)
    {
        AssertInPass("BindVertexBuffer");
        if (binding >= MAX_VERTEX_BINDINGS)
            throw std::out_of_range("Vertex buffer binding index out of range");

        mVertexBuffers[binding] = { static_cast<const OpenGLBuffer&>(buffer).GetID(), offset };
        mVertexStateDirty = true;
    }

    void OpenGLCommandList::BindIndexBuffer(const Buffer& buffer, IndexType type, size_t offset)
    {
        AssertInPass("BindIndexBuffer");
        mIndexBuffer = static_cast<const OpenGLBuffer&>(buffer).GetID();
        mIndexType = type;
        mIndexOffset = offset;
        mVertexStateDirty = true;
    }

    void OpenGLCommandList::BindGroup(uint32_t set, const DH::BindGroup& group)
    {
        AssertInPass("BindGroup");
        if (set >= MAX_BIND_GROUPS)
            throw std::out_of_range("Bind group set index out of range");

        mBindGroups[set] = &group;
        mBindGroupsDirty = true;
    }

    void OpenGLCommandList::PushConstants(const void* data, uint32_t size)
    {
        AssertInPass("PushConstants");
        if (size > MAX_PUSH_CONSTANT_SIZE)
            throw std::out_of_range("Push constants exceed MAX_PUSH_CONSTANT_SIZE");

        // Keep a shadow copy so partial updates preserve the remaining bytes (Vulkan semantics),
        // then orphan and re-upload the whole block to avoid stalling on draws still reading it
        std::memcpy(mPushConstantData.data(), data, size);

        const GLuint pushBuffer = GPUContext::Get<OpenGLContext>()->GetPushConstantBuffer();
        glBindBuffer(GL_COPY_WRITE_BUFFER, pushBuffer);
        glBufferData(GL_COPY_WRITE_BUFFER, MAX_PUSH_CONSTANT_SIZE, nullptr, GL_STREAM_DRAW);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, MAX_PUSH_CONSTANT_SIZE, mPushConstantData.data());
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    //
    void OpenGLCommandList::FlushDrawState(uint32_t firstInstance)
    {
        if (mPipeline == nullptr)
            throw std::logic_error("Draw called without a bound pipeline");

        const GraphicsPipelineDesc& desc = mPipeline->GetDesc();

        // OpenGL 4.1 has no base instance, so firstInstance is emulated by offsetting the
        // per-instance attribute pointers
        if (mVertexStateDirty || firstInstance != mAppliedFirstInstance)
        {
            for (const VertexBinding& binding : desc.vertexBindings)
            {
                const VertexBufferBinding& vb = mVertexBuffers.at(binding.binding);
                if (vb.buffer == 0)
                {
                    DH_WARN(DH::OpenGLCommandList,
                            "No vertex buffer bound to binding " << binding.binding);
                    continue;
                }

                glBindBuffer(GL_ARRAY_BUFFER, vb.buffer);
                const size_t instanceOffset =
                  binding.perInstance ? (size_t)firstInstance * binding.stride : 0;

                for (const VertexAttribute& attribute : desc.vertexAttributes)
                {
                    if (attribute.binding != binding.binding)
                        continue;

                    const GL::VertexFormat format = GL::ToVertexFormat(attribute.format);
                    const size_t offset = vb.offset + attribute.offset + instanceOffset;

                    glEnableVertexAttribArray(attribute.location);
                    glVertexAttribPointer(attribute.location,
                                          format.components,
                                          format.type,
                                          format.normalised,
                                          (GLsizei)binding.stride,
                                          reinterpret_cast<const void*>(offset));
                    glVertexAttribDivisor(attribute.location, binding.perInstance ? 1 : 0);
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // The element array binding is VAO state
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

            mVertexStateDirty = false;
            mAppliedFirstInstance = firstInstance;
        }

        if (mBindGroupsDirty)
        {
            for (uint32_t set = 0; set < MAX_BIND_GROUPS; set++)
            {
                const DH::BindGroup* group = mBindGroups[set];
                if (group == nullptr)
                    continue;

                for (const BindGroupEntry& entry : group->GetEntries())
                {
                    const OpenGLGraphicsPipeline::ResourceSlot* slot =
                      mPipeline->FindSlot(set, entry.binding);
                    if (slot == nullptr || slot->slot == OpenGLGraphicsPipeline::NO_SLOT)
                        continue; // Not used by this pipeline's shaders

                    if (slot->type == ShaderResourceType::UniformBuffer && entry.buffer)
                    {
                        const auto& buffer = static_cast<const OpenGLBuffer&>(*entry.buffer);
                        const size_t size =
                          entry.size != 0 ? entry.size : buffer.GetSize() - entry.offset;
                        glBindBufferRange(GL_UNIFORM_BUFFER,
                                          slot->slot,
                                          buffer.GetID(),
                                          (GLintptr)entry.offset,
                                          (GLsizeiptr)size);
                    }
                    else if (IsTextureResource(slot->type) && entry.texture)
                    {
                        const OpenGLTextureBase& texture = AsOpenGLTexture(*entry.texture);
                        glActiveTexture(GL_TEXTURE0 + slot->slot);
                        glBindTexture(texture.GetTarget(), texture.GetID());
                        glBindSampler(slot->slot,
                                      entry.sampler
                                        ? static_cast<const OpenGLSampler&>(*entry.sampler).GetID()
                                        : 0);
                    }
                }
            }
            mBindGroupsDirty = false;
        }

        if (mPipeline->GetPushConstantSlot() != OpenGLGraphicsPipeline::NO_SLOT)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER,
                             mPipeline->GetPushConstantSlot(),
                             GPUContext::Get<OpenGLContext>()->GetPushConstantBuffer());
        }
    }

    void OpenGLCommandList::Draw(uint32_t vertexCount,
                                 uint32_t instanceCount,
                                 uint32_t firstVertex,
                                 uint32_t firstInstance)
    {
        AssertInPass("Draw");
        FlushDrawState(firstInstance);

        glDrawArraysInstanced(GL::ToTopology(mPipeline->GetDesc().topology),
                              (GLint)firstVertex,
                              (GLsizei)vertexCount,
                              (GLsizei)instanceCount);

        CheckGLErrors("OpenGLCommandList::Draw");
    }

    void OpenGLCommandList::DrawIndexed(uint32_t indexCount,
                                        uint32_t instanceCount,
                                        uint32_t firstIndex,
                                        int32_t vertexOffset,
                                        uint32_t firstInstance)
    {
        AssertInPass("DrawIndexed");
        if (mIndexBuffer == 0)
            throw std::logic_error("DrawIndexed called without a bound index buffer");

        FlushDrawState(firstInstance);

        const size_t byteOffset = mIndexOffset + (size_t)firstIndex * GL::IndexByteSize(mIndexType);
        glDrawElementsInstancedBaseVertex(GL::ToTopology(mPipeline->GetDesc().topology),
                                          (GLsizei)indexCount,
                                          GL::ToIndexType(mIndexType),
                                          reinterpret_cast<const void*>(byteOffset),
                                          (GLsizei)instanceCount,
                                          vertexOffset);

        CheckGLErrors("OpenGLCommandList::DrawIndexed");
    }
}
