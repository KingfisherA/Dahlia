#include "GraphicsEngine/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanShader.h"

#include "VulkanGPUContext.h"

namespace GEng {
    //
    //
    VulkanGraphicsPipeline::VulkanGraphicsPipeline() {
        Reset();
    }

    //
    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        vkDeviceWaitIdle(vkCtx->GetDevice());
        if (mGraphicsPipeline != nullptr) {
            vkDestroyPipelineLayout(vkCtx->GetDevice(), mLayout, nullptr);
            vkDestroyPipeline(vkCtx->GetDevice(), mGraphicsPipeline, nullptr);
        }
    }

    //
    void VulkanGraphicsPipeline::Reset() {
        mDynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        mDynamicStates.dynamicStateCount = DEFAULT_DYNAMIC_STATES.size();
        mDynamicStates.pDynamicStates = DEFAULT_DYNAMIC_STATES.data();

        mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mInputAssembly.primitiveRestartEnable = VK_FALSE;

        mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mRasterizer.depthClampEnable = VK_FALSE;
        mRasterizer.rasterizerDiscardEnable = VK_FALSE;
        mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        mRasterizer.lineWidth = 1.0f;
        mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        mRasterizer.depthBiasEnable = VK_FALSE;
        mRasterizer.depthBiasConstantFactor = 0.0f; // Optional
        mRasterizer.depthBiasClamp = 0.0f; // Optional
        mRasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        EnableMultisampling(1, false);
    }

    //
    void VulkanGraphicsPipeline::DefineVertexSpecification(const ShaderAttribute *specs, size_t numOfSpec,
                                                           bool enableInstance) {
        mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        mAttributeDescriptions.resize(numOfSpec);

        uint32_t stride = 0;
        for (size_t i = 0; i < numOfSpec; i++) {
            const ShaderAttribute &spec = specs[i];
            VkVertexInputAttributeDescription &description = mAttributeDescriptions[i];
            description.binding = spec.binding;
            description.location = spec.location;
            description.format = [spec]() {
                switch (spec.vertexSize) {
                    default:
                    case 1:
                        return VK_FORMAT_R32_SFLOAT;
                    case 2:
                        return VK_FORMAT_R32G32_SFLOAT;
                    case 3:
                        return VK_FORMAT_R32G32B32_SFLOAT;
                    case 4:
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                }
            }();
            description.offset = spec.offset * sizeof(float);


            stride += specs->vertexSize * sizeof(float);
        }


        mBindingDescription.binding = 0;
        mBindingDescription.stride = stride;
        mBindingDescription.inputRate = enableInstance
                                            ? VK_VERTEX_INPUT_RATE_INSTANCE
                                            : VK_VERTEX_INPUT_RATE_VERTEX;


        mVertexInputInfo.vertexBindingDescriptionCount = 1;
        mVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mAttributeDescriptions.size());
        mVertexInputInfo.pVertexBindingDescriptions = &mBindingDescription;
        mVertexInputInfo.pVertexAttributeDescriptions = mAttributeDescriptions.data();
    }

    //
    void VulkanGraphicsPipeline::AddVertexShader(VulkanShader *shader) {
        mVertexStage.reset();
        mVertexStage.emplace();
        VkPipelineShaderStageCreateInfo &shaderStageInfo = mVertexStage.value();
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageInfo.module = shader->GetModule();
        shaderStageInfo.pName = "main";
        shaderStageInfo.pSpecializationInfo = nullptr;
    }

    //
    void VulkanGraphicsPipeline::AddFragmentShader(VulkanShader *shader) {
        mFragmentStage.reset();
        mFragmentStage.emplace();
        VkPipelineShaderStageCreateInfo &shaderStageInfo = mFragmentStage.value();
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfo.module = shader->GetModule();
        shaderStageInfo.pName = "main";
        shaderStageInfo.pSpecializationInfo = nullptr;
    }

    //
    void VulkanGraphicsPipeline::AddGeometryShader(VulkanShader *shader) {
        mGeometryStage.reset();
        mGeometryStage.emplace();
        VkPipelineShaderStageCreateInfo &shaderStageInfo = mGeometryStage.value();
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        shaderStageInfo.module = shader->GetModule();
        shaderStageInfo.pName = "main";
        shaderStageInfo.pSpecializationInfo = nullptr;
    }

    //
    void VulkanGraphicsPipeline::SetPrimitive(Primitive primitive) {
        mInputAssembly.topology = [primitive]() {
            switch (primitive) {
                default:
                case Primitive::Triangles:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case Primitive::Lines:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case Primitive::Points:
                    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case Primitive::TriangleFan:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            }
        }();
    }

    //
    void VulkanGraphicsPipeline::SetViewport(uint32_t width, uint32_t height) {
        mViewportWidth = width;
        mViewportHeight = height;
    }

    void VulkanGraphicsPipeline::SetDepthTesting(bool enabled) {
        if (!enabled) {
            mDepthStencil.reset();
            return;
        }

        mDepthStencil.reset();
        mDepthStencil.emplace();
        VkPipelineDepthStencilStateCreateInfo &depthStencil = mDepthStencil.value();
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional
    }

    void VulkanGraphicsPipeline::SetPolygonDrawMode(GPUContext::PolygonMode mode) {
        mRasterizer.polygonMode = [mode]() {
            switch (mode) {
                case GPUContext::PolygonMode::Solid:
                    return VK_POLYGON_MODE_FILL;
                case GPUContext::PolygonMode::Lines:
                    return VK_POLYGON_MODE_LINE;
                case GPUContext::PolygonMode::Points:
                    return VK_POLYGON_MODE_POINT;
            }
        }();
    }

    //
    void VulkanGraphicsPipeline::SetCulling(bool enabled) {
        if (enabled)
            mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        else
            mRasterizer.cullMode = VK_CULL_MODE_NONE;
    }

    //
    void VulkanGraphicsPipeline::SetFrontFace(FrontFace frontFace) {
        mRasterizer.frontFace = [frontFace]() {
            switch (frontFace) {
                case FrontFace::Clockwise:
                    return VK_FRONT_FACE_CLOCKWISE;
                case FrontFace::Counterclockwise:
                    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            }
        }();
    }

    //
    void VulkanGraphicsPipeline::SetBlending(bool enabled, GPUContext::Blending blending) {
        if (!enabled) return;
        if (blending != GPUContext::Blending::Alpha) return;

        mColorBlendAttachment.reset();
        mColorBlendAttachment.emplace();

        auto &colorBlendAttachment = mColorBlendAttachment.value();
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT
                                              | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        mColourBlendState.reset();
        mColourBlendState.emplace();

        VkPipelineColorBlendStateCreateInfo &colorBlending = mColourBlendState.value();
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
    }

    //
    void VulkanGraphicsPipeline::BindPushConstants(VkDeviceSize pushConstantSize, VkShaderStageFlags stageFlags) {
        mPushConstants.reset();
        mPushConstants.emplace();
        VkPushConstantRange &pushConstants = mPushConstants.value();
        pushConstants.offset = 0;
        pushConstants.size = pushConstantSize;
        pushConstants.stageFlags = stageFlags;
    }

    //
    void VulkanGraphicsPipeline::EnableMultisampling(size_t numSamples, bool enableSampleShading) {
        VkPipelineMultisampleStateCreateInfo &multisampling = mMultisampling;
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = enableSampleShading ? VK_TRUE : VK_FALSE;
        multisampling.rasterizationSamples = [numSamples]() {
            switch (numSamples) {
                default:
                case 1:
                    return VK_SAMPLE_COUNT_1_BIT;
                case 2:
                    return VK_SAMPLE_COUNT_2_BIT;
                case 4:
                    return VK_SAMPLE_COUNT_4_BIT;
                case 8:
                    return VK_SAMPLE_COUNT_8_BIT;
                case 16:
                    return VK_SAMPLE_COUNT_16_BIT;
                case 64:
                    return VK_SAMPLE_COUNT_64_BIT;
            }
        }();
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional
    }

    //
    void VulkanGraphicsPipeline::UpdatePipeline(VkRenderPass renderPass) {
        VulkanGPUContext *vkCtx = GPUContext::Get<VulkanGPUContext>();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr; // &mDescriptorSetLayout;

        if (mPushConstants.has_value()) {
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &mPushConstants.value();
        }

        if (vkCreatePipelineLayout(vkCtx->GetDevice(), &pipelineLayoutInfo, nullptr, &mLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout \n");

        std::array<VkPipelineShaderStageCreateInfo, 3> shaderStages;
        uint32_t i = 0;

        if (mVertexStage.has_value())
            shaderStages[i++] = mVertexStage.value();
        if (mFragmentStage.has_value())
            shaderStages[i++] = mFragmentStage.value();
        if (mGeometryStage.has_value())
            shaderStages[i++] = mGeometryStage.value();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) mViewportWidth;
        viewport.height = (float) mViewportHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {mViewportWidth, mViewportHeight};

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = i;
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &mVertexInputInfo;
        pipelineInfo.pInputAssemblyState = &mInputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &mRasterizer;
        pipelineInfo.pMultisampleState = &mMultisampling;
        pipelineInfo.pDepthStencilState = mDepthStencil.has_value() ? &mDepthStencil.value() : nullptr;
        pipelineInfo.pColorBlendState = mColourBlendState.has_value() ? &mColourBlendState.value() : nullptr;
        pipelineInfo.pDynamicState = &mDynamicStates;
        pipelineInfo.layout = mLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(vkCtx->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &mGraphicsPipeline) !=
            VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline\n");
    }

    //
    //
} // GEng
