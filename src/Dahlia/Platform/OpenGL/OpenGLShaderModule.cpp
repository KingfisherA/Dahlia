#include <cstdlib>
#include <stdexcept>
#include <vector>

#include <spirv_glsl.hpp>

#include "Dahlia/Platform/OpenGL/OpenGLShaderModule.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    std::string OpenGLShaderModule::UniformBlockName(uint32_t set, uint32_t binding)
    {
        return "DH_UBO_s" + std::to_string(set) + "_b" + std::to_string(binding);
    }

    std::string OpenGLShaderModule::SamplerName(uint32_t set, uint32_t binding)
    {
        return "DH_Tex_s" + std::to_string(set) + "_b" + std::to_string(binding);
    }

    std::string OpenGLShaderModule::VaryingName(uint32_t location)
    {
        return "DH_Varying" + std::to_string(location);
    }

    static std::string TranslateToGLSL410(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        spirv_cross::CompilerGLSL compiler(spirv.data(), spirv.size());
        const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        auto setBinding = [&compiler](spirv_cross::ID id)
        {
            return std::pair{ compiler.get_decoration(id, spv::DecorationDescriptorSet),
                              compiler.get_decoration(id, spv::DecorationBinding) };
        };

        for (const spirv_cross::Resource& ubo : resources.uniform_buffers)
        {
            auto [set, binding] = setBinding(ubo.id);
            compiler.set_name(ubo.base_type_id, OpenGLShaderModule::UniformBlockName(set, binding));
            compiler.unset_decoration(ubo.id, spv::DecorationBinding);
            compiler.unset_decoration(ubo.id, spv::DecorationDescriptorSet);
        }

        for (const spirv_cross::Resource& image : resources.sampled_images)
        {
            auto [set, binding] = setBinding(image.id);
            compiler.set_name(image.id, OpenGLShaderModule::SamplerName(set, binding));
            compiler.unset_decoration(image.id, spv::DecorationBinding);
            compiler.unset_decoration(image.id, spv::DecorationDescriptorSet);
        }

        for (const spirv_cross::Resource& pc : resources.push_constant_buffers)
            compiler.set_name(pc.base_type_id, OpenGLShaderModule::PUSH_CONSTANT_BLOCK_NAME);

        // Without separate shader objects, GL links vertex outputs to fragment inputs by name.
        // Optimised SPIR-V has no names, so name the interface variables from their location.
        const auto& varyings =
          stage == ShaderStage::Vertex ? resources.stage_outputs : resources.stage_inputs;
        for (const spirv_cross::Resource& varying : varyings)
        {
            const uint32_t location = compiler.get_decoration(varying.id, spv::DecorationLocation);
            compiler.set_name(varying.id, OpenGLShaderModule::VaryingName(location));
        }

        if (!resources.storage_buffers.empty() || !resources.storage_images.empty() ||
            !resources.separate_images.empty() || !resources.separate_samplers.empty())
        {
            throw std::runtime_error(
              "Shader uses storage buffers/images or separate samplers, which OpenGL 4.1 does not "
              "support. Use uniform buffers and combined image samplers (sampler2D).");
        }

        spirv_cross::CompilerGLSL::Options options;
        options.version = 410;
        options.es = false;
        options.vulkan_semantics = false;
        options.enable_420pack_extension = false; // No layout(binding) on macOS
        options.emit_push_constant_as_uniform_buffer = true;
        options.separate_shader_objects = false;
        compiler.set_common_options(options);

        return compiler.compile();
    }

    OpenGLShaderModule::OpenGLShaderModule(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        mReflection = ShaderModule::Reflect(stage, spirv);
        mGLSLSource = TranslateToGLSL410(stage, spirv);

#ifdef CONFIG_DEBUG
        // Set DH_DUMP_GLSL=1 to print the GLSL generated from each SPIR-V module
        if (const char* dump = std::getenv("DH_DUMP_GLSL"); dump != nullptr && dump[0] == '1')
        {
            DH_LOG(Blue,
                   DH::OpenGLShaderModule,
                   (stage == ShaderStage::Vertex ? "Vertex" : "Fragment")
                     << " shader GLSL:\n"
                     << mGLSLSource);
        }
#endif

        mShaderID = glCreateShader(stage == ShaderStage::Vertex ? GL_VERTEX_SHADER
                                                                : GL_FRAGMENT_SHADER);
        const char* source = mGLSLSource.c_str();
        glShaderSource(mShaderID, 1, &source, nullptr);
        glCompileShader(mShaderID);

        GLint compiled = GL_FALSE;
        glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
            GLint logLength = 0;
            glGetShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log((size_t)logLength + 1, '\0');
            glGetShaderInfoLog(mShaderID, logLength, nullptr, log.data());

            glDeleteShader(mShaderID);
            mShaderID = 0;

            DH_FATAL(DH::OpenGLShaderModule,
                     "Translated GLSL failed to compile:\n"
                       << log.data() << "\n--- Generated source ---\n"
                       << mGLSLSource);
            throw std::runtime_error("OpenGL shader compilation failed");
        }
    }

    OpenGLShaderModule::~OpenGLShaderModule()
    {
        if (mShaderID != 0)
            glDeleteShader(mShaderID);
        mShaderID = 0;
    }
}
