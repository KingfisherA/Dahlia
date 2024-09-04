#include <GLFW/glfw3.h>

#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Events/KeyEvents.h"
#include "GraphicsEngine/Events/MouseEvents.h"
#include "GraphicsEngine/Events/WindowEvents.h"

#include "GLFWDisplayBase.h"

namespace GEng {
    //
    void GLFWDisplayBase::CreateDisplay(const DisplayProps &props) {
        mData.mWidth = props.width;
        mData.mHeight = props.height;
        mData.mTitle = props.title;

        mWindow = glfwCreateWindow((int) props.width, (int) props.height,
                                   props.title.c_str(),
                                   nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, (void *) &mData);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *window, int width, int height) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);
            data.mWidth = width;
            data.mHeight = height;

            WindowResizeEvent resizeEvt(width, height);
            data.mEventCallback(resizeEvt);
        });
        glfwSetWindowCloseCallback(mWindow, [](GLFWwindow *window) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);

            WindowCloseEvent closeEvent;
            data.mEventCallback(closeEvent);
        });
        glfwSetKeyCallback(mWindow, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, mods, 0);
                    data.mEventCallback(event);
                    break;
                }

                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key, mods);
                    data.mEventCallback(event);
                    break;
                }

                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, mods, 1);
                    data.mEventCallback(event);
                    break;
                }
            }
        });
        glfwSetCharCallback(mWindow, [](GLFWwindow *window, unsigned int keyCode) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);
            KeyTypedEvent event(keyCode);
            data.mEventCallback(event);
        });
        glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *window, int button, int action, int mods) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);

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
                    data.mEventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(GLFWToMouseButton(button));
                    data.mEventCallback(event);
                    break;
                }
            }
        });
        glfwSetScrollCallback(mWindow, [](GLFWwindow *window, double xoffset, double yoffset) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);

            MouseScrollEvent event((float) xoffset, (float) yoffset);
            data.mEventCallback(event);
        });
        glfwSetCursorPosCallback(mWindow, [](GLFWwindow *window, double xPos, double yPos) {
            GLFWData &data = *(GLFWData *) glfwGetWindowUserPointer(window);

            MouseMoveEvent event({xPos, yPos});
            data.mEventCallback(event);
        });
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
    glm::vec2 GLFWDisplayBase::GetContentScaling() const {
        glm::vec2 scaling;
        glfwGetWindowContentScale(mWindow, &scaling.x, &scaling.y);
        return scaling;
    }
    //
} // GEng
