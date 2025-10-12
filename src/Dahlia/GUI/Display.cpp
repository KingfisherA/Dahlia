#include "Dahlia/GUI/Display.h"

#include "Dahlia/Platform/OpenGL/OpenGLContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLDisplay.h"

//
//
namespace DH {
    //
    Display *Display::Create(GPUContext *ctx, const DisplayProps &properties) {
        switch (ctx->GetContextAPI()) {

            default:
            case GraphicsAPI::OpenGL:
                return new OpenGLDisplay(ctx, properties);
        }
    }
    //
}
