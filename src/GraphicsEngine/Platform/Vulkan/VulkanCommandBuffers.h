#pragma once
#include <vulkan/vulkan.h>

namespace GEng {
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
} // GEng
