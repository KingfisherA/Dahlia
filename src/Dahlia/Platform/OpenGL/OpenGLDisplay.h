#pragma once

#include "../../Dahlia-API.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"

#include <GLFW/glfw3.h>

namespace DH {
    class GPUContext;

    class DH_API  OpenGLDisplay : public GLFWDisplayBase {
    public:
        OpenGLDisplay(GPUContext *ctx, const DisplayProps &properties);

        void Refresh() override;
    };
}
