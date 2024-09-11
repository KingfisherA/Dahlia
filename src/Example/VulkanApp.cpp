#include "VulkanApp.h"
#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanDisplay.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanBuffer.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanShader.h"
#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/Maths/Geometry.h"

#include <fstream>
#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/norm.hpp>

#include<chrono>

#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/Events/WindowEvents.h"

namespace Example {
    using namespace GEng;
    //
    //
    bool VulkanApp::Init() {
        GPUContext *ctx = GEng::GPUContext::Create(API::Vulkan, 1, 0);
        mWindow = new VulkanDisplay(ctx, {"Example Graphics App (Vulkan)", 1920, 1080});
        mWindow->BindEventCallback(std::bind(&VulkanApp::OnEvent, this, std::placeholders::_1));

        mGraphicsPipeline = new GEng::VulkanGraphicsPipeline();

        sRef<Mesh> sphereMesh = Geometry::CreateQuadsphere(1.0f, 40);


        size_t bufSize = sphereMesh->GetVertexFloatCount() + sphereMesh->GetNormalsFloatCount();
        float *buffer = new float[bufSize];
        for (size_t i = 0; i < sphereMesh->GetVertexFloatCount() / 3; i++) {
            buffer[i * 6 + 0] = sphereMesh->GetVertexArray()[i * 3 + 0];
            buffer[i * 6 + 1] = sphereMesh->GetVertexArray()[i * 3 + 1];
            buffer[i * 6 + 2] = sphereMesh->GetVertexArray()[i * 3 + 2];

            buffer[i * 6 + 3] = sphereMesh->GetNormalsArray()[i * 3 + 0];
            buffer[i * 6 + 4] = sphereMesh->GetNormalsArray()[i * 3 + 1];
            buffer[i * 6 + 5] = sphereMesh->GetNormalsArray()[i * 3 + 2];
        }

        mVertexBuffer = new VulkanVertexBuffer(buffer,
                                               bufSize * sizeof(float),
                                               VulkanBuffer::DataType::Float32);
        mIndexBuffer = new VulkanIndexBuffer(sphereMesh->GetIndiciesArray(),
                                             sphereMesh->GetIndexIntCount() * sizeof(uint32_t),
                                             VulkanBuffer::DataType::UInt32);

        delete[] buffer;

        std::vector<char> vertexShaderSource = ReadBinaryFile("../../../../Assets/Shaders/Vulkan/Diffuse.vert.spv");
        std::vector<char> fragmentShaderSource = ReadBinaryFile("../../../../Assets/Shaders/Vulkan/Diffuse.frag.spv");
        VulkanShader *vertexShader = new VulkanShader(VulkanShader::Type::Vertex, vertexShaderSource.data(),
                                                      vertexShaderSource.size());
        VulkanShader *fragmentShader = new VulkanShader(VulkanShader::Type::Fragment, fragmentShaderSource.data(),
                                                        fragmentShaderSource.size());

        float aspect = 1920.0f / 1080.0f;
        mPushConstants.mProjectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f)
                                           * glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));
        mPushConstants.mLightPos = {-1.0f, -1.0f, 0.7f};

        mGraphicsPipeline->AddVertexShader(vertexShader);
        mGraphicsPipeline->AddFragmentShader(fragmentShader);
        mGraphicsPipeline->SetViewport(1920, 1080);
        mGraphicsPipeline->SetBlending(true, GPUContext::Blending::Alpha);
        mGraphicsPipeline->SetCulling(true);
        mGraphicsPipeline->SetPrimitive(Primitive::Triangles);
        mGraphicsPipeline->SetDepthTesting(true);
        mGraphicsPipeline->SetFrontFace(VulkanGraphicsPipeline::FrontFace::Clockwise);
        mGraphicsPipeline->BindPushConstants(sizeof(PushConstants),
                                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

        ShaderAttribute attributes[] = {
            {0, 0, 3, 0},
            {0, 1, 3, 3}
        };
        mGraphicsPipeline->DefineVertexSpecification(attributes, sizeof(attributes) / sizeof(ShaderAttribute), false);

        mGraphicsPipeline->UpdatePipeline(mWindow->GetRenderPass());

        VulkanDrawingCommands &cmds = mWindow->GetCommandList();

        cmds.AddVertexBuffer(mVertexBuffer);
        cmds.BindIndexBuffer(mIndexBuffer);
        cmds.BindGraphicsPipeline(mGraphicsPipeline);
        UploadPushConstants();
        cmds.Draw();

        delete vertexShader;
        delete fragmentShader;

        return true;
    }

    //
    void VulkanApp::Run() {
        while (!mWindow->CloseRequested()) {
            double time = mWindow->GetTime();
            double deltaTime = time - mLastTime;

            mPushConstants.mLightPos = glm::vec3{cos(mCounter * 0.5), -sin(mCounter * 0.6), sin(mCounter)} * 10.0f;
            UploadPushConstants();

            mWindow->RenderFrame();

            mWindow->Refresh();

            mCounter += 1.0 * deltaTime;
            mLastTime = time;
        }

        OnDestroy();
    }

    //
    void VulkanApp::UploadPushConstants() const {
        VulkanDrawingCommands &cmds = mWindow->GetCommandList();
        cmds.UploadPushConstants((const void *) &mPushConstants,
                                 sizeof(PushConstants), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    //
    void VulkanApp::OnEvent(Event &e) {
        switch (e.GetEventType()) {
            case EventType::WindowResize: {
                WindowResizeEvent &resizeEvent = static_cast<WindowResizeEvent &>(e);

                float aspect = (float) resizeEvent.GetWidth() / (float) resizeEvent.GetHeight();
                mPushConstants.mProjectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f)
                                                   * glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));

                resizeEvent.Handle();
                return;
            }

            default:
                return;
        }
    }

    //
    std::vector<char> VulkanApp::ReadBinaryFile(const char *path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    //
    void VulkanApp::OnDestroy() {
        delete mVertexBuffer;
        delete mIndexBuffer;
        delete mWindow;
        delete mGraphicsPipeline;
        GEng::GPUContext::Get()->DestroyContext();
    }
} // Example
