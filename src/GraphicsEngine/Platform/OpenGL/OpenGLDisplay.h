#pragma once

#include "../../GEng_API.h"
#include "GraphicsEngine/Platform/GLFW/GLFWDisplayBase.h"

#include <GLFW/glfw3.h>

namespace GEng {
    class GPUContext;

    class GENG_API  OpenGLDisplay : public GLFWDisplayBase {
    public:
        OpenGLDisplay(GPUContext *ctx, const DisplayProps &properties);

        void Refresh() override;
    };
}
