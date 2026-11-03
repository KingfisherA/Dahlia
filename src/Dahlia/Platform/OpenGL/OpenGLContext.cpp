#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Dahlia/Graphics/GraphicsPipeline.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    OpenGLContext::OpenGLContext(uint32_t major, uint32_t minor)
      : mMajorVersion(major)
      , mMinorVersion(minor)
    {
        // Vulkan/Metal render targets have their origin at the top left; OpenGL's is bottom left
        mLimits.textureOriginBottomLeft = true;
    }

    void OpenGLContext::InitOpenGLFunctionBindings()
    {
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            DH_FATAL(DH::OpenGLContext, (const char*)glewGetErrorString(glewError));
            return;
        }
        // glewInit can raise a spurious GL_INVALID_ENUM on core profiles
        while (glGetError() != GL_NO_ERROR)
        {
        }

        GLint value = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
        mLimits.maxTextureSize = (uint32_t)value;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
        mLimits.maxColourAttachments = (uint32_t)value;
        mLimits.maxBindGroups = MAX_BIND_GROUPS;
        mLimits.maxPushConstantSize = MAX_PUSH_CONSTANT_SIZE;

        glGenBuffers(1, &mPushConstantBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mPushConstantBuffer);
        glBufferData(GL_COPY_WRITE_BUFFER, MAX_PUSH_CONSTANT_SIZE, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        CheckGLErrors("OpenGLContext::InitOpenGLFunctionBindings");
        DH_LOG(Green, OpenGL GLEW API, "GLEW Initialised OK!");
    }

    void OpenGLContext::WaitIdle()
    {
        glFinish();
    }

    std::string OpenGLContext::GetVersionStr()
    {
        return { (const char*)glGetString(GL_VERSION) };
    }

    std::string OpenGLContext::GetVendorStr()
    {
        return { (const char*)glGetString(GL_VENDOR) };
    }

    std::string OpenGLContext::GetShadingVersionStr()
    {
        return { (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) };
    }

    void OpenGLContext::DestroyContext()
    {
        // The window (and with it the GL context) may already be destroyed, which frees every GL
        // object; only delete explicitly while a context is still current
        if (mPushConstantBuffer != 0 && glfwGetCurrentContext() != nullptr)
            glDeleteBuffers(1, &mPushConstantBuffer);
        mPushConstantBuffer = 0;

        glfwTerminate();
        delete this;
    }

    void CheckGLErrors(const char* where)
    {
#ifdef CONFIG_DEBUG
        for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
        {
            const char* name = [error]()
            {
                switch (error)
                {
                    case GL_INVALID_ENUM:
                        return "GL_INVALID_ENUM";
                    case GL_INVALID_VALUE:
                        return "GL_INVALID_VALUE";
                    case GL_INVALID_OPERATION:
                        return "GL_INVALID_OPERATION";
                    case GL_INVALID_FRAMEBUFFER_OPERATION:
                        return "GL_INVALID_FRAMEBUFFER_OPERATION";
                    case GL_OUT_OF_MEMORY:
                        return "GL_OUT_OF_MEMORY";
                    default:
                        return "Unknown GL error";
                }
            }();
            DH_FATAL(OpenGL Error, name << " in " << where);
        }
#else
        (void)where;
#endif
    }
}
