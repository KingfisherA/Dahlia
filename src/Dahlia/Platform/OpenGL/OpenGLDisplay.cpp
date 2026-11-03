#include <GLFW/glfw3.h>
#include <stdexcept>

#include "Dahlia/Platform/OpenGL/OpenGLCommandList.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLDisplay.h"

namespace DH
{
    OpenGLDisplay::OpenGLDisplay(GPUContext* ctx, const DisplayProps& properties)
    {
        if (ctx->GetContextAPI() != GraphicsAPI::OpenGL)
            throw std::runtime_error(
              "Cannot initialise an OpenGL window in a non-OpenGL GPU context");

        auto* glCtx = static_cast<OpenGLContext*>(ctx);

        // macOS only provides OpenGL 3.2+ as a forward compatible core profile
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)glCtx->GetMajorVersion());
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)glCtx->GetMinorVersion());
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);

        CreateDisplay(properties);
        if (mWindow == nullptr)
            throw std::runtime_error("Failed to create an OpenGL " +
                                     std::to_string(glCtx->GetMajorVersion()) + "." +
                                     std::to_string(glCtx->GetMinorVersion()) +
                                     " core profile window");

        glfwMakeContextCurrent(mWindow);
        glCtx->InitOpenGLFunctionBindings();

        mCommandList = umake<OpenGLCommandList>(*this);

        GLFWDisplayBase::Show();
    }

    OpenGLDisplay::~OpenGLDisplay() = default;

    CommandList* OpenGLDisplay::BeginFrame()
    {
        const Dimension2D size = GetFramebufferSize();
        if (size.x == 0 || size.y == 0)
            return nullptr; // Minimised

        return mCommandList.get();
    }

    void OpenGLDisplay::EndFrame()
    {
        glfwSwapBuffers(mWindow);
    }

    RenderTargetLayout OpenGLDisplay::GetRenderTargetLayout() const
    {
        return { { Format::RGBA8 }, Format::D24S8, 1 };
    }
}
