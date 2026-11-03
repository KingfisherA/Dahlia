#include <algorithm>
#include <stdexcept>
#include <vector>

#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Platform/OpenGL/OpenGLGraphicsPipeline.h"
#include "Dahlia/Platform/OpenGL/OpenGLShaderModule.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(const GraphicsPipelineDesc& desc)
      : GraphicsPipeline(desc)
    {
        LinkProgram();
        AssignResourceSlots();

        // Vertex attribute pointers are set at draw time, once the vertex buffers are known
        glGenVertexArrays(1, &mVAO);

        CheckGLErrors("OpenGLGraphicsPipeline::OpenGLGraphicsPipeline");
    }

    OpenGLGraphicsPipeline::~OpenGLGraphicsPipeline()
    {
        glDeleteVertexArrays(1, &mVAO);
        glDeleteProgram(mProgram);
        mVAO = 0;
        mProgram = 0;
    }

    void OpenGLGraphicsPipeline::LinkProgram()
    {
        auto* vertex = static_cast<OpenGLShaderModule*>(mDesc.vertexShader.get());
        auto* fragment = static_cast<OpenGLShaderModule*>(mDesc.fragmentShader.get());

        mProgram = glCreateProgram();
        glAttachShader(mProgram, vertex->GetShaderID());
        glAttachShader(mProgram, fragment->GetShaderID());
        glLinkProgram(mProgram);
        glDetachShader(mProgram, vertex->GetShaderID());
        glDetachShader(mProgram, fragment->GetShaderID());

        GLint linked = GL_FALSE;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            GLint logLength = 0;
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log((size_t)logLength + 1, '\0');
            glGetProgramInfoLog(mProgram, logLength, nullptr, log.data());

            glDeleteProgram(mProgram);
            mProgram = 0;

            DH_FATAL(DH::OpenGLGraphicsPipeline, "Program failed to link:\n" << log.data());
            throw std::runtime_error("OpenGL program link failed");
        }
    }

    void OpenGLGraphicsPipeline::AssignResourceSlots()
    {
        // Collect the union of both stages' resources
        for (const sRef<ShaderModule>& module : { mDesc.vertexShader, mDesc.fragmentShader })
        {
            for (const ShaderResource& resource : module->GetReflection().resources)
            {
                if (FindSlot(resource.set, resource.binding) == nullptr)
                    mSlots.push_back({ resource.set, resource.binding, resource.type, NO_SLOT });
            }
        }

        std::sort(mSlots.begin(),
                  mSlots.end(),
                  [](const ResourceSlot& a, const ResourceSlot& b)
                  { return a.set != b.set ? a.set < b.set : a.binding < b.binding; });

        GLint previousProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
        glUseProgram(mProgram);

        GLuint nextUniformBlock = 0, nextTextureUnit = 0;
        for (ResourceSlot& slot : mSlots)
        {
            if (slot.type == ShaderResourceType::UniformBuffer)
            {
                const std::string name = OpenGLShaderModule::UniformBlockName(slot.set, slot.binding);
                const GLuint blockIndex = glGetUniformBlockIndex(mProgram, name.c_str());
                if (blockIndex == GL_INVALID_INDEX)
                    continue; // Optimised out by the GL compiler

                slot.slot = nextUniformBlock++;
                glUniformBlockBinding(mProgram, blockIndex, slot.slot);
            }
            else
            {
                const std::string name = OpenGLShaderModule::SamplerName(slot.set, slot.binding);
                const GLint location = glGetUniformLocation(mProgram, name.c_str());
                if (location < 0)
                    continue;

                slot.slot = nextTextureUnit++;
                glUniform1i(location, (GLint)slot.slot);
            }
        }

        const GLuint pushBlock =
          glGetUniformBlockIndex(mProgram, OpenGLShaderModule::PUSH_CONSTANT_BLOCK_NAME);
        if (pushBlock != GL_INVALID_INDEX)
        {
            mPushConstantSlot = nextUniformBlock++;
            glUniformBlockBinding(mProgram, pushBlock, mPushConstantSlot);
        }

        glUseProgram((GLuint)previousProgram);
    }

    const OpenGLGraphicsPipeline::ResourceSlot* OpenGLGraphicsPipeline::FindSlot(
      uint32_t set,
      uint32_t binding) const
    {
        for (const ResourceSlot& slot : mSlots)
        {
            if (slot.set == set && slot.binding == binding)
                return &slot;
        }
        return nullptr;
    }
}
