#include <GL/glew.h>
#include<GLFW/glfw3.h>

#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Utilities/Assert.h"

#include<unordered_set>

namespace GEng {
    static std::unordered_set<GLuint> gPreviousErrorId;

    OpenGLContext::OpenGLContext(uint32_t major, uint32_t minor) : mMajorVersion(major), mMinorVersion(minor) {
        if (glfwInit() != GLFW_TRUE)
            throw std::runtime_error("Failed to initialize GLFW");
    }

    void OpenGLContext::InitOpenGLFunctionBindings() {
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            GENG_FATAL(GEng::OpenGLContext, (const char *) glewGetErrorString(glewError));
            return;
        }

        glEnable(GL_ALPHA);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_DEBUG_OUTPUT);

        if (mMajorVersion == 4 && mMinorVersion >= 3)
            glDebugMessageCallback([](
                               GLenum source,
                               GLenum type,
                               GLuint id,
                               GLenum serverity,
                               GLsizei length,
                               const GLchar *message,
                               const void *userParam) {
                                       if (gPreviousErrorId.find(id) == gPreviousErrorId.end()) {
                                           gPreviousErrorId.insert(id);
                                           if (type == GL_DEBUG_TYPE_ERROR) {
                                               GENG_FATAL(OpenGL Error, message);
                                           } else {
                                               GENG_LOG(Blue, OpenGL Information, message);
                                           }
                                       }
                                   },
                                   nullptr);

        GENG_LOG(Green, OpenGL GLEW API, "GLEW Intilised OK!");
    }

    void OpenGLContext::ClearBindings() {
        bindings.clear();
    }

    void OpenGLContext::ResetBindings() {
        for (auto &itPair: bindings) {
            itPair.second = 0;
        }
    }

    void OpenGLContext::UpdateBindings() {
        for (auto &itPair: bindings) {
            int binding = -1;
            glGetIntegerv(itPair.first, &binding);
            itPair.second = (GLuint) binding;
        }
    }

    void OpenGLContext::DeleteBinding(GLenum target) {
        bindings.erase(target);
    }

    void OpenGLContext::RegisterBind(GLenum target, GLuint id) {
        bindings[target] = id;
    }

    bool OpenGLContext::BindToContext(GLenum target, GLuint id) {
        auto it = bindings.find(target);
        if (it == bindings.end()) {
            bindings.insert({target, id});
            return false;
        }

        if (it->second == id)
            return true;

        it->second = id;

        return false;
    }

    GLuint OpenGLContext::GetBinding(GLenum target) {
        auto it = bindings.find(target);

        if (it != bindings.end())
            return it->second;

        return 0;
    }

    void OpenGLContext::Reset() {
        //ClearBindings();
        ResetBindings();
    }

    void OpenGLContext::Clear(const glm::vec4 &clearColour) {
        glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    std::string OpenGLContext::GetVersionStr() {
        return {(const char *) glGetString(GL_VERSION)};
    }

    std::string OpenGLContext::GetVendorStr() {
        return {(const char *) glGetString(GL_VENDOR)};
    }

    std::string OpenGLContext::GetShadingVersionStr() {
        return {(const char *) glGetString(GL_SHADING_LANGUAGE_VERSION)};
    }

    void OpenGLContext::SetFaceCulling(bool activate) {
        if (activate)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }

    void OpenGLContext::SetDepthTest(bool activate) {
        if (!activate)
            glDisable(GL_DEPTH_TEST);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    void OpenGLContext::SetPolygonMode(const PolygonMode mode) {
        glPolygonMode(GL_FRONT_AND_BACK, [mode]()-> GLenum {
            switch (mode) {
                case PolygonMode::Points:
                    return GL_POINT;

                case PolygonMode::Lines:
                    return GL_LINE;

                default:
                case PolygonMode::Solid:
                    return GL_FILL;
            }
        }());
    }

    void OpenGLContext::SetBlending(bool activate, const Blending mode) {
        if (!activate)
            glDisable(GL_BLEND);

        glEnable(GL_BLEND);

        switch (mode) {
            case Blending::Alpha:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case Blending::Additive:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
        }
    }

    void OpenGLContext::SetViewport(const glm::ivec4 &bounds) {
        glViewport(bounds.x, bounds.y, bounds.z, bounds.w);
    }

    void OpenGLContext::ActivateDefaultFramebuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        OpenGLContext::RegisterBind(GL_FRAMEBUFFER_BINDING, 0);
    }

    void OpenGLContext::DestroyContext() {
        glfwTerminate();
        delete this;
    }
}
