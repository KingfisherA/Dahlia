#include "VulkanApp.h"
#include "GraphicsEngine/Events/Events.h"


namespace Example {
    using namespace GEng;
    bool VulkanApp::Init() {
        GPUContext *ctx = GEng::GPUContext::Create(API::Vulkan, 1, 0);
        mWindow.reset(GEng::Display::Create(ctx, {"Example Graphics App (Vulkan)", 1920, 1080}));
        mWindow->BindEventCallback(std::bind(&VulkanApp::OnEvent, this, std::placeholders::_1));

        return true;
    }
    //
    void VulkanApp::Run() {
        while (!mWindow->CloseRequested()) {
            mWindow->Refresh();
        }

        OnDestroy();
    }
    //
    void VulkanApp::OnEvent(Event &e) {
    }
    //
    void VulkanApp::OnDestroy() {
        mWindow.reset();
        GEng::GPUContext::Get()->DestroyContext();
    }
} // Example
