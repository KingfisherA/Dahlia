#pragma once
#include <vulkan/vulkan_core.h>

namespace GEng {
    class VulkanShader {
    public:
        enum class Type {
            Vertex = 0,
            Fragment,
            Geomertry,
            Compute
        };

        VulkanShader(Type type, char* spirVCode, size_t codeSize);
        VulkanShader(const VulkanShader &other) = delete;
        VulkanShader(VulkanShader &&other) = delete;

        ~VulkanShader();


        [[nodiscard]] Type GetType() const { return mType; }
        [[nodiscard]] VkShaderModule GetModule() const { return mShaderModule; }

    private:
        Type mType;
        VkShaderModule mShaderModule = nullptr;
    };
} // GEng
