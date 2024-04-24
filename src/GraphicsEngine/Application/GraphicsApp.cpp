#include "GraphicsEngine/Application/GraphicsApp.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLCore.h"
#include "GraphicsEngine/Platform/API Core.h"

#include "GraphicsEngine/3D/Framebuffer.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"

#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Events/WindowEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/Events/KeyEvents.h"
//
//
namespace GEng
{

#define BIND_EVENT_FNC(x) std::bind(&GraphicsApp::x, this, std::placeholders::_1)

    GraphicsApp::GraphicsApp(const char *displayTitle, size_t width, size_t height) : m_MouseLastPos(0.0f)
    {
        m_Window.reset(Display::Create({displayTitle, width, height}));
        m_Window->BindEventCallback(BIND_EVENT_FNC(OnEvent));

        m_VersionStr = (const char *)glGetString(GL_VERSION);
        m_VersionStr.insert(0, "OpenGL ");
        m_VendorStr = (const char *)glGetString(GL_VENDOR);
        m_ShadingLanguageStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        m_ShadingLanguageStr.insert(0, "GLSL ");

        APICore::Initilise();

        m_ImGuiLayer = new ImGuiLayer(m_Window.get());
        PushOverlay(m_ImGuiLayer);

        mViewportFramebuffer = Framebuffer::Create(FramebufferSpecification{800, 600, 1, {FramebufferAttachment(PixelFormat::RGBA8), FramebufferAttachment(PixelFormat::DEPTH24_STENCIL8)}});
        mViewportFramebuffer->Bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mViewportFramebuffer->Unbind();
    }

    int GraphicsApp::Run()
    {
        while (m_IsRunning)
        {
            {
                double newTime = m_Window->GetTime();
                m_DeltaTime = newTime - m_Time;
                m_Time = newTime;
            }

            if (m_viewPortPanelSize.x != this->m_lastViewportSize.x ||
                m_viewPortPanelSize.y != this->m_lastViewportSize.y)
            {
                mViewportFramebuffer->Resize((size_t)m_viewPortPanelSize.x, (size_t)m_viewPortPanelSize.y);

                for (Layer *layer : m_LayerStack)
                    layer->Resize((size_t)m_viewPortPanelSize.x, (size_t)m_viewPortPanelSize.y);

                OnViewportResize();
                m_lastViewportSize = m_viewPortPanelSize;
            }

            OnRender();

            m_ImGuiLayer->Begin();

            for (Layer *layer : m_LayerStack)
                layer->OnImGuiRender();

            if (mRenderViewport)
                this->RenderViewport();
            this->OnRenderImGui();

            m_ImGuiLayer->End();

            m_Window->Refresh();
        }

        return 0;
    }

    void GraphicsApp::RenderViewport()
    {
        // ImGui::ShowDemoWindow(nullptr);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        if (ImGui::Begin("Viewport", nullptr))
        {
            mIsViewportFocused = ImGui::IsWindowFocused();
            m_viewPortPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image(mViewportFramebuffer->QueryTextureAttachment()->GetRendererID(), ImVec2{(float)mViewportFramebuffer->GetWidth(),
                                                                                                 (float)mViewportFramebuffer->GetHeight()});
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void GraphicsApp::PushLayer(Layer *layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void GraphicsApp::PushOverlay(Layer *overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }

    void GraphicsApp::OnEvent(Event &e)
    {
        EventDispatcher dispatcher(e);

        auto DispatchApplicationEvents = [this, &dispatcher]()
        {
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
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.IsHandled())
                break;
            (*it)->OnEvent(e);
        }
    }

    bool GraphicsApp::OnWindowResized(WindowResizeEvent &e)
    {
        glViewport(0, 0, (GLsizei)e.GetWidth(), (GLsizei)e.GetHeight());

        return true;
    }

    bool GraphicsApp::OnWindowClose(WindowCloseEvent &e)
    {
        m_IsRunning = false;
        return true;
    }

    bool GraphicsApp::OnKeyPressed(KeyPressedEvent &e)
    {
        m_KeyDown = e.GetKeyCode();
        m_KeyMods = e.GetModifiers();
        return false;
    }

    bool GraphicsApp::OnKeyReleased(KeyReleasedEvent &e)
    {
        m_KeyDown = -1;
        m_KeyMods = 0;
        return false;
    }

    bool GraphicsApp::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        m_MouseButtonDown = (int)e.GetMouseButton();
        return true;
    }

    bool GraphicsApp::OnMouseButtonReleased(MouseButtonReleasedEvent &e)
    {
        m_MouseButtonDown = -1;
        return true;
    }

    bool GraphicsApp::OnMouseMove(MouseMoveEvent &e)
    {
        const glm::dvec2 &mousePos = e.GetMousePos();
        glm::dvec2 deltaMouse(mousePos.x - m_MouseLastPos.x, mousePos.y - m_MouseLastPos.y);

        m_MouseLastPos = mousePos;
        return true;
    }

    bool GraphicsApp::OnMouseScroll(MouseScrollEvent &e)
    {
        double currentScrollTime = m_Window->GetTime();
        double deltaScroll = glm::max(1.0 / (currentScrollTime - mLastScrollTime), 1.0) * glm::sign(e.GetYOffset());
        mLastScrollTime = currentScrollTime;

        return false;
    }
}