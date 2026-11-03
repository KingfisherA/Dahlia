#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanShaderModule.h"

namespace DH
{
    VulkanShaderModule::VulkanShaderModule(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        mReflection = ShaderModule::Reflect(stage, spirv);

        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = spirv.size_bytes();
        info.pCode = spirv.data();
        DH_VK_CHECK(vkCreateShaderModule(GPUContext::Get<VulkanGPUContext>()->GetDevice(),
                                         &info,
                                         nullptr,
                                         &mModule),
                    "vkCreateShaderModule");
    }

    VulkanShaderModule::~VulkanShaderModule()
    {
        // Pipelines keep their own copy of the code, so the module can go immediately
        vkDestroyShaderModule(GPUContext::Get<VulkanGPUContext>()->GetDevice(), mModule, nullptr);
    }
}
