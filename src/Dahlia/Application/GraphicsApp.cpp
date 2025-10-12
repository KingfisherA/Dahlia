#include "Dahlia/Application/GraphicsApp.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"
#include "Dahlia/Platform/GPUContext.h"

#include "Dahlia/3D/Framebuffer.h"
#include "Dahlia/GUI/Display.h"
#include "Dahlia/GUI/ImGuiLayer.h"
#include "Dahlia/GUI/Text.h"

#include "Dahlia/Events/Events.h"
#include "Dahlia/Events/WindowEvents.h"
#include "Dahlia/Events/MouseEvents.h"
#include "Dahlia/Events/KeyEvents.h"
//
//
namespace DH {
#define BIND_EVENT_FNC(x) std::bind(&GraphicsApp::x, this, std::placeholders::_1)

    GraphicsApp::GraphicsApp(const char *displayTitle, size_t width, size_t height,
                             const FramebufferSpecification &viewportSpec,
                             const Font &font) : mFont(font) {
        mInitParams = std::make_unique<InitParams>();
        mInitParams->displayTitle = displayTitle;
        mInitParams->width = width;
        mInitParams->height = height;
        mInitParams->viewportSpec = viewportSpec;
    }

    bool GraphicsApp::Init(GraphicsAPI api, uint32_t majorVersion, uint32_t minorVersion) {
        if (mInitParams == nullptr)
            return false;

        GPUContext *ctx = GPUContext::Create(api, majorVersion, minorVersion);

        mWindow.reset(Display::Create(ctx, {mInitParams->displayTitle, mInitParams->width, mInitParams->height}));
        mWindow->BindEventCallback(BIND_EVENT_FNC(OnEvent));

        mVersionStr = ctx->GetVersionStr();

        mVendorStr = ctx->GetVendorStr();
        mShadingLanguageStr = ctx->GetShadingVersionStr();

        switch (ctx->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                mVersionStr.insert(0, "OpenGL ");
                mShadingLanguageStr.insert(0, "GLSL ");
                break;
        }

        m_ImGuiLayer = new ImGuiLayer(mWindow.get(), mFont);
        PushOverlay(m_ImGuiLayer);

        mViewportFramebuffer = Framebuffer::Create(mInitParams->viewportSpec);
        mViewportFramebuffer->Clear({0.0f, 0.0f, 0.0f, 1.0f});
        ctx->ActivateDefaultFramebuffer();

        mInitParams.reset();
        return this->OnInit();
    }

    void GraphicsApp::Run() {
        while (mIsRunning) {
            {
                double newTime = mWindow->GetTime();
                mDeltaTime = newTime - mTime;
                mTime = newTime;
            }

            if (mViewPortPanelSize.x != this->mLastViewportSize.x ||
                mViewPortPanelSize.y != this->mLastViewportSize.y) {
                if (mViewPortPanelSize.x > 10 && mViewPortPanelSize.y > 10)
                    mViewportFramebuffer->Resize((size_t) mViewPortPanelSize.x, (size_t) mViewPortPanelSize.y);

                for (Layer *layer: mLayerStack)
                    layer->Resize((size_t) mViewPortPanelSize.x, (size_t) mViewPortPanelSize.y);

                OnViewportResize();
                mLastViewportSize = mViewPortPanelSize;
            }

            GPUContext::Get()->Reset();
            OnRender();
            GPUContext::Get()->ActivateDefaultFramebuffer();

            m_ImGuiLayer->Begin();

            for (Layer *layer: mLayerStack)
                layer->OnImGuiRender();

            if (mRenderViewport)
                this->RenderViewport();
            this->OnImGuiRender();

            m_ImGuiLayer->End();

            mWindow->Refresh();
        }

        this->OnDestroy();
    }

    void GraphicsApp::OnDestroy() {
        for (Layer *layer: mLayerStack) {
            layer->OnDestroy();
        }

        GPUContext::Get()->DestroyContext();
    }

    void GraphicsApp::RenderViewport() {
        // ImGui::ShowDemoWindow(nullptr);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        if (ImGui::Begin(mViewportName.c_str(), nullptr)) {
            mIsViewportFocused = ImGui::IsWindowFocused();
            mViewPortPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID) (intptr_t) mViewportFramebuffer->QueryTextureAttachment(0)->GetRendererID(),
                         ImVec2{(float) mViewportFramebuffer->GetWidth(), (float) mViewportFramebuffer->GetHeight()},
                         {0, 1}, {1, 0});
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void GraphicsApp::PushLayer(Layer *layer) {
        mLayerStack.PushLayer(layer);
    }

    void GraphicsApp::PushOverlay(Layer *overlay) {
        mLayerStack.PushOverlay(overlay);
    }

    void GraphicsApp::OnEvent(Event &e) {
        EventDispatcher dispatcher(e);

        auto DispatchApplicationEvents = [this, &dispatcher]() {
            if (dispatcher.GetEvent().IsHandled())
                return;
            dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FNC(OnWindowClose));
            dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FNC(OnWindowResized));
            dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FNC(OnKeyPressed));
            dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FNC(OnKeyReleased));
            dispatcher.Dispatch<MouseMoveEvent>(BIND_EVENT_FNC(OnMouseMove));
            dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FNC(OnMouseButtonPressed));
            dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FNC(OnMouseButtonReleased));
            dispatcher.Dispatch<MouseScrollEvent>(BIND_EVENT_FNC(OnMouseScroll));
        };

        DispatchApplicationEvents();
        for (auto it = mLayerStack.rbegin(); it != mLayerStack.rend(); ++it) {
            if (e.IsHandled())
                break;
            (*it)->OnEvent(e);
        }
    }

    bool GraphicsApp::OnWindowResized(WindowResizeEvent &e) {
        GPUContext::Get()->SetViewport({0, 0, e.GetWidth(), e.GetHeight()});

        return true;
    }

    bool GraphicsApp::OnWindowClose(WindowCloseEvent &e) {
        mIsRunning = false;
        return true;
    }

    bool GraphicsApp::OnKeyPressed(KeyPressedEvent &e) {
        mKeyDown = e.GetKeyCode();
        mKeyMods = e.GetModifiers();
        return false;
    }

    bool GraphicsApp::OnKeyReleased(KeyReleasedEvent &e) {
        mKeyDown = KeyCode::UNKNOWN;
        mKeyMods = 0;
        return false;
    }

    bool GraphicsApp::OnMouseButtonPressed(MouseButtonPressedEvent &e) {
        mMouseButtonDown = e.GetMouseButton();
        return true;
    }

    bool GraphicsApp::OnMouseButtonReleased(MouseButtonReleasedEvent &e) {
        mMouseButtonDown = MouseButton::None;
        return true;
    }

    bool GraphicsApp::OnMouseMove(MouseMoveEvent &e) {
        mCurrentMousePos = e.GetMousePos();
        mDeltaMousePos = mCurrentMousePos - mLastMousePos;

        mLastMousePos = mCurrentMousePos;
        return true;
    }

    bool GraphicsApp::OnMouseScroll(MouseScrollEvent &e) {
        double currentScrollTime = mWindow->GetTime();
        mDeltaScroll = glm::max(1.0 / (currentScrollTime - mLastScrollTime), 1.0) * glm::sign(e.GetYOffset());
        mLastScrollTime = currentScrollTime;

        return false;
    }
}
