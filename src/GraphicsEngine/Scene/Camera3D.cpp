#include "GraphicsEngine/Scene/Camera3D.h"
#include <GLFW/glfw3.h>

#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include<glm/gtx/transform.hpp>

#include<iostream>

//
//

namespace GEng {
    //
    //
    Camera3D::Camera3D(float pitchYawAcc, float posAcc, float zoomAcc, float decayFactor) : mViewMatrix(1.0f),
        mPosVel(0.0f), mCentrePos(0.0f), mPitchYawAcc(pitchYawAcc), mPosAcc(posAcc), mZoomAcc(zoomAcc),
        mDecayFactor(decayFactor) {
        RecalculateActualPos();
    }

    //
    void Camera3D::UpdateMovement(const glm::vec2 mouseDPosNormalised, int buttonClicked, int key, int modKeys) {
        if (buttonClicked != -1 && ((modKeys & GLFW_MOD_SHIFT) || key == 340)) {
            mZoomVel += glm::length(mouseDPosNormalised) * glm::sign(mouseDPosNormalised.y) * 2.0f * mZoomAcc;
        } else if (buttonClicked == GLFW_MOUSE_BUTTON_RIGHT) {
            mPosVel += glm::vec3(
                        this->GetInverseViewMatrix() * glm::vec4(-mouseDPosNormalised.x, mouseDPosNormalised.y, 0.0f,
                                                                 0.0f)) *
                    mPosAcc;
        } else if (buttonClicked == GLFW_MOUSE_BUTTON_LEFT) {
            mPitchVel -= mouseDPosNormalised.y * mPitchYawAcc;
            mYawVel += mouseDPosNormalised.x * mPitchYawAcc;
        } else {
            glm::vec3 vecToCam = normalize(mCentrePos - mActualPos);

            glm::vec3 deltaVec(0.0f);

            if (key == GLFW_KEY_W) {
                deltaVec = vecToCam;
            }
            if (key == GLFW_KEY_S) {
                deltaVec = -vecToCam;
            }
            if (key == GLFW_KEY_A) {
                deltaVec = glm::cross(vecToCam, glm::vec3(mViewMatrix[0][1], mViewMatrix[1][1], mViewMatrix[2][1]));
            }
            if (key == GLFW_KEY_D) {
                deltaVec = -glm::cross(
                    vecToCam,
                    glm::vec3(mViewMatrix[0][1], mViewMatrix[1][1], mViewMatrix[2][1])
                    /*The camera's up vector in world space*/);
            }


            mPosVel += deltaVec;
        }
    }

    void Camera3D::UpdateZoom(double mouseWheelD) {
        mZoomVel -= static_cast<float>(mouseWheelD) * mZoomAcc;
    }

    void Camera3D::UpdateCamera(float deltaTime) {
        mCentrePos += mPosVel * deltaTime;
        mPosVel *= mDecayFactor;

        mZoomVel *= mDecayFactor;

        mYaw += mYawVel * deltaTime;
        mPitch += mPitchVel * deltaTime;
        mZoom += mZoomVel * deltaTime;
        mZoom = std::max(mZoom, 0.0f);

        mPitchVel *= mDecayFactor;
        mYawVel *= mDecayFactor;

        RecalculateActualPos();
        RecalculateViewMatrix();
    }

    //
    void Camera3D::RecalculateActualPos() {
        float horiDistanceFromCentre = mZoom * (float) cos(glm::radians(mPitch));
        float vertiDistanceFromCentre = mZoom * (float) sin(glm::radians(mPitch));

        mActualPos = mCentrePos;
        mActualPos.x -= horiDistanceFromCentre * (float) sin(glm::radians(180.0f - mYaw));
        mActualPos.y += vertiDistanceFromCentre;
        mActualPos.z -= horiDistanceFromCentre * (float) cos(glm::radians(180.0f - mYaw));
    }

    void Camera3D::RecalculateViewMatrix() {
        mViewMatrix = glm::rotate(glm::radians(mPitch), glm::vec3(1, 0, 0)) *
                      glm::rotate(glm::radians(mYaw), glm::vec3(0, 1, 0)) * glm::rotate(
                          glm::radians(mRoll), glm::vec3(0, 0, 1)) * glm::translate(-mActualPos);
        mInvViewMatrix = glm::inverse(mViewMatrix);
    }

    //
}
