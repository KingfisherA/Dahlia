#include "Application.h"

#include <filesystem>
#include <ImGui/imgui.h>
#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "GraphicsEngine/Events/KeyEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/3D/Texture.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"
#include "GraphicsEngine/3D/Framebuffer.h"

#include "GraphicsEngine/Utilities/IO.h"
#include "GraphicsEngine/Utilities/PathUtils.h"
#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/3D/ShaderProgram.h"
#include "GraphicsEngine/Utilities/Assert.h"


#pragma region Entry Point

int main(int argc, char **argv) {
    Example::Application app;
    if (!app.Init(GEng::GraphicsAPI::OpenGL, 4, 6)) {
        GENG_FATAL(main, "Failed to initialise application")
        return 1;
    }
    app.Run();
    return 0;
}

#pragma endregion

namespace Example {
    using namespace GEng;
    // Constants
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.0f;
#define BIND_EVENT_FNC(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application::Application() : GraphicsApp("Example Graphics Engine Application", 800, 600,
                                             FramebufferSpecification{
                                                 800, 600, 1,
                                                 {
                                                     FramebufferAttachment(PixelFormat::RGBA8),
                                                     FramebufferAttachment(PixelFormat::DEPTH24_STENCIL8)
                                                 }
                                             }) {
    }

    Application::~Application() {
        mText.reset();
    }

    void Application::UpdateProjectionMatrix() {
        projectionMatrix = glm::perspective<float>(glm::radians(45.0f),
                                                   (float) mViewportFramebuffer->GetSpecification().Width / (float)
                                                   mViewportFramebuffer->GetSpecification().Height, NEAR_PLANE,
                                                   FAR_PLANE);
    }

    void Application::OnImGuiRender() {
        RenderCameraPanel();
        RenderSceneInformation();
    }

    void Application::OnViewportResize() {
        GraphicsApp::OnViewportResize();
        UpdateProjectionMatrix();
    }

    bool Application::OnInit() {
        GRAPHICS_APP_INIT_IMGUI_CTX

        this->mRenderViewport = true;
        this->mViewportName = "Viewport";

        std::cout << "Example Application\n";

        GPUContext *ctx = GPUContext::Get();
        ctx->SetDepthTest(true);

        mCam.SetCentrePosition({0.0f, 0.0f, 0.0f});
        mCam.SetZoom(10.0f);
        UpdateProjectionMatrix();

        mVAO = VertexArray::Create(Geometry::CreateZSurface({5.0f, 5.0f, 0.5f}, {-3.0f, -3.0f}, {3.0f, 3.0f}, 500,
                                                            [](float x, float y)-> float {
                                                                return (float) cos(x * x * x + y * y * y);
                                                            })); //Geometry::CreateQuadsphere(1.0f, 40));

        mShader = ShaderProgram::Create(Res::ShaderSources::GetColourDiffuseGLSL());
        mShader->UploadMatF("uTransformMatrix", glm::mat4(1.0f));

        mUbuntuLight = mFontManager.CreateFontAtlas(Res::FontRes::GetDefaultFont(),
                                                    80.0f);

        mText = Text::Create();
        mText->SetScaling(mWindow->GetContentScaling().x * 0.3f);
        mText->AddText("Example Graphics Engine Application",
            {glm::vec3{-1.0f, -0.95f, 2.0f}, TextLayoutInfo::Alignment::Left});
        mText->LayoutText(*mUbuntuLight);

        return true;
    }

    void Application::OnDestroy() {
        mVAO.reset();
        mShader.reset();
        mUbuntuLight.reset();
        mText.reset();

        GraphicsApp::OnDestroy();
    }

    bool Application::OnMouseMove(MouseMoveEvent &e) {
        GraphicsApp::OnMouseMove(e);

        if (mIsViewportFocused) {
            mDeltaMousePos /= glm::vec2((float)mWindow->GetWidth(), (float)mWindow->GetHeight());
            mCam.UpdateMovement(mDeltaMousePos,
                                mMouseButtonDown,
                                mKeyDown,
                                mKeyMods);
        }

        return true;
    }

    bool Application::OnMouseScroll(MouseScrollEvent &e) {
        GraphicsApp::OnMouseScroll(e);

        if (mIsViewportFocused)
            mCam.UpdateZoom((float) mDeltaScroll);

        return true;
    }

    void Application::OnRender() {
        mCam.UpdateCamera((float) mDeltaTime);
        mViewportFramebuffer->Bind();

        GPUContext::Get()->SetPolygonMode(mPolygonMode);
        mViewportFramebuffer->Clear({glm::vec3(0.2f), 1.0f});

        glm::mat4 pvMatrix = projectionMatrix * mCam.GetViewMatrix();

        mShader->Bind();
        mShader->UploadMatF("uPVMatrix", pvMatrix);

        mVAO->SubmitArrays(Primitive::Triangles);

        float aspect = (float) mViewportFramebuffer->GetWidth() / (float) mViewportFramebuffer->GetHeight();
        mText->DrawText(glm::ortho(-aspect, aspect, 1.0f, -1.0f, 0.1f, 100.0f));

        mViewportFramebuffer->Unbind();
    }

    void Application::RenderCameraPanel() {
        if (!ImGui::Begin("Camera")) {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Camera Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            static glm::vec3 newPosition{0.0f};
            static float newZoom = 0.0f;
            const glm::vec3 &centrePos = mCam.GetCentrePosition();
            const glm::vec3 &actualPos = mCam.GetActualPosition();

            newPosition = centrePos;
            newZoom = mCam.GetZoom();

            if (ImGui::Button("Reset Camera##cam")) {
                newPosition = {0.0f, 0.0f, 0.0f};
                newZoom = 10.0f;
            }

            ImGui::Text("Camera Centre Position:");
            ImGui::DragFloat3("##cam_centre_pos", &newPosition[0]);
            ImGui::SameLine();
            if (ImGui::Button("Reset##cam_pos"))
                newPosition = {0.0f, 0.0f, 0.0f};
            if (newPosition != centrePos)
                mCam.SetCentrePosition(newPosition);

            ImGui::Text("Camera Zoom:");
            ImGui::DragFloat("##cam_zoom", &newZoom, 0.5f);
            ImGui::SameLine();
            if (ImGui::Button("Reset##cam_zoom"))
                newZoom = 10.0f;
            if (newZoom != mCam.GetZoom())
                mCam.SetZoom(newZoom);

            ImGui::Text("Camera Actual Position: X=%.2f, Y=%.2f, Z=%.2f", actualPos.x, actualPos.y, actualPos.z);
            ImGui::Text("Pitch=%.2f, Yaw=%.2f, Roll=%.2f", mCam.GetPitch(), mCam.GetYaw(), mCam.GetRoll());
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Render Style", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            const char *items[] = {"Solid", "Wireframe"};
            static int selectedItem = 0;
            ImGui::Combo("Render Style##combo", &selectedItem, items, 2);

            for (Layer *layer: mLayerStack)
                layer->SetRenderStyle((GEng::GPUContext::PolygonMode) selectedItem);

            mPolygonMode = (GEng::GPUContext::PolygonMode) selectedItem;

            ImGui::Unindent();
        }

        ImGui::End();
    }

    void Application::RenderSceneInformation() {
        if (!ImGui::Begin("Scene Information")) {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Adaptor Information", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Renderer API: %s", mVersionStr.c_str());
            ImGui::Text("Vendor: %s", mVendorStr.c_str());
            ImGui::Text("Shading Language Version: %s", mShadingLanguageStr.c_str());
        }

        if (ImGui::CollapsingHeader("Frame Information", ImGuiTreeNodeFlags_DefaultOpen)) {
            const ImGuiIO &myIO = ImGui::GetIO();
            ImGui::Text("Frame rate: %.0f fps", myIO.Framerate);
            ImGui::Text("Frame time: %.1f ms", myIO.DeltaTime * 1000.0f);
        }

        ImGui::End();
    }
}
