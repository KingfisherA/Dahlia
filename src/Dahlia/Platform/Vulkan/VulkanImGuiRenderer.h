#pragma once

#include "Dahlia/GUI/ImGuiRenderer.h"

namespace DH
{
    // imgui_impl_glfw + imgui_impl_vulkan using dynamic rendering into the display render pass
    class VulkanImGuiRenderer : public ImGuiRenderer
    {
      public:
        explicit VulkanImGuiRenderer(const Display& display);
        ~VulkanImGuiRenderer() override;

        void NewFrame(CommandList& cmd) override;
        void Render(ImDrawData* drawData, CommandList& cmd) override;

        // Whether the ImGui Vulkan backend is initialised (textures registered with it are
        // freed with its descriptor pool when it shuts down)
        static bool IsActive() { return sActive; }

      private:
        static inline bool sActive = false;
    };
}
