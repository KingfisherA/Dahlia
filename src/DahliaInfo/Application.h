#pragma once

#include <ImGui/imgui.h>
#include "Dahlia/Utilities/Memory.h"
#include "Dahlia/GUI/Display.h"
#include "Dahlia/Layers/LayerStack.h"
#include "Dahlia/GUI/ImGuiLayer.h"
#include "Dahlia/Scene/Camera3D.h"
#include "Dahlia/Events/WindowEvents.h"
#include "Dahlia/Application/GraphicsApp.h"
#include "Dahlia/GUI/Text.h"

namespace DH {
    class VertexArray;
    class ShaderProgram;
}

namespace DH::InfoApplication {
    class Application : public DH::GraphicsApp {
    public:
        Application();
        virtual ~Application();

        void UpdateProjectionMatrix();

        inline glm::mat4 &GetProjectionMatrix() { return projectionMatrix; }

        const DH::Camera3D &GetCamera() const { return mCam; }
        DH::Camera3D &GetCamera() { return mCam; }

    protected:
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;
        virtual void OnViewportResize() override;
        virtual bool OnInit() override;
        virtual void OnDestroy() override;

        virtual bool OnMouseMove(DH::MouseMoveEvent &e) override;
        virtual bool OnMouseScroll(DH::MouseScrollEvent &e) override;

    private:
        void RenderCameraPanel();
        void RenderSceneInformation();

        DH::Camera3D mCam;
        glm::mat4 projectionMatrix{};

        DH::GPUContext::PolygonMode mPolygonMode = DH::GPUContext::PolygonMode::Solid;
        sRef<DH::VertexArray> mVAO;
        sRef<DH::ShaderProgram> mShader;

        DH::FontManager mFontManager;
        sRef<DH::FontAtlas> mUbuntuLight;
        sRef<DH::Text> mText;
    };
}
