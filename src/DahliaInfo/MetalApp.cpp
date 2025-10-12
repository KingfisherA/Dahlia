// MetalApp.cpp

#include "MetalApp.h"
#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/Metal/MetalDisplay.h"
#include <unistd.h>

namespace DH::InfoApplication
{
    //
    bool MetalApp::Init()
    {
        GPUContext* ctx = GPUContext::Create(GraphicsAPI::Metal, 1, 1);

        mDisplay = new MetalDisplay(ctx, DisplayProps{ "Metal App", 800, 600 });

        mDisplay->Show();

        return true;
    }
    //
    void MetalApp::Run()
    {
        while (!mDisplay->CloseRequested())
        {
            mDisplay->Refresh();
        }
    }
    //
    void MetalApp::Destroy()
    {
        delete mDisplay;
        GPUContext::Get()->DestroyContext();
    }
}