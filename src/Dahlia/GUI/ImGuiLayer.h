#pragma once

#include <imgui.h>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Layers/Layer.h"
#include "Dahlia/Events/Events.h"
#include "Dahlia/GUI/Text.h"


namespace ImGui {
    DH_API void TextCentered(const char *fmt);
}

namespace DH {
    class CommandList;

    class Display;
    class Font;
    class ImGuiRenderer;

    class DH_API  ImGuiLayer : public Layer {
    public:
        class Context {
        public:
            // Context() {
            //     ctx = ImGui::CreateContext();
            //     ImGui::GetAllocatorFunctions(&allocFunc, &freeFunc, &userData);
            // }
            Context() = default;
            Context(const Context &) = default;
            ImGuiContext *ctx = nullptr;
            ImGuiMemAllocFunc allocFunc = nullptr;
            ImGuiMemFreeFunc freeFunc = nullptr;
            void *userData = nullptr;
        };

        ImGuiLayer(const Display *mainDisplay, const Font &font);
        ~ImGuiLayer() override;

        void OnInit() override;
        void OnDestroy() override;
        void OnImGuiRender() override;

        void OnEvent(Event &e) override;

        void RenderDockspace();


        [[nodiscard]] Context &GetContext() { return mContext; }

        // Begin/End bracket the ImGui frame. Both must be called inside the display render pass;
        // the OpenGL renderer draws directly, Vulkan/Metal record into `cmd`.
        void Begin(CommandList &cmd);
        void End(CommandList &cmd);

    private:
        void SetDarkThemeColors();

        Font mFont;
        Context mContext;

        const Display *mDisplay;
        uRef<ImGuiRenderer> mRenderer;
    };
}
