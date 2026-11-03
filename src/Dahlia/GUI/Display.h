#pragma once

#include <glm/vec2.hpp>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Events/Events.h"
#include "Dahlia/Graphics/GraphicsTypes.h"
#include "Dahlia/Platform/GPUContext.h"

namespace DH {
    class CommandList;

    struct DH_API DisplayProps {
        std::string title;
        size_t width = 0, height = 0;
    };

    class DH_API  Display {
    public:
        using EventCallbackFnc = std::function<void(Event &)>;

        virtual ~Display() = default;

        // Polls window events
        virtual void Refresh() = 0;

        // Acquires the next back buffer and returns the command list for this frame. Returns
        // nullptr if no frame can be rendered (e.g. the swapchain is being recreated or the window
        // is minimised); skip rendering and call Refresh as usual in that case.
        [[nodiscard]] virtual CommandList *BeginFrame() = 0;
        // Submits the frame's commands and presents the back buffer
        virtual void EndFrame() = 0;

        // Attachment formats of the back buffer, for pipelines that render to the display
        [[nodiscard]] virtual RenderTargetLayout GetRenderTargetLayout() const = 0;
        // Size of the back buffer in pixels (differs from GetWidth/GetHeight on high DPI screens)
        [[nodiscard]] virtual Dimension2D GetFramebufferSize() const = 0;

        [[nodiscard]] virtual size_t GetWidth() const = 0;
        [[nodiscard]] virtual size_t GetHeight() const = 0;
        virtual void SetSize(size_t width, size_t height) =0;
        [[nodiscard]] virtual glm::vec2 GetContentScaling() const =0;

        virtual void Show() const = 0;
        virtual void Hide() const = 0;
        virtual void Focus() const = 0;

        [[nodiscard]] virtual bool CloseRequested() const = 0;
        [[nodiscard]] virtual double GetTime() const = 0;

        virtual void BindEventCallback(const EventCallbackFnc &callback) = 0;

        [[nodiscard]] virtual void *GetAPIDisplay() const = 0;

        static Display *Create(GPUContext *ctx, const DisplayProps &properties);
    };
}
