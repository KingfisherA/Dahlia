#include "Application.h"

#include <ImGui/imgui.h>
#include <filesystem>
#include <string_view>
#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Dahlia/Events/KeyEvents.h"
#include "Dahlia/Events/MouseEvents.h"
#include "Dahlia/GUI/Display.h"
#include "Dahlia/GUI/ImGuiLayer.h"
#include "Dahlia/Graphics/BindGroup.h"
#include "Dahlia/Graphics/Buffer.h"
#include "Dahlia/Graphics/CommandList.h"
#include "Dahlia/Graphics/Framebuffer.h"
#include "Dahlia/Graphics/GPUMesh.h"
#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Graphics/ShaderModule.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Utilities/Memory.h"

#include "Dahlia/Maths/Geometry.h"
#include "Dahlia/Utilities/Assert.h"
#include "Dahlia/Utilities/IO.h"
#include "Dahlia/Utilities/PathUtils.h"

#include <glm/gtc/matrix_transform.hpp>

#pragma region Entry Point

int
main(int argc, char** argv)
{
    // Choose the backend with --vulkan or --metal; the default is OpenGL 4.1. Every backend
    // follows OpenGL's conventions, so the application code is identical for all of them.
    DH::GraphicsAPI api = DH::GraphicsAPI::OpenGL;
    uint32_t major = 4, minor = 1;
    for (int i = 1; i < argc; i++)
    {
        const std::string_view arg = argv[i];
        if (arg == "--vulkan")
        {
            api = DH::GraphicsAPI::Vulkan;
            major = 1;
            minor = 3;
        }
        else if (arg == "--metal")
        {
            api = DH::GraphicsAPI::Metal;
        }
        else if (arg == "--opengl")
        {
            api = DH::GraphicsAPI::OpenGL;
        }
    }

    DH::InfoApplication::Application app;
    if (!app.Init(api, major, minor))
    {
        DH_FATAL(main, "Failed to initialise application")
        return 1;
    }
    app.Run();

    return 0;
}

#pragma endregion

namespace DH::InfoApplication
{
    // Matches the push_constant block in Shaders/ColourDiffuse.vert
    struct SurfacePushConstants
    {
        glm::mat4 pvMatrix;
        glm::mat4 transformMatrix;
    };

    // Matches the Material uniform block in Shaders/ColourDiffuse.frag (std140)
    struct MaterialUniforms
    {
        glm::vec4 lightPos{ 10.0f, 10.0f, 10.0f, 0.0f };
        glm::vec4 colour{ 1.0f, 1.0f, 1.0f, 1.0f }; // a = opacity
    };

    // Constants
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.0f;
#define BIND_EVENT_FNC(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application::Application()
      : GraphicsApp("Dahlia Engine",
                    800,
                    600,
                    FramebufferSpecification{ 800,
                                              600,
                                              1,
                                              { FramebufferAttachment(Format::RGBA8),
                                                FramebufferAttachment(Format::D24S8) } })
    {
    }

    Application::~Application()
    {
        mText.reset();
    }

    void Application::UpdateProjectionMatrix()
    {
        projectionMatrix =
          glm::perspective<float>(glm::radians(45.0f),
                                  (float)mViewportFramebuffer->GetSpecification().width /
                                    (float)mViewportFramebuffer->GetSpecification().height,
                                  NEAR_PLANE,
                                  FAR_PLANE);
    }

    void Application::OnImGuiRender()
    {
        RenderCameraPanel();
        RenderSceneInformation();
    }

    void Application::OnViewportResize()
    {
        GraphicsApp::OnViewportResize();
        UpdateProjectionMatrix();
    }

    bool Application::OnInit()
    {
        GRAPHICS_APP_INIT_IMGUI_CTX

        this->mRenderViewport = true;
        this->mViewportName = "Viewport";

        std::cout << "Example Application\n";

        mCam.SetCentrePosition({ 0.0f, 0.0f, 0.0f });
        mCam.SetZoom(10.0f);
        UpdateProjectionMatrix();

        mSurfaceMesh = GPUMesh::Create(Geometry::CreateZSurface(
          { 5.0f, 5.0f, 0.5f },
          { -3.0f, -3.0f },
          { 3.0f, 3.0f },
          500,
          [](float x, float y) -> float
          { return (float)cos(x * x * x + y * y * y); })); // Geometry::CreateQuadsphere(1.0f, 40));

        // Fragment shader material (set 0, binding 0)
        const MaterialUniforms material{};
        mMaterialBuffer =
          Buffer::Create({ sizeof(MaterialUniforms), BufferUsage::Uniform }, &material);
        sRef<BindGroupLayout> materialLayout = BindGroupLayout::Create(
          { { 0, ShaderResourceType::UniformBuffer, ShaderStage::Fragment } });
        mMaterialGroup =
          BindGroup::Create(materialLayout, { BindGroupEntry::UniformBuffer(0, mMaterialBuffer) });

        GraphicsPipelineDesc desc;
        desc.vertexShader =
          ShaderModule::Create(ShaderStage::Vertex, Res::ShaderSources::GetColourDiffuseVert());
        desc.fragmentShader =
          ShaderModule::Create(ShaderStage::Fragment, Res::ShaderSources::GetColourDiffuseFrag());
        // The shader only reads positions (location 0)
        desc.vertexBindings = { mSurfaceMesh->GetVertexBindings().front() };
        desc.vertexAttributes = { mSurfaceMesh->GetVertexAttributes().front() };
        desc.depth = { true, true, CompareOp::Less };
        desc.targetLayout = mViewportFramebuffer->GetLayout();
        desc.bindGroupLayouts = { materialLayout };
        desc.pushConstantSize = sizeof(SurfacePushConstants);

        mSolidPipeline = GraphicsPipeline::Create(desc);
        desc.raster.polygonMode = PolygonMode::Lines;
        mWireframePipeline = GraphicsPipeline::Create(desc);

        mUbuntuLight = mFontManager.CreateFontAtlas(Res::FontRes::GetDefaultFont(), 80.0f);

        mText = Text::Create(mViewportFramebuffer->GetLayout());
        mText->SetScaling(mWindow->GetContentScaling().x * 0.3f);
        mText->AddText("Example Graphics Engine Application",
                       { glm::vec3{ -1.0f, -0.95f, 2.0f }, TextLayoutInfo::Alignment::Left });
        mText->LayoutText(*mUbuntuLight);

        return true;
    }

