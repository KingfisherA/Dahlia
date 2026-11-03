#include "Dahlia/Platform/Vulkan/VulkanBindGroup.h"
#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanTexture.h"

namespace DH
{
    static VkDescriptorType ToDescriptorType(ShaderResourceType type)
    {
        return type == ShaderResourceType::UniformBuffer ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                                                         : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    static VkShaderStageFlags ToStageFlags(ShaderStage stages)
    {
        VkShaderStageFlags flags = 0;
        if (stages & ShaderStage::Vertex)
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (stages & ShaderStage::Fragment)
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        return flags;
    }

    VulkanBindGroupLayout::VulkanBindGroupLayout(const std::vector<BindGroupLayoutEntry>& entries)
      : BindGroupLayout(entries)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (const BindGroupLayoutEntry& entry : entries)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = entry.binding;
            binding.descriptorType = ToDescriptorType(entry.type);
            binding.descriptorCount = 1;
            binding.stageFlags = ToStageFlags(entry.stages);
            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = (uint32_t)bindings.size();
        info.pBindings = bindings.data();
        DH_VK_CHECK(vkCreateDescriptorSetLayout(
                      GPUContext::Get<VulkanGPUContext>()->GetDevice(), &info, nullptr, &mLayout),
                    "vkCreateDescriptorSetLayout");
    }

    VulkanBindGroupLayout::~VulkanBindGroupLayout()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        ctx->DeferDestroy([device = ctx->GetDevice(), layout = mLayout]()
                          { vkDestroyDescriptorSetLayout(device, layout, nullptr); });
    }

    VulkanBindGroup::VulkanBindGroup(const sRef<BindGroupLayout>& layout,
                                     const std::vector<BindGroupEntry>& entries)
      : BindGroup(layout, entries)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        const VkDescriptorSetLayout setLayout =
          static_cast<const VulkanBindGroupLayout&>(*layout).GetLayout();

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = ctx->GetDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &setLayout;
        DH_VK_CHECK(vkAllocateDescriptorSets(ctx->GetDevice(), &allocInfo, &mSet),
                    "vkAllocateDescriptorSets");

        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;
        bufferInfos.reserve(entries.size());
        imageInfos.reserve(entries.size());
        std::vector<VkWriteDescriptorSet> writes;

        for (const BindGroupEntry& entry : entries)
        {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = mSet;
            write.dstBinding = entry.binding;
            write.descriptorCount = 1;

            if (entry.buffer)
            {
                const auto& buffer = static_cast<const VulkanBuffer&>(*entry.buffer);
                bufferInfos.push_back({ buffer.GetBuffer(),
                                        entry.offset,
                                        entry.size != 0 ? entry.size : VK_WHOLE_SIZE });
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.pBufferInfo = &bufferInfos.back();
            }
            else
            {
                const auto& texture = static_cast<const VulkanTexture2D&>(*entry.texture);
                const auto& sampler = static_cast<const VulkanSampler&>(*entry.sampler);
                imageInfos.push_back({ sampler.GetSampler(),
                                       texture.GetView(),
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.pImageInfo = &imageInfos.back();
            }
            writes.push_back(write);
        }

        vkUpdateDescriptorSets(ctx->GetDevice(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
    }

    VulkanBindGroup::~VulkanBindGroup()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        // Keep the referenced resources alive until the set can no longer be in use
        ctx->DeferDestroy([device = ctx->GetDevice(),
                           pool = ctx->GetDescriptorPool(),
                           set = mSet,
                           entries = mEntries]() { vkFreeDescriptorSets(device, pool, 1, &set); });
    }
}
