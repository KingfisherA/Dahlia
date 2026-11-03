#pragma once

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/ShaderModule.h"

namespace DH
{
    // SPIR-V is consumed directly. OpenGL conventions are applied by the pipeline (depth clip
    // control) and command list (flipped viewport), so the shader is not modified.
    class DH_API VulkanShaderModule : public ShaderModule
    {
      public:
        VulkanShaderModule(ShaderStage stage, std::span<const uint32_t> spirv);
        VulkanShaderModule(const VulkanShaderModule&) = delete;
        ~VulkanShaderModule() override;

        [[nodiscard]] VkShaderModule GetModule() const { return mModule; }

      private:
        VkShaderModule mModule = VK_NULL_HANDLE;
    };
}
