//
// Created by charlie on 07/09/24.
//

#include "VulkanShader.h"

#include "VulkanGPUContext.h"

namespace GEng {
    //
    //
    VulkanShader::VulkanShader(Type type, char *spirVCode, size_t codeSize): mType(type) {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = codeSize;
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(spirVCode);

        if (vkCreateShaderModule(vkCtx->GetDevice(), &shaderModuleCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS) {
            GENG_FATAL(GEng::VulkanShader, "Failed to create Vulkan shader module")
            throw std::runtime_error("Failed to create Vulkan shader module");
        }
    }

    //
    VulkanShader::~VulkanShader() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();
        vkDeviceWaitIdle(vkCtx->GetDevice());
        vkDestroyShaderModule(vkCtx->GetDevice(), mShaderModule, nullptr);
    }

    //
    //
} // GEng
