#include "Dahlia/Utilities/Memory.h"
#include "Dahlia/3D/Framebuffer.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLFramebuffer.h"
//
//

namespace DH
{
	sRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification &spec)
	{
		switch (GPUContext::Get()->GetContextAPI())
		{
		case GraphicsAPI::OpenGL:
			return (sRef<Framebuffer>)make<OpenGLFramebuffer>(spec);
		default:
			return nullptr;
		}
	}
}
