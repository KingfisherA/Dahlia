#include <GLFW/glfw3.h>

#include "Dahlia/Events/Events.h"
#include "Dahlia/Events/KeyEvents.h"
#include "Dahlia/Events/MouseEvents.h"
#include "Dahlia/Events/WindowEvents.h"

#include "GLFWDisplayBase.h"

namespace DH {
    //
    void GLFWDisplayBase::Show() const { glfwShowWindow(mWindow); }
    //
    void GLFWDisplayBase::Hide() const { glfwHideWindow(mWindow); }
    //
    void GLFWDisplayBase::Focus() const { glfwFocusWindow(mWindow); }
    //
    bool GLFWDisplayBase::CloseRequested() const { return glfwWindowShouldClose(mWindow); }
    //
    double GLFWDisplayBase::GetTime() const { return glfwGetTime(); }
    //
    void GLFWDisplayBase::CreateDisplay(const DisplayProps &props) {
        mWidth = props.width;
        mHeight = props.height;
        mTitle = props.title;

        mWindow = glfwCreateWindow((int) props.width, (int) props.height,
                                   props.title.c_str(),
                                   nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, (void *) this);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(mWindow, OnWindowResize);
        glfwSetWindowCloseCallback(mWindow, OnWindowClose);
        glfwSetKeyCallback(mWindow, OnKeyPressed);
        glfwSetCharCallback(mWindow, OnCharPressed);
        glfwSetMouseButtonCallback(mWindow, OnMousePressed);
        glfwSetScrollCallback(mWindow, OnScroll);
        glfwSetCursorPosCallback(mWindow, OnCursorPos);
    }

    //
    void GLFWDisplayBase::OnWindowResize(GLFWwindow *window, int width, int height) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);
        display.mWidth = width;
        display.mHeight = height;

        WindowResizeEvent resizeEvt(width, height);
        display.mEventCallback(resizeEvt);
    }

    //
    void GLFWDisplayBase::OnWindowClose(GLFWwindow *window) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);

        WindowCloseEvent closeEvent;
        display.mEventCallback(closeEvent);
    }

    //
    void GLFWDisplayBase::OnKeyPressed(GLFWwindow *window, int key, int scancode, int action, int mods) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);
        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(static_cast<KeyCode>(key), mods, 0);
                display.mEventCallback(event);
                break;
            }

            case GLFW_RELEASE: {
                KeyReleasedEvent event(static_cast<KeyCode>(key), mods);
                display.mEventCallback(event);
                break;
            }

            case GLFW_REPEAT: {
                KeyPressedEvent event(static_cast<KeyCode>(key), mods, 1);
                display.mEventCallback(event);
                break;
            }
        }
    }

    //
    void GLFWDisplayBase::OnCharPressed(GLFWwindow *window, unsigned int keyCode) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);
        KeyTypedEvent event(keyCode);
        display.mEventCallback(event);
    }

    //
    void GLFWDisplayBase::OnMousePressed(GLFWwindow *window, int button, int action, int mods) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);

        auto GLFWToMouseButton = [](int button) {
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    return MouseButton::Left;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    return MouseButton::Centre;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    return MouseButton::Right;
                default:
                    return MouseButton::None;
            }
        };

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(GLFWToMouseButton(button));
                display.mEventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(GLFWToMouseButton(button));
                display.mEventCallback(event);
                break;
            }
        }
    }

    //
    void GLFWDisplayBase::OnScroll(GLFWwindow *window, double xoffset, double yoffset) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);

        MouseScrollEvent event((float) xoffset, (float) yoffset);
        display.mEventCallback(event);
    }

    //
    void GLFWDisplayBase::OnCursorPos(GLFWwindow *window, double xPos, double yPos) {
        GLFWDisplayBase &display = *(GLFWDisplayBase *) glfwGetWindowUserPointer(window);

        MouseMoveEvent event({xPos, yPos});
        display.mEventCallback(event);
    }

    //
    GLFWDisplayBase::~GLFWDisplayBase() {
        glfwDestroyWindow(mWindow);
    }

    //
    void GLFWDisplayBase::Refresh() {
        glfwPollEvents();
    }

    //
    void GLFWDisplayBase::SetSize(size_t width, size_t height) {
        glfwSetWindowSize(mWindow, (int)width, (int)height);
    }

    //
    glm::vec2 GLFWDisplayBase::GetContentScaling() const {
        glm::vec2 scaling;
        glfwGetWindowContentScale(mWindow, &scaling.x, &scaling.y);
        return scaling;
    }

    //
} // GEng
