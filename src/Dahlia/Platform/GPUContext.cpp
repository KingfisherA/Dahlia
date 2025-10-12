#include <stdexcept>

#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

//
//
namespace DH {
    GPUContext *GPUContext::sCurrentContext = nullptr;

    GPUContext *GPUContext::Create(const GraphicsAPI api, uint32_t major, uint32_t minor) {
        if (sCurrentContext != nullptr)
            throw std::runtime_error("Cannot create two instances of a GPU context");

        switch (api) {

            default:
            case GraphicsAPI::OpenGL: {
                sCurrentContext = reinterpret_cast<GPUContext *>(new OpenGLContext(major, minor));
                return sCurrentContext;
            }
        }
    }
}
