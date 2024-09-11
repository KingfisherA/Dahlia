#pragma once
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"
#include "VulkanGraphicsPipeline.h"

namespace GEng {
    class VulkanDisplay;

    class VulkanCommandPool {
    public:
        VulkanCommandPool(uint32_t queueFamilyIndex);
        ~VulkanCommandPool();

        VkCommandPool GetCommandPool() const { return mCommandPool; }

    private:
        VkCommandPool mCommandPool = nullptr;
    };

    class VulkanCommandBuffersBase {
    public:
        virtual ~VulkanCommandBuffersBase() = default;
        virtual VkCommandBuffer GetBuffer() const =0;
    };

    class VulkanOneTimeCommandBuffer : public VulkanCommandBuffersBase {
    public:
        VulkanOneTimeCommandBuffer(VulkanCommandPool *pool);
        ~VulkanOneTimeCommandBuffer() override;

        void Begin();
        void End();
        void Submit(VkQueue queue);

        void EndAndSubmit(VkQueue queue) {
            End();
            Submit(queue);
        }

        VkCommandBuffer GetBuffer() const { return mCommandBuffer; }

    private:
        VkCommandBuffer mCommandBuffer = nullptr;
        VkCommandPool mCommandPool = nullptr;
    };

    class VulkanDrawingCommands {
    public:
        VulkanDrawingCommands() = default;
        VulkanDrawingCommands(const VulkanDrawingCommands &other) = default;

        ~VulkanDrawingCommands() {
            free(mPushConstantData);
        }

        //
        void BindGraphicsPipeline(VulkanGraphicsPipeline *pipeline) { mPipeline = pipeline; }
        //
        void AddVertexBuffer(VulkanVertexBuffer *buffer) { mVertexBuffers.push_back(buffer); }
        void ClearVertexBuffers() { mVertexBuffers.clear(); }
        //
        void BindIndexBuffer(VulkanIndexBuffer *buffer) { mIndexBuffer = buffer; }
        //
        void UploadPushConstants(const void *data, size_t size, VkShaderStageFlags shaderStages) {
            free(mPushConstantData);
            mPushConstantData = malloc(size);
            memcpy(mPushConstantData, data, size);
            mPushConstantDataSize = size;
            mPushConstantShaderStages = shaderStages;
        }

        void BindDescriptorSets() {
        }

        //
        void Draw(size_t elementCount = 0, size_t instanceCount = 1) {
            mElementCount = elementCount;
            mInstanceCount = instanceCount;
        }

        //
    private:
        friend class VulkanDisplay;

        VkCommandBuffer mCommandBuffer = nullptr;
        VkCommandPool mCommandPool = nullptr;

        VulkanGraphicsPipeline *mPipeline = nullptr;
        std::vector<VulkanVertexBuffer *> mVertexBuffers;
        VulkanIndexBuffer *mIndexBuffer = nullptr;

        VkShaderStageFlags mPushConstantShaderStages;
        size_t mPushConstantDataSize = 0;
        void *mPushConstantData = nullptr;

        size_t mElementCount = 0, mInstanceCount = 1;
    };
} // GEng
