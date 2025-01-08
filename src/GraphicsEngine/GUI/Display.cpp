#include "GraphicsEngine/GUI/Display.h"

#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLDisplay.h"

//
//
namespace GEng {
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
