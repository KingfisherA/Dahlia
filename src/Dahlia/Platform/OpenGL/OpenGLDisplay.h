#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Platform/GLFW/GLFWDisplayBase.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    class GPUContext;
    class OpenGLCommandList;

    class DH_API OpenGLDisplay : public GLFWDisplayBase
    {
      public:
        OpenGLDisplay(GPUContext* ctx, const DisplayProps& properties);
        ~OpenGLDisplay() override;

        [[nodiscard]] CommandList* BeginFrame() override;
        void EndFrame() override;

        [[nodiscard]] RenderTargetLayout GetRenderTargetLayout() const override;

      private:
        uRef<OpenGLCommandList> mCommandList;
    };
}
