#include "Application.h"

#include <filesystem>
#include <ImGui/imgui.h>
#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Events/KeyEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/Platform/Windows/Console.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/3D/Texture.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"
#include "GraphicsEngine/3D/Framebuffer.h"

#include "GraphicsEngine/Utilities/IO.h"
#include "GraphicsEngine/Utilities/PathUtils.h"
#include "GraphicsEngine/3D/Framebuffer.h"
#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/3D/ShaderProgram.h"

// #define IMSPINNER_DEMO
// #include "GUI/ImSpinner.h"

#pragma region Entry Point

int main(int argc, char **argv) {
    Example::Application app;
    if (!app.Init()) {
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

    Application::Application() : GraphicsApp("Example Graphics Engine Application", 800, 600, FramebufferSpecification{
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
        GEng::GraphicsApp::OnViewportResize();
        UpdateProjectionMatrix();
    }

    bool Application::OnInit() {
        this->mRenderViewport = true;
        this->mViewportName = "Viewport";

        std::cout << "Example Application\n";

        GPUContext *ctx = GPUContext::Get();
        ctx->SetDepthTest(true);

        mCam.MoveTo(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
        UpdateProjectionMatrix();

        mVAO = VertexArray::Create(Geometry::CreateZSurface({5.0f,5.0f, 0.5f}, {-3.0f, -3.0f}, {3.0f, 3.0f}, 500,
                                                            [](float x, float y)->float {
                                                                return (float)cos(x*x*x+y*y*y);
                                                            })); //Geometry::CreateQuadsphere(1.0f, 40));

        const char *shaderFilePath = "../../../../Assets/Shaders/ColourDiffuse.glsl";
        mShader = GEng::ShaderProgram::CreateFromFile(shaderFilePath);
        mShader->UploadMatF("uTransformMatrix", glm::mat4(1.0f));

#ifdef _WIN32
        const char* fontPath = "C:/Windows/Fonts/Ubuntu-L.ttf";
#else
        const char *fontPath = "/usr/share/fonts/truetype/ubuntu/UbuntuSans[wdth,wght].ttf";
#endif
        mUbuntuLight = mFontManager.CreateFontAtlas(fontPath,
                                                    80.0f);

        mText = Text::Create();
        //         constexpr const char *TEST_TXT =
        //                 R"(Seen you eyes son show. Far two unaffected one alteration apartments celebrated but middletons interested.
        // Described deficient applauded consisted my me do. Passed edward two talent effect seemed engage six. On ye great do child sorry lived. Proceed cottage far letters ashamed get clothes day. Stairs regret at if matter to. On as needed almost at basket remain. By improved sensible servants children striking in surprise.
        //
        // By in no ecstatic wondered disposal my speaking. Direct wholly valley or uneasy it at really. Sir wish like said dull and need make.
        // Sportsman one bed departure rapturous situation disposing his. Off say yet ample ten ought hence. Depending in newspaper an september do existence strangers.
        // Total great saw water had mirth happy new. Projecting pianoforte no of partiality is on. Nay besides joy society him totally six.
        //
        // Repulsive questions contented him few extensive supported. Of remarkably thoroughly he appearance in. Supposing tolerably applauded or of be.
        // Suffering unfeeling so objection agreeable allowance me of. Ask within entire season sex common far who family. As be valley warmth assure on.
        // Park girl they rich hour new well way you. Face ye be me been room we sons fond.
        //
        // Written enquire painful ye to offices forming it. Then so does over sent dull on. Likewise offended humoured mrs fat trifling answered.
        // On ye position greatest so desirous. So wound stood guest weeks no terms up ought. By so these am so rapid blush songs begin. Nor but mean time one over.
        //
        // Was certainty remaining engrossed applauded sir how discovery. Settled opinion how enjoyed greater joy adapted too shy. Now properly surprise expenses interest nor replying she she.
        // Bore tall nay many many time yet less. Doubtful for answered one fat indulged margaret sir shutters together. Ladies so in wholly around whence in at. Warmth he up giving oppose if.
        // Impossible is dissimilar entreaties oh on terminated. Earnest studied article country ten respect showing had. But required offering him elegance son improved informed. )";
        mText->SetScaling(m_Window->GetContentScaling().x * 0.3f);
        mText->AddText("Example Graphics Engine Application", {{-1.0f, -0.95f, 2.0f}, TextLayoutInfo::Alignment::Left});
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

    bool Application::OnMouseMove(GEng::MouseMoveEvent &e) {
        GraphicsApp::OnMouseMove(e);

        if (mIsViewportFocused)
            mCam.UpdateMouse((float) mDeltaMousePos.x,
                             (float) mDeltaMousePos.y,
                             m_MouseButtonDown,
                             m_KeyMods);

        return true;
    }

    bool Application::OnMouseScroll(GEng::MouseScrollEvent &e) {
        GraphicsApp::OnMouseScroll(e);

        if (mIsViewportFocused)
            mCam.UpdateZoom((float) mDeltaScroll);

        return true;
    }

    void Application::OnRender() {
        mCam.UpdateCamera(m_DeltaTime, m_KeyDown);
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

            for (Layer *layer: m_LayerStack)
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
            ImGui::Text("Renderer API: %s", m_VersionStr.c_str());
            ImGui::Text("Vendor: %s", m_VendorStr.c_str());
            ImGui::Text("Shading Language Version: %s", m_ShadingLanguageStr.c_str());
        }

        if (ImGui::CollapsingHeader("Frame Information", ImGuiTreeNodeFlags_DefaultOpen)) {
            const ImGuiIO &myIO = ImGui::GetIO();
            ImGui::Text("Frame rate: %.0f fps", myIO.Framerate);
            ImGui::Text("Frame time: %.1f ms", myIO.DeltaTime * 1000.0f);
        }

        ImGui::End();
    }
}
