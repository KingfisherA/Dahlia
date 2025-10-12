#pragma once

#include "Dahlia/GUI/Display.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"

namespace CA
{
    class MetalLayer;
}

namespace DH
{
    class GPUContext;
    class DisplayProps;

    class MetalDisplay : public GLFWDisplayBase
    {
      public:
        MetalDisplay(GPUContext* ctx, const DisplayProps& props);

        CA::MetalLayer* GetMetalLayer() { return mtlLayer; }

      private:
        CA::MetalLayer* mtlLayer = nullptr;
    };
}