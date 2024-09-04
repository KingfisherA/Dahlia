#pragma once

#include <GLFW/glfw3.h>
#include "GraphicsEngine/GUI/Display.h"

#include<string>

namespace GEng {
    class GLFWDisplayBase : public Display {
    public:
        ~GLFWDisplayBase() override;

        void Show() const override { glfwShowWindow(mWindow); }
        void Hide() const override { glfwHideWindow(mWindow); }
        void Focus() const override { glfwFocusWindow(mWindow); }

        void Refresh() override;
        [[nodiscard]] size_t GetWidth() const override { return mData.mWidth; }
        [[nodiscard]] size_t GetHeight() const override { return mData.mHeight; }
        [[nodiscard]] glm::vec2 GetContentScaling() const override;

        void BindEventCallback(const EventCallbackFnc &callback) override {
            mData.mEventCallback = callback;
        }

        [[nodiscard]] bool CloseRequested() const override { return glfwWindowShouldClose(mWindow); }
        [[nodiscard]] double GetTime() const override { return glfwGetTime(); }

        [[nodiscard]] void *GetAPIDisplay() const override { return (void *) mWindow; };

    protected:
        GLFWDisplayBase() = default;

        void CreateDisplay(const DisplayProps &props);

        GLFWwindow *mWindow = nullptr;

        struct GLFWData {
        public:
            size_t mWidth = 0, mHeight = 0;
            std::string mTitle;

            EventCallbackFnc mEventCallback;
        } mData;
    };
} // GEng
