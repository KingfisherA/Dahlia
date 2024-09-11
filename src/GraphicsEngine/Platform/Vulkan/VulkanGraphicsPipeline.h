#pragma once
#include <optional>
#include<vulkan/vulkan.h>
#include <vector>

#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/GPUContext.h"

namespace GEng {
    class VulkanShader;
}

namespace GEng {
    struct ShaderAttribute {
        uint32_t binding = 0, location = 0, vertexSize = 3, offset = 0;
    };

    class VulkanGraphicsPipeline {
    public:
        VulkanGraphicsPipeline();
        virtual ~VulkanGraphicsPipeline();

        enum class FrontFace {
            Clockwise = 0,
            Counterclockwise
        };

        static inline std::array<VkDynamicState, 2> DEFAULT_DYNAMIC_STATES = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        void Reset();

        void DefineVertexSpecification(const ShaderAttribute *specs, size_t numOfSpec, bool enableInstance);

        void AddVertexShader(VulkanShader* shader);
        void AddFragmentShader(VulkanShader* shader);
        void AddGeometryShader(VulkanShader* shader);

        void SetPrimitive(Primitive primitive);
        void SetViewport(uint32_t width, uint32_t height);
        void SetDepthTesting(bool enabled);

        void SetPolygonDrawMode(GPUContext::PolygonMode mode);
        void SetCulling(bool enabled);
        void SetFrontFace(FrontFace frontFace);

        void SetBlending(bool enabled, GPUContext::Blending blending);

        void BindPushConstants(VkDeviceSize pushConstantSize,
                               VkShaderStageFlags stageFlags);

        void EnableMultisampling(size_t numSamples, bool enableSampleShading);

        void UpdatePipeline(VkRenderPass renderPass);

        VkPipeline GetGraphicsPipeline() const { return mGraphicsPipeline; }
        VkPipelineLayout GetPipelineLayout() const { return mLayout; }

    private:
        std::optional<VkPipelineShaderStageCreateInfo> mVertexStage;
        std::optional<VkPipelineShaderStageCreateInfo> mFragmentStage;
        std::optional<VkPipelineShaderStageCreateInfo> mGeometryStage;

        VkPipelineDynamicStateCreateInfo mDynamicStates{};
        VkPipelineRasterizationStateCreateInfo mRasterizer{};

        std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;
        VkVertexInputBindingDescription mBindingDescription{};
        VkPipelineVertexInputStateCreateInfo mVertexInputInfo{};

        std::optional<VkPushConstantRange> mPushConstants;

        VkPipelineInputAssemblyStateCreateInfo mInputAssembly{};
        std::optional<VkPipelineDepthStencilStateCreateInfo> mDepthStencil{};

        std::optional<VkPipelineColorBlendAttachmentState> mColorBlendAttachment;
        std::optional<VkPipelineColorBlendStateCreateInfo> mColourBlendState;

        //To Do
        VkPipelineMultisampleStateCreateInfo mMultisampling;

        VkPipelineLayout mLayout = nullptr;
        VkPipeline mGraphicsPipeline = nullptr;

        uint32_t mViewportWidth = 0, mViewportHeight = 0;
    };
} // GEng
