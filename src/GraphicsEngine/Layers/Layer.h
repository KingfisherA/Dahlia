#pragma once

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Events/Events.h"
#include "GraphicsEngine/Utilities/Memory.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLCore.h"

namespace GEng
{
	class Framebuffer;
	class Camera3D;

	GENG_API class Layer
	{
	public:
		Layer(const std::string_view name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(const sRef<GEng::Framebuffer> &screenBuffer, const Camera3D &cam,
							  const glm::mat4 &projectionMatrix, const glm::vec2 &nearFarPlanes)
		{
		}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event &e) {}

		void SetRenderStyle(GLenum renderMode) { mRenderStyle = renderMode; }
		GLenum GetRenderStyle() const { return mRenderStyle; }

		virtual void Resize(unsigned int width, unsigned int height) {};

		inline const std::string &GetName() const { return m_Name; }

	protected:
		GLenum mRenderStyle = GL_FILL;
		std::string m_Name;
	};
}
