#pragma once

#include <string>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    // Translates SPIR-V to GLSL 410 with SPIRV-Cross and compiles it to a GL shader object.
    //
    // GLSL 410 has no layout(binding = N), so every resource is renamed to a deterministic name
    // derived from its set/binding. After linking, OpenGLGraphicsPipeline looks the names up and
    // assigns uniform block bindings and sampler texture units. Because the names only depend on
    // set/binding, a block declared in both stages links as a single GL uniform block.
    class DH_API OpenGLShaderModule : public ShaderModule
    {
      public:
        OpenGLShaderModule(ShaderStage stage, std::span<const uint32_t> spirv);
        OpenGLShaderModule(const OpenGLShaderModule&) = delete;
        ~OpenGLShaderModule() override;

        [[nodiscard]] GLuint GetShaderID() const { return mShaderID; }
        [[nodiscard]] const std::string& GetGLSLSource() const { return mGLSLSource; }

        // Names given to resources in the generated GLSL
        static std::string UniformBlockName(uint32_t set, uint32_t binding);
        static std::string SamplerName(uint32_t set, uint32_t binding);
        // Vertex outputs and fragment inputs are named from their location so the stages link
        static std::string VaryingName(uint32_t location);
        static constexpr const char* PUSH_CONSTANT_BLOCK_NAME = "DH_PushConstants";

      private:
        std::string mGLSLSource;
        GLuint mShaderID = 0;
    };
}
