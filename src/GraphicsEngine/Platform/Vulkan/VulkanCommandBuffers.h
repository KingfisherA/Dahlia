#pragma once
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

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
        //
        void BindGraphicsPipeline(VkPipeline pipeline) { mPipeline = pipeline; }
        //
        void BindVertexBuffers(VulkanVertexBuffer *buffer) { mVertexBuffers.push_back(buffer); }
        void ClearVertexBuffers() { mVertexBuffers.clear(); }
        //
        void BindIndexBuffer(VulkanIndexBuffer *buffer) { mIndexBuffer = buffer; }
        //
        void UploadPushConstants() {
        }
        void BindDescriptorSets() {
        }
        //
        void Draw(size_t instanceCount = 1, size_t elementCount = 0) {
            mElementCount = elementCount;
            mInstanceCount = instanceCount;
        }
        //
    private:
        friend class VulkanDisplay;

        VkCommandBuffer mCommandBuffer = nullptr;
        VkCommandPool mCommandPool = nullptr;

        VkPipeline mPipeline = nullptr;
        std::vector<VulkanVertexBuffer*> mVertexBuffers;
        VulkanIndexBuffer* mIndexBuffer = nullptr;

        size_t mElementCount = 0, mInstanceCount = 1;
    };
} // GEng
