#pragma once

#include "Dahlia/GUI/ImGuiRenderer.h"

namespace DH
{
    // imgui_impl_glfw + imgui_impl_metal drawing into the display render pass's encoder
    class MetalImGuiRenderer : public ImGuiRenderer
    {
      public:
        explicit MetalImGuiRenderer(const Display& display);
        ~MetalImGuiRenderer() override;

        void NewFrame(CommandList& cmd) override;
        void Render(ImDrawData* drawData, CommandList& cmd) override;
    };
}
