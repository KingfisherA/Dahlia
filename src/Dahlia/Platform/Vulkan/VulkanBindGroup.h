#pragma once

#include <vulkan/vulkan.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/BindGroup.h"

namespace DH
{
    class DH_API VulkanBindGroupLayout : public BindGroupLayout
    {
      public:
        explicit VulkanBindGroupLayout(const std::vector<BindGroupLayoutEntry>& entries);
        VulkanBindGroupLayout(const VulkanBindGroupLayout&) = delete;
        ~VulkanBindGroupLayout() override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout() const { return mLayout; }

      private:
        VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;
    };

    // A descriptor set written once at creation. Textures must be in SHADER_READ_ONLY_OPTIMAL
    // when sampled (true for uploaded textures and colour render targets outside their pass).
    class DH_API VulkanBindGroup : public BindGroup
    {
      public:
        VulkanBindGroup(const sRef<BindGroupLayout>& layout, const std::vector<BindGroupEntry>& entries);
        VulkanBindGroup(const VulkanBindGroup&) = delete;
        ~VulkanBindGroup() override;

        [[nodiscard]] VkDescriptorSet GetSet() const { return mSet; }

      private:
        VkDescriptorSet mSet = VK_NULL_HANDLE;
    };
}
