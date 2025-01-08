#pragma once

#include<string>
#include "GraphicsEngine/GUI/Display.h"

struct GLFWwindow;

namespace GEng {
    class GLFWDisplayBase : public Display {
    public:
        ~GLFWDisplayBase() override;

        void Show() const override;
        void Hide() const override;
        void Focus() const override;

        void Refresh() override;
        [[nodiscard]] size_t GetWidth() const override { return mWidth; }
        [[nodiscard]] size_t GetHeight() const override { return mHeight; }
        void SetSize(size_t width, size_t height) override;
        [[nodiscard]] glm::vec2 GetContentScaling() const override;

        void BindEventCallback(const EventCallbackFnc &callback) override {
            mEventCallback = callback;
        }

        [[nodiscard]] bool CloseRequested() const override;
        [[nodiscard]] double GetTime() const override;

        [[nodiscard]] void *GetAPIDisplay() const override { return (void *) mWindow; };

    protected:
        GLFWDisplayBase() = default;

        void CreateDisplay(const DisplayProps &props);

        static void OnWindowResize(GLFWwindow *window, int width, int height);
        static void OnWindowClose(GLFWwindow *window);
        static void OnKeyPressed(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void OnCharPressed(GLFWwindow *window, unsigned int keyCode);
        static void OnMousePressed(GLFWwindow *window, int button, int action, int mods);
        static void OnScroll(GLFWwindow *window, double xoffset, double yoffset);
        static void OnCursorPos(GLFWwindow *window, double xPos, double yPos);

        GLFWwindow *mWindow = nullptr;

        size_t mWidth = 0, mHeight = 0;
        std::string mTitle;

        EventCallbackFnc mEventCallback;
    };
} // GEng
