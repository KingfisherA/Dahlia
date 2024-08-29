
#include "GraphicsEngine/3D/Framebuffer.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLFramebuffer.h"
//
//

namespace GEng
{
	sRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification &spec)
	{
		switch (GPUContext::Get()->GetContextAPI())
		{
		case API::OpenGL:
			return (sRef<Framebuffer>)make<OpenGLFramebuffer>(spec);
			break;
		default:
			return nullptr;
		}
	}
}
