#pragma once

#include <ImGui/imgui.h>
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/Layers/LayerStack.h"
#include "GraphicsEngine/GUI/ImGuiLayer.h"
#include "GraphicsEngine/Scene/Camera.hpp"
#include "GraphicsEngine/Events/WindowEvents.h"
#include "GraphicsEngine/Application/GraphicsApp.h"
#include "GraphicsEngine/GUI/Text.h"

namespace GEng {
    class VertexArray;
    class ShaderProgram;
}

namespace Example {
    class Application : public GEng::GraphicsApp {
    public:
        Application();
        virtual ~Application();

        void UpdateProjectionMatrix();

        inline glm::mat4 &GetProjectionMatrix() { return projectionMatrix; }

        const GEng::Camera &GetCamera() const { return mCam; }
        GEng::Camera &GetCamera() { return mCam; }

    protected:
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;
        virtual void OnViewportResize() override;
        virtual bool OnInit() override;

        virtual bool OnMouseMove(GEng::MouseMoveEvent &e) override;
        virtual bool OnMouseScroll(GEng::MouseScrollEvent &e) override;

    private:
        void RenderCameraPanel();
        void RenderSceneInformation();

        GEng::Camera mCam;
        glm::mat4 projectionMatrix;

        GLenum mRenderStyle = GL_FILL;
        sRef<GEng::VertexArray> mVAO;
        sRef<GEng::ShaderProgram> mShader;

        GEng::FontManager mFontManager;
        sRef<GEng::FontAtlas> mUbuntuLight;
        sRef<GEng::Text> mText;
    };
}
