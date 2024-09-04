#include "GraphicsEngine/Application/GraphicsApp.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Platform/GPUContext.h"

#include "GraphicsEngine/3D/Framebuffer.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"

#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Events/WindowEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/Events/KeyEvents.h"
//
//
namespace GEng {
#define BIND_EVENT_FNC(x) std::bind(&GraphicsApp::x, this, std::placeholders::_1)

    GraphicsApp::GraphicsApp(const char *displayTitle, size_t width, size_t height,
                             const FramebufferSpecification &viewportSpec) {
        mInitParams = std::make_unique<InitParams>();
        mInitParams->displayTitle = displayTitle;
        mInitParams->width = width;
        mInitParams->height = height;
        mInitParams->viewportSpec = viewportSpec;
    }

    bool GraphicsApp::Init() {
        if (mInitParams == nullptr)
            return false;

        GPUContext *ctx = GPUContext::Create(API::OpenGL, 4, 6);

        m_Window.reset(Display::Create(ctx, {mInitParams->displayTitle, mInitParams->width, mInitParams->height}));
        m_Window->BindEventCallback(BIND_EVENT_FNC(OnEvent));

        m_VersionStr = ctx->GetVersionStr();
        m_VersionStr.insert(0, "OpenGL ");
        m_VendorStr = ctx->GetVendorStr();
        m_ShadingLanguageStr = ctx->GetShadingVersionStr();
        m_ShadingLanguageStr.insert(0, "GLSL ");

        m_ImGuiLayer = new ImGuiLayer(m_Window.get());
        PushOverlay(m_ImGuiLayer);

        mViewportFramebuffer = Framebuffer::Create(mInitParams->viewportSpec);
        mViewportFramebuffer->Clear({0.0f, 0.0f, 0.0f, 1.0f});
        ctx->ActivateDefaultFramebuffer();

        mInitParams.reset();
        return this->OnInit();
    }

    void GraphicsApp::Run() {
        while (m_IsRunning) {
            {
                double newTime = m_Window->GetTime();
                m_DeltaTime = newTime - m_Time;
                m_Time = newTime;
            }

            if (m_viewPortPanelSize.x != this->m_lastViewportSize.x ||
                m_viewPortPanelSize.y != this->m_lastViewportSize.y) {
                if (m_viewPortPanelSize.x > 10 && m_viewPortPanelSize.y > 10)
                    mViewportFramebuffer->Resize((size_t) m_viewPortPanelSize.x, (size_t) m_viewPortPanelSize.y);

                for (Layer *layer: m_LayerStack)
                    layer->Resize((size_t) m_viewPortPanelSize.x, (size_t) m_viewPortPanelSize.y);

                OnViewportResize();
                m_lastViewportSize = m_viewPortPanelSize;
            }

            GPUContext::Get()->Reset();
            OnRender();
            GPUContext::Get()->ActivateDefaultFramebuffer();

            m_ImGuiLayer->Begin();

            for (Layer *layer: m_LayerStack)
                layer->OnImGuiRender();

            if (mRenderViewport)
                this->RenderViewport();
            this->OnImGuiRender();

            m_ImGuiLayer->End();

            m_Window->Refresh();
        }

        this->OnDestroy();
    }

    void GraphicsApp::OnDestroy() {
        for (Layer *layer: m_LayerStack) {
            layer->OnDestroy();
        }

        GPUContext::Get()->DestroyContext();
    }

    void GraphicsApp::RenderViewport() {
        // ImGui::ShowDemoWindow(nullptr);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        if (ImGui::Begin(mViewportName.c_str(), nullptr)) {
            mIsViewportFocused = ImGui::IsWindowFocused();
            m_viewPortPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image((void *) mViewportFramebuffer->QueryTextureAttachment()->GetRendererID(),
                         ImVec2{(float) mViewportFramebuffer->GetWidth(), (float) mViewportFramebuffer->GetHeight()},
                         {0, 1}, {1, 0});
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void GraphicsApp::PushLayer(Layer *layer) {
        m_LayerStack.PushLayer(layer);
    }

    void GraphicsApp::PushOverlay(Layer *overlay) {
        m_LayerStack.PushOverlay(overlay);
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
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
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
        m_IsRunning = false;
        return true;
    }

    bool GraphicsApp::OnKeyPressed(KeyPressedEvent &e) {
        m_KeyDown = e.GetKeyCode();
        m_KeyMods = e.GetModifiers();
        return false;
    }

    bool GraphicsApp::OnKeyReleased(KeyReleasedEvent &e) {
        m_KeyDown = -1;
        m_KeyMods = 0;
        return false;
    }

    bool GraphicsApp::OnMouseButtonPressed(MouseButtonPressedEvent &e) {
        m_MouseButtonDown = (int) e.GetMouseButton();
        return true;
    }

    bool GraphicsApp::OnMouseButtonReleased(MouseButtonReleasedEvent &e) {
        m_MouseButtonDown = -1;
        return true;
    }

    bool GraphicsApp::OnMouseMove(MouseMoveEvent &e) {
        mCurrentMousePos = e.GetMousePos();
        mDeltaMousePos = mCurrentMousePos - mLastMousePos;

        mLastMousePos = mCurrentMousePos;
        return true;
    }

    bool GraphicsApp::OnMouseScroll(MouseScrollEvent &e) {
        double currentScrollTime = m_Window->GetTime();
        mDeltaScroll = glm::max(1.0 / (currentScrollTime - mLastScrollTime), 1.0) * glm::sign(e.GetYOffset());
        mLastScrollTime = currentScrollTime;

        return false;
    }
}
