#include "Camera2D.h"
#include "GLFW/glfw3.h"

#include <algorithm>
#include <iostream>
#include <glm/gtx/transform.hpp>

//
//
namespace GEng {
    constexpr float CAM_ACCELERATION = 0.2f;
    constexpr float ZOOM_ACCELERATION = 0.01f;

    void Camera2D::UpdateMouse(float mouseDX, float mouseDY, int buttonClicked, int modKeys) {
        if (buttonClicked == GLFW_MOUSE_BUTTON_LEFT) {
            mVel -= glm::vec2(mouseDX, mouseDY) * CAM_ACCELERATION;
        }
    }

    void Camera2D::UpdateZoom(float mouseWheelD) {
        mZoomVel += std::clamp(mouseWheelD * ZOOM_ACCELERATION, -4.0f, 4.0f);
    }

    void Camera2D::UpdateCamera(float deltaTime) {
        mPos += mVel * deltaTime;
        mVel *= 0.9f;
        mZoom += mZoomVel * deltaTime;
        mZoom = std::clamp(mZoom, 0.05f, 100.0f);
        mZoomVel *= 0.9f;

        RecalculateViewMatrix();
    }

    void Camera2D::RecalculateViewMatrix() {
        mViewMatrix = glm::translate(-glm::vec3(mPos, 0.0f)) * glm::scale(glm::vec3(mZoom));
    }

    void Camera2D::RecalculateOrthographicProjection(const glm::vec2 &screenDimensions) {
        constexpr float zNear = 0.1f;
        constexpr float zFar = 100.0f;
        float aspect = screenDimensions.x / screenDimensions.y;
        mProjectionMatrix = glm::ortho(-aspect, aspect, 1.0f, -1.0f, 0.0001f, 1000.0f);
    }
}
