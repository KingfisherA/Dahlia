#pragma once

#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/GUI/Display.h"

namespace Example {
    class VulkanApp {
    public:
        VulkanApp() = default;

        bool Init();

        void Run();



    private:
        void OnDestroy();
        void OnEvent(GEng::Event& e);

        uRef<GEng::Display> mWindow;
    };
} // Example
