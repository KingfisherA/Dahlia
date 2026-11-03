#pragma once

#include "Dahlia/GUI/ImGuiRenderer.h"

namespace DH
{
    class OpenGLImGuiRenderer : public ImGuiRenderer
    {
      public:
        explicit OpenGLImGuiRenderer(const Display& display);
        ~OpenGLImGuiRenderer() override;

        void NewFrame(CommandList& cmd) override;
        void Render(ImDrawData* drawData, CommandList& cmd) override;
        void RenderPlatformWindows() override;
    };
}
