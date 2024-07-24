#pragma once

#include"GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLCore.h"
#include "GraphicsEngine/Utilities/Assert.h"
#include <string>

GENG_API enum class API
{
	OpenGL = 0
};

constexpr API PROGRAM_API = API::OpenGL;

namespace GEng
{
	GENG_API class APICore
	{
	public:
		static void Initilise()
		{
			switch (PROGRAM_API)
			{
			case API::OpenGL:
				OpenGLCore::Initilise([](int err, const char *mes)
									  { UNI_FATAL(OpenGLCore; GLEW API Initilisation Error, std::string("Unable to intilise GLEW. Error Message: ") + mes); });
				break;
			}
		}
	};
}