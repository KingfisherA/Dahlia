#pragma once

#include <algorithm>
#include "glm/vec2.hpp"
#include "glm/matrix.hpp"
#include "GraphicsEngine/GEng_API.h"

namespace GEng {
    GENG_API class Camera2D {
    public:
        Camera2D() = default;

        Camera2D(const Camera2D &cam) : mViewMatrix(cam.mViewMatrix),
                                        mPos(cam.mPos),
                                        mZoom(cam.mZoom) {
        }

        Camera2D &operator=(const Camera2D &cam) {
            mViewMatrix = cam.mViewMatrix;
            mPos = cam.mPos;
            mZoom = cam.mZoom;

            return *this;
        }

        void UpdateMouse(float mouseDX,
                         float mouseDY,
                         int buttonClicked,
                         int modKeys = 0);

        void UpdateZoom(float mouseWheelD);

        void MoveTo(const glm::vec2 &newPos, const float newZoom) {
            mPos = newPos;
            mZoom = newZoom;
        }

        void UpdateCamera(float deltaTime);

        const glm::mat4 &GetViewMatrix() const {
            return mViewMatrix;
        }

        const glm::mat4 &GetProjectionMatrix() const { return mProjectionMatrix; }

        void SetPos(const glm::vec2 &pos) {
            mVel = {0.0f, 0.0f};
            mPos = pos;
        }

        const glm::vec2 &GetPos() const { return mPos; }

        const glm::vec2 &GetVelocity() const { return mVel; }
        const float &GetZoomVelocity() const { return mZoomVel; }

        const void SetZoom(float z) {
            mZoomVel = 0.0f;
            mZoom = std::clamp(z, 0.05f, 100.0f);
        }

        const float &GetZoom() const { return mZoom; }

        void RecalculateViewMatrix();
        void RecalculateOrthographicProjection(const glm::vec2 &screenDimensions);

    private:
        glm::mat4 mProjectionMatrix;
        glm::mat4 mViewMatrix;
        glm::vec2 mPos = {0.0f, 0.0f};
        glm::vec2 mVel = {0, 0};

        float mZoom = 1.0f;
        float mZoomVel = 0.0f;
    };
}
