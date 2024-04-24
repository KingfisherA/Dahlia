
#include "GraphicsEngine/3D/Framebuffer.h"
#include "GraphicsEngine/Platform/API Core.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.h"
//
//

namespace GEng
{
	sRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification &spec)
	{
		switch (PROGRAM_API)
		{
		case API::OpenGL:
			return (sRef<Framebuffer>)make<OpenGLFramebuffer>(spec);
			break;
		default:
			return nullptr;
		}
	}
}
