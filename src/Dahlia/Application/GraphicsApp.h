#pragma once

#include <string>
#include <imgui.h>
#include <glm/glm.hpp>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Utilities/Memory.h"
#include "Dahlia/Layers/LayerStack.h"
#include "Dahlia/GUI/Display.h"
#include "Dahlia/3D/Framebuffer.h"
#include "Dahlia/Events/KeyEvents.h"
#include "Dahlia/Events/MouseEvents.h"
#include "Dahlia/GUI/ImGuiLayer.h"
#include "Dahlia/GUI/Text.h"
#include "Dahlia/Resources/Resources.h"

namespace DH {
    class Event;
    class Layer;
    class WindowCloseEvent;
    class KeyPressedEvent;
    class KeyReleasedEvent;
    class MouseButtonPressedEvent;
    class MouseButtonReleasedEvent;
    class MouseMoveEvent;
    class MouseScrollEvent;
    class WindowResizeEvent;
    class Framebuffer;
    class ImGuiLayer;

    using size_t = std::size_t;

#define GRAPHICS_APP_INIT_IMGUI_CTX ImGui::SetCurrentContext(m_ImGuiLayer->GetContext().ctx);\
    ImGui::SetAllocatorFunctions(m_ImGuiLayer->GetContext().allocFunc, m_ImGuiLayer->GetContext().freeFunc, m_ImGuiLayer->GetContext().userData);

    class DH_API  GraphicsApp {
    public:
        GraphicsApp(const char *displayTitle,
                    size_t width, size_t height, const FramebufferSpecification &viewportSpec,
                    const Font &font = Res::FontRes::GetDefaultFont());
        virtual ~GraphicsApp() = default;

        bool Init(GraphicsAPI api, uint32_t majorVersion, uint32_t minorVersion);
        void Run();

        void OnEvent(Event &e);

        void PushLayer(Layer *layer);
        void PushOverlay(Layer *overlay);

        Framebuffer &GetViewportFramebuffer() { return *mViewportFramebuffer; }
        [[nodiscard]] const Display &GetDisplay() const { return *mWindow; }
        [[nodiscard]] constexpr double GetDeltaTime() const { return mDeltaTime; }
        [[nodiscard]] constexpr double GetTime() const { return mTime; }

    protected:
        virtual void OnViewportResize() {
        }

        virtual void OnRender() {
        }

        virtual void OnImGuiRender() {
        }

        virtual bool OnInit() { return true; }
        virtual void OnDestroy();

        virtual bool OnWindowResized(WindowResizeEvent &e);
        virtual bool OnWindowClose(WindowCloseEvent &e);
        virtual bool OnKeyPressed(KeyPressedEvent &e);
        virtual bool OnKeyReleased(KeyReleasedEvent &e);
        virtual bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
        virtual bool OnMouseButtonReleased(MouseButtonReleasedEvent &e);
        virtual bool OnMouseMove(MouseMoveEvent &e);
        virtual bool OnMouseScroll(MouseScrollEvent &e);
        bool mIsViewportFocused = false;
        bool mRenderViewport = true;

        void RenderViewport();

        std::string mVersionStr, mVendorStr, mShadingLanguageStr;

        bool mIsRunning = true;
        uRef<Display> mWindow;
        sRef<Framebuffer> mViewportFramebuffer = nullptr;
        ImVec2 mViewPortPanelSize = {0, 0};
        ImVec2 mLastViewportSize = {0, 0};

        ImGuiLayer *m_ImGuiLayer = nullptr;
        LayerStack mLayerStack;

        double mTime = 0.0;
        double mDeltaTime = 0.0;

        glm::vec2 mCurrentMousePos{};
        glm::vec2 mLastMousePos{};
        glm::vec2 mDeltaMousePos{};
        MouseButton mMouseButtonDown = MouseButton::None;
        KeyCode mKeyDown = KeyCode::UNKNOWN;
        int mKeyMods = 0;

        double mDeltaScroll = 0.0;
        double mLastScrollTime = 0.0;

        std::string mViewportName;

        Font mFont;

    private:
        struct InitParams {
            const char *displayTitle = nullptr;
            size_t width = 0, height = 0;
            FramebufferSpecification viewportSpec;
        };

        uRef<InitParams> mInitParams = nullptr;
    };
}
