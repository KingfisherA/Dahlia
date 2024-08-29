#pragma once

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/GUI/Display.h"

#include <GLFW/glfw3.h>

namespace GEng
{
	 constexpr size_t OPENGL_MAJOR_VERSION = 4;
	 constexpr size_t OPENGL_MINOR_VERSION = 6;

	GENG_API class GLFWDisplay : public Display
	{
	public:
		GLFWDisplay(const DisplayProps &properties);
		virtual ~GLFWDisplay();

		virtual void Show() const { glfwShowWindow(m_WindowID); }
		virtual void Hide() const { glfwHideWindow(m_WindowID); }
		virtual void Focus() const { glfwFocusWindow(m_WindowID); }

		virtual void Refresh() override;
		virtual size_t GetWidth() const override { return m_Data.width; }
		virtual size_t GetHeight() const override { return m_Data.height; }
		virtual glm::vec2 GetContentScaling() const;

		virtual inline void BindEventCallback(const EventCallbackFnc &callback) override { m_Data.EventCallback = callback; }

		virtual bool CloseRequested() const override { return glfwWindowShouldClose(m_WindowID); }
		virtual double GetTime() const override { return glfwGetTime(); }

		virtual void *GetAPIDisplay() const override { return (void *)m_WindowID; };

	private:
		void Init(const DisplayProps &properties,
					   const size_t majorVersion = OPENGL_MAJOR_VERSION,
					   const size_t minorVersion = OPENGL_MINOR_VERSION);
		static void PrintErrorCallback(int errorCode, const char *message);

		GLFWwindow *m_WindowID = nullptr;

		GENG_API struct  GLFWData
		{
			std::string title;
			size_t width = 0, height = 0;

			EventCallbackFnc EventCallback;
		};

		GLFWData m_Data;
	};
}
