#include "GraphicsEngine/GUI/Display.h"

#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLDisplay.h"
#include "GraphicsEngine/Platform/Vulkan/VulkanDisplay.h"

//
//
namespace GEng {
    //
    Display *Display::Create(GPUContext *ctx, const DisplayProps &properties) {
        switch (ctx->GetContextAPI()) {
            case API::Vulkan:
                return new VulkanDisplay(ctx, properties);

            default:
            case API::OpenGL:
                return new OpenGLDisplay(ctx, properties);
        }
    }
    //
}
