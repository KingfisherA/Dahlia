#include <algorithm>
#include <array>
#include <vector>

#include "Dahlia/Platform/Vulkan/VulkanBindGroup.h"
#include "Dahlia/Platform/Vulkan/VulkanCommon.h"
#include "Dahlia/Platform/Vulkan/VulkanContext.h"
#include "Dahlia/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Dahlia/Platform/Vulkan/VulkanShaderModule.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDesc& desc)
      : GraphicsPipeline(desc)
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        VkDevice device = ctx->GetDevice();

        // Layout
        std::vector<VkDescriptorSetLayout> setLayouts;
        for (const sRef<BindGroupLayout>& layout : mDesc.bindGroupLayouts)
        {
            if (!layout)
                throw std::invalid_argument("Vulkan pipelines cannot have empty bind group layouts");
            setLayouts.push_back(static_cast<const VulkanBindGroupLayout&>(*layout).GetLayout());
        }

        VkPushConstantRange pushRange{ PUSH_CONSTANT_STAGES, 0, mDesc.pushConstantSize };
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
        layoutInfo.pSetLayouts = setLayouts.data();
        layoutInfo.pushConstantRangeCount = mDesc.pushConstantSize > 0 ? 1 : 0;
        layoutInfo.pPushConstantRanges = &pushRange;
        DH_VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &mLayout),
                    "vkCreatePipelineLayout");

        // Shaders
        const std::array<VkPipelineShaderStageCreateInfo, 2> stages = {
            VkPipelineShaderStageCreateInfo{
              VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
              nullptr,
              0,
              VK_SHADER_STAGE_VERTEX_BIT,
              static_cast<const VulkanShaderModule&>(*mDesc.vertexShader).GetModule(),
              "main",
              nullptr },
            VkPipelineShaderStageCreateInfo{
              VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
              nullptr,
              0,
              VK_SHADER_STAGE_FRAGMENT_BIT,
              static_cast<const VulkanShaderModule&>(*mDesc.fragmentShader).GetModule(),
              "main",
              nullptr },
        };

        // Vertex input
        std::vector<VkVertexInputBindingDescription> bindings;
        for (const VertexBinding& binding : mDesc.vertexBindings)
        {
            bindings.push_back({ binding.binding,
                                 binding.stride,
                                 binding.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE
                                                     : VK_VERTEX_INPUT_RATE_VERTEX });
        }
        std::vector<VkVertexInputAttributeDescription> attributes;
        for (const VertexAttribute& attribute : mDesc.vertexAttributes)
        {
            attributes.push_back({ attribute.location,
                                   attribute.binding,
                                   VK::ToVertexFormat(attribute.format),
                                   attribute.offset });
        }
        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = (uint32_t)bindings.size();
        vertexInput.pVertexBindingDescriptions = bindings.data();
        vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
        vertexInput.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK::ToTopology(mDesc.topology);

        // OpenGL clip space depth ([-1, 1])
        VkPipelineViewportDepthClipControlCreateInfoEXT clipControl{};
        clipControl.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT;
        clipControl.negativeOneToOne = VK_TRUE;
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = &clipControl;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        PolygonMode polygonMode = mDesc.raster.polygonMode;
        if (polygonMode != PolygonMode::Solid && !ctx->SupportsNonSolidFill())
        {
            DH_WARN(DH::VulkanGraphicsPipeline,
                    "Device lacks fillModeNonSolid; drawing with PolygonMode::Solid instead");
            polygonMode = PolygonMode::Solid;
        }
        VkPipelineRasterizationStateCreateInfo raster{};
        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster.polygonMode = VK::ToPolygonMode(polygonMode);
        raster.cullMode = VK::ToCullMode(mDesc.raster.cullMode);
        raster.frontFace = VK::ToFrontFace(mDesc.raster.frontFace);
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples =
          (VkSampleCountFlagBits)std::max<uint32_t>(mDesc.targetLayout.samples, 1);

        // As in OpenGL, disabling the depth test also disables depth writes
        VkPipelineDepthStencilStateCreateInfo depth{};
        depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth.depthTestEnable = mDesc.depth.testEnabled ? VK_TRUE : VK_FALSE;
        depth.depthWriteEnable = mDesc.depth.testEnabled && mDesc.depth.writeEnabled ? VK_TRUE : VK_FALSE;
        depth.depthCompareOp = VK::ToCompareOp(mDesc.depth.compareOp);

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (mDesc.blend != BlendPreset::None)
        {
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            if (mDesc.blend == BlendPreset::Alpha)
            {
                blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            }
            else // Additive, matching glBlendFunc(GL_SRC_ALPHA, GL_ONE)
            {
                blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            }
        }
        const std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(
          mDesc.targetLayout.colourFormats.size(), blendAttachment);
        VkPipelineColorBlendStateCreateInfo blend{};
        blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend.attachmentCount = (uint32_t)blendAttachments.size();
        blend.pAttachments = blendAttachments.data();

        const std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
                                                              VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount = (uint32_t)dynamicStates.size();
        dynamic.pDynamicStates = dynamicStates.data();

        // Attachment formats (dynamic rendering)
        std::vector<VkFormat> colourFormats;
        for (Format format : mDesc.targetLayout.colourFormats)
            colourFormats.push_back(ctx->ToTextureFormat(format));
        const Format depthFormat = mDesc.targetLayout.depthFormat;
        VkPipelineRenderingCreateInfo rendering{};
        rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        rendering.colorAttachmentCount = (uint32_t)colourFormats.size();
        rendering.pColorAttachmentFormats = colourFormats.data();
        rendering.depthAttachmentFormat =
          IsDepthFormat(depthFormat) ? ctx->ToTextureFormat(depthFormat) : VK_FORMAT_UNDEFINED;
        rendering.stencilAttachmentFormat =
          HasStencil(depthFormat) ? ctx->ToTextureFormat(depthFormat) : VK_FORMAT_UNDEFINED;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &rendering;
        pipelineInfo.stageCount = (uint32_t)stages.size();
        pipelineInfo.pStages = stages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &raster;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pDepthStencilState = &depth;
        pipelineInfo.pColorBlendState = &blend;
        pipelineInfo.pDynamicState = &dynamic;
        pipelineInfo.layout = mLayout;

        DH_VK_CHECK(
          vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline),
          "vkCreateGraphicsPipelines");
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        auto* ctx = GPUContext::Get<VulkanGPUContext>();
        ctx->DeferDestroy(
          [device = ctx->GetDevice(), pipeline = mPipeline, layout = mLayout]()
          {
              vkDestroyPipeline(device, pipeline, nullptr);
              vkDestroyPipelineLayout(device, layout, nullptr);
          });
    }
}