    void Application::OnDestroy()
    {
        mText.reset();
        mUbuntuLight.reset();
        mMaterialGroup.reset();
        mMaterialBuffer.reset();
        mSolidPipeline.reset();
        mWireframePipeline.reset();
        mSurfaceMesh.reset();

        GraphicsApp::OnDestroy();
    }

    bool Application::OnMouseMove(MouseMoveEvent& e)
    {
        GraphicsApp::OnMouseMove(e);

        if (mIsViewportFocused)
        {
            mDeltaMousePos /= glm::vec2((float)mWindow->GetWidth(), (float)mWindow->GetHeight());
            mCam.UpdateMovement(mDeltaMousePos, mMouseButtonDown, mKeyDown, mKeyMods);
        }

        return true;
    }

    bool Application::OnMouseScroll(MouseScrollEvent& e)
    {
        GraphicsApp::OnMouseScroll(e);

        if (mIsViewportFocused)
            mCam.UpdateZoom((float)mDeltaScroll);

        return true;
    }

    void Application::OnRender(CommandList& cmd)
    {
        mCam.UpdateCamera((float)mDeltaTime);

        RenderPassDesc pass;
        pass.target = mViewportFramebuffer.get();
        pass.clearColour = { glm::vec3(0.2f), 1.0f };
        cmd.BeginRenderPass(pass);

        const SurfacePushConstants pushConstants{ projectionMatrix * mCam.GetViewMatrix(),
                                                  glm::mat4(1.0f) };

        cmd.BindPipeline(mPolygonMode == PolygonMode::Lines ? *mWireframePipeline
                                                            : *mSolidPipeline);
        cmd.BindGroup(0, *mMaterialGroup);
        cmd.PushConstants(pushConstants);
        mSurfaceMesh->Draw(cmd);

        float aspect =
          (float)mViewportFramebuffer->GetWidth() / (float)mViewportFramebuffer->GetHeight();
        mText->DrawText(cmd, glm::ortho(-aspect, aspect, 1.0f, -1.0f, 0.1f, 100.0f));

        cmd.EndRenderPass();
    }

    void Application::RenderCameraPanel()
    {
        if (!ImGui::Begin("Camera"))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Camera Properties", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            static glm::vec3 newPosition{ 0.0f };
            static float newZoom = 0.0f;
            const glm::vec3& centrePos = mCam.GetCentrePosition();
            const glm::vec3& actualPos = mCam.GetActualPosition();

            newPosition = centrePos;
            newZoom = mCam.GetZoom();

            if (ImGui::Button("Reset Camera##cam"))
            {
                newPosition = { 0.0f, 0.0f, 0.0f };
                newZoom = 10.0f;
            }

            ImGui::Text("Camera Centre Position:");
            ImGui::DragFloat3("##cam_centre_pos", &newPosition[0]);
            ImGui::SameLine();
            if (ImGui::Button("Reset##cam_pos"))
                newPosition = { 0.0f, 0.0f, 0.0f };
            if (newPosition != centrePos)
                mCam.SetCentrePosition(newPosition);

            ImGui::Text("Camera Zoom:");
            ImGui::DragFloat("##cam_zoom", &newZoom, 0.5f);
            ImGui::SameLine();
            if (ImGui::Button("Reset##cam_zoom"))
                newZoom = 10.0f;
            if (newZoom != mCam.GetZoom())
                mCam.SetZoom(newZoom);

            ImGui::Text("Camera Actual Position: X=%.2f, Y=%.2f, Z=%.2f",
                        actualPos.x,
                        actualPos.y,
                        actualPos.z);
            ImGui::Text(
              "Pitch=%.2f, Yaw=%.2f, Roll=%.2f", mCam.GetPitch(), mCam.GetYaw(), mCam.GetRoll());
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Render Style", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent();
            const char* items[] = { "Solid", "Wireframe" };
            static int selectedItem = 0;
            ImGui::Combo("Render Style##combo", &selectedItem, items, 2);

            for (Layer* layer : mLayerStack)
                layer->SetRenderStyle((DH::PolygonMode)selectedItem);

            mPolygonMode = (DH::PolygonMode)selectedItem;

            ImGui::Unindent();
        }

        ImGui::End();
    }

    void Application::RenderSceneInformation()
    {
        if (!ImGui::Begin("Scene Information"))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Adaptor Information", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Renderer API: %s", mVersionStr.c_str());
            ImGui::Text("Vendor: %s", mVendorStr.c_str());
            ImGui::Text("Shading Language Version: %s", mShadingLanguageStr.c_str());
        }

        if (ImGui::CollapsingHeader("Frame Information", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const ImGuiIO& myIO = ImGui::GetIO();
            ImGui::Text("Frame rate: %.0f fps", myIO.Framerate);
            ImGui::Text("Frame time: %.1f ms", myIO.DeltaTime * 1000.0f);
        }

        ImGui::End();
    }
}
