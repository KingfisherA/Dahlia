
#include "GraphicsEngine/GUI/Display.h"
#include "GraphicsEngine/Platform/GLFW/GLFWDisplay.h"
#include "GraphicsEngine/Utilities/Memory.h"
//
//

namespace GEng
{
	//
	Display *Display::Create(const DisplayProps &properties)
	{
		return new GLFWDisplay(properties);
	}
	//
}