#include <stdexcept>

#include <spirv_cross.hpp>

#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLShaderModule.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanShaderModule.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalShaderModule.h"
#endif

namespace DH
{
    sRef<ShaderModule> ShaderModule::Create(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLShaderModule>(stage, spirv);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanShaderModule>(stage, spirv);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalShaderModule>(stage, spirv);
#endif
            default:
                throw std::runtime_error("ShaderModule is not implemented for this graphics API yet");
        }
    }

    ShaderReflection ShaderModule::Reflect(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        spirv_cross::Compiler compiler(spirv.data(), spirv.size());
        const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        ShaderReflection reflection;
        reflection.stage = stage;

        auto addResources = [&](const auto& list, ShaderResourceType type)
        {
            for (const spirv_cross::Resource& resource : list)
            {
                reflection.resources.push_back(
                  { resource.name,
                    compiler.get_decoration(resource.id, spv::DecorationDescriptorSet),
                    compiler.get_decoration(resource.id, spv::DecorationBinding),
                    type });
            }
        };
        addResources(resources.uniform_buffers, ShaderResourceType::UniformBuffer);

        for (const spirv_cross::Resource& resource : resources.sampled_images)
        {
            const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
            if (type.image.arrayed || type.image.ms ||
                (type.image.dim != spv::Dim2D && type.image.dim != spv::Dim3D))
            {
                throw std::runtime_error("Shader sampler '" + resource.name +
                                         "' is not a sampler2D or sampler3D, which are the only "
                                         "sampler types Dahlia supports");
            }
            reflection.resources.push_back(
              { resource.name,
                compiler.get_decoration(resource.id, spv::DecorationDescriptorSet),
                compiler.get_decoration(resource.id, spv::DecorationBinding),
                type.image.dim == spv::Dim3D ? ShaderResourceType::CombinedTextureSampler3D
                                             : ShaderResourceType::CombinedTextureSampler });
        }

        for (const spirv_cross::Resource& pc : resources.push_constant_buffers)
        {
            reflection.pushConstantSize =
              (uint32_t)compiler.get_declared_struct_size(compiler.get_type(pc.base_type_id));
        }

        return reflection;
    }
}
