#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Utilities/Memory.h"

struct ImDrawData;

namespace DH
{
    class CommandList;
    class Display;

    // Connects Dear ImGui to GLFW (platform backend) and the active graphics API (renderer
    // backend). Constructing one initialises both backends; destroying it shuts them down. Must
    // be created after the ImGui context.
    class DH_API ImGuiRenderer
    {
      public:
        virtual ~ImGuiRenderer() = default;

        // Starts the backends' frame. Called inside the display render pass, before
        // ImGui::NewFrame.
        virtual void NewFrame(CommandList& cmd) = 0;
        // Draws the main viewport's draw data into the display render pass
        virtual void Render(ImDrawData* drawData, CommandList& cmd) = 0;
        // Updates and renders secondary OS windows when multi-viewports are enabled
        virtual void RenderPlatformWindows();

        static uRef<ImGuiRenderer> Create(const Display& display);
    };
}
