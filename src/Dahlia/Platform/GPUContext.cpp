#include <stdexcept>

#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

//
//
namespace DH
{
    GPUContext* GPUContext::sCurrentContext = nullptr;

    GPUContext::GPUContext()
    {
        if (glfwInit() != GLFW_TRUE)
            throw std::runtime_error("Failed to initialize GLFW");
    }

    GPUContext* GPUContext::Create(const GraphicsAPI api, uint32_t major, uint32_t minor)
    {
        if (sCurrentContext != nullptr)
            throw std::runtime_error("Cannot create two instances of a GPU context");

        switch (api)
        {

            default:
            case GraphicsAPI::OpenGL:
            {
                sCurrentContext = reinterpret_cast<GPUContext*>(new OpenGLContext(major, minor));
                return sCurrentContext;
            }

            case GraphicsAPI::Metal:
            {
                sCurrentContext = reinterpret_cast<GPUContext*>(new MetalGPUContext());
                return sCurrentContext;
            }
        }
    }
}
