#pragma once

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Layers/Layer.h"
#include "GraphicsEngine/Events/Events.h"

namespace ImGui
{
	GENG_API void TextCentered(const char *txt);
}

namespace GEng
{
	class Display;

	GENG_API class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const Display *mainDisplay);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event &e) override;

		void RenderDockspace();

		void Begin();
		void End();

	private:
		void SetDarkThemeColors();

		const Display *mDisplay;
	};
}