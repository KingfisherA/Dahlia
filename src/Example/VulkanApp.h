#pragma once

#include <glm/matrix.hpp>

#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/Scene/Camera3D.h"

namespace GEng {
    class VulkanDisplay;
    class VulkanGraphicsPipeline;
    class VulkanVertexBuffer;
    class VulkanIndexBuffer;
}

namespace Example {
    class VulkanApp {
    public:
        VulkanApp() = default;

        bool Init();

        void Run();

    private:
        void UploadPushConstants() const;

        void OnDestroy();
        void OnEvent(GEng::Event &e);

        static std::vector<char> ReadBinaryFile(const char *path);

        GEng::VulkanDisplay *mWindow = nullptr;
        GEng::VulkanGraphicsPipeline *mGraphicsPipeline = nullptr;
        GEng::VulkanVertexBuffer *mVertexBuffer = nullptr;
        GEng::VulkanIndexBuffer *mIndexBuffer = nullptr;

        double mLastTime = 0.0;
        double mCounter = 0.0;

        struct PushConstants {
            alignas(16) glm::mat4 mProjectionMatrix;
            alignas(16) glm::vec3 mLightPos;
        } mPushConstants;
    };
} // Example
