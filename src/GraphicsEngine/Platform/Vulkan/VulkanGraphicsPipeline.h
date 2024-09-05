#pragma once
#include <optional>
#include<vulkan/vulkan.h>
#include <vector>

#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/GPUContext.h"

namespace GEng {


    struct ShaderAttribute {
        uint32_t binding = 0, location = 0, vertexSize=3, offset=0;
    };

    class VulkanGraphicsPipeline {
    public:
        VulkanGraphicsPipeline();
        virtual ~VulkanGraphicsPipeline();

        enum class FrontFace {
            Clockwise = 0,
            Counterclockwise
        };

        void Reset();

        void DefineVertexSpecification(const ShaderAttribute* specs, size_t numOfSpec, bool enableInstance);

        void AddVertexShader(VkShaderModule shader);
        void AddFragmentShader(VkShaderModule shader);
        void AddGeometryShader(VkShaderModule shader);

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
        VkPipelineVertexInputStateCreateInfo mVertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo mInputAssembly{};
        std::optional<VkPipelineDepthStencilStateCreateInfo> mDepthStencil{};

        std::optional<VkPipelineColorBlendStateCreateInfo> mColourBlendState;

        //To Do
        std::optional<VkPipelineMultisampleStateCreateInfo> mMultisampling{};

        std::optional<VkPushConstantRange> mPushConstants;

        VkPipelineLayout mLayout = nullptr;
        VkPipeline mGraphicsPipeline = nullptr;

        uint32_t mViewportWidth = 0, mViewportHeight = 0;
    };
} // GEng
