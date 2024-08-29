#pragma once

#include <unordered_map>

#include"GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Platform/GPUContext.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include<unordered_map>

namespace GEng {
    using GLid = unsigned int;

    GENG_API class OpenGLContext : public GPUContext {
    public:
        typedef void (*OpenGLErrorCallback)(int errorCode, const char *message);

        OpenGLContext(OpenGLErrorCallback errorCallback);

        virtual API GetContextAPI() const override { return API::OpenGL; }

        void ClearBindings();
        void ResetBindings();
        void UpdateBindings();
        void DeleteBinding(GLenum target);
        void RegisterBind(GLenum target, GLuint id);
        //Returns whether the OpenGL object was already bound to the context
        bool BindToContext(GLenum target, GLuint id);
        GLuint GetBinding(GLenum target);

        void Reset() override;

        void Clear(const glm::vec4 &clearColour) override;

        std::string GetVersionStr() override;
        std::string GetVendorStr() override;
        std::string GetShadingVersionStr() override;

        void SetFaceCulling(bool activate) override;
        void SetDepthTest(bool activate) override;
        void SetPolygonMode(const PolygonMode mode) override;
        void SetBlending(bool activate, const Blending mode) override;

        void SetViewport(const glm::ivec4 &bounds) override;

        void ActivateDefaultFramebuffer() override;

    private:
        std::unordered_map<GLenum, GLuint> bindings;
    };
}
