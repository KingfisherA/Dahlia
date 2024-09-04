#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLDisplay.h"

//
//
namespace GEng {
    OpenGLDisplay::OpenGLDisplay(GPUContext *ctx, const DisplayProps &properties) {
        if (ctx->GetContextAPI() != API::OpenGL)
            throw std::runtime_error("Cannot initialise an OpenGL window in a non-OpenGL GPU context");

        OpenGLContext *glCtx = reinterpret_cast<OpenGLContext *>(ctx);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glCtx->GetMajorVersion());
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glCtx->GetMinorVersion());

        CreateDisplay(properties);
        glfwMakeContextCurrent(mWindow);
        glCtx->InitOpenGLFunctionBindings();

        GLFWDisplayBase::Show();
    }
    //
    void OpenGLDisplay::Refresh() {
        GLFWDisplayBase::Refresh();
        glfwSwapBuffers(mWindow);
    }
    //
}
