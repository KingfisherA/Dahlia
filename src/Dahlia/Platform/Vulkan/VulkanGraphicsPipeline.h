#pragma once

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"

namespace DH
{
    // Built for dynamic rendering (no VkRenderPass) with dynamic viewport/scissor. Uses
    // VK_EXT_depth_clip_control (negativeOneToOne) so shaders keep OpenGL's [-1, 1] clip depth.
    class DH_API VulkanGraphicsPipeline : public GraphicsPipeline
    {
      public:
        explicit VulkanGraphicsPipeline(const GraphicsPipelineDesc& desc);
        VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
        ~VulkanGraphicsPipeline() override;

        [[nodiscard]] VkPipeline GetPipeline() const { return mPipeline; }
        [[nodiscard]] VkPipelineLayout GetLayout() const { return mLayout; }

        static constexpr VkShaderStageFlags PUSH_CONSTANT_STAGES =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      private:
        VkPipelineLayout mLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}
