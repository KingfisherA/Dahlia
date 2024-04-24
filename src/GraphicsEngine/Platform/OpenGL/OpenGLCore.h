#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GEng
{
	using GLid = unsigned int;
	class OpenGLCore
	{
	public:
		enum class Blending
		{
			Alpha,
			Additive
		};

		static GLenum Initilise(void (*ErrorCallback)(int errorCode, const char *message));
		static void SetBlending(const Blending blend);
	};
}
