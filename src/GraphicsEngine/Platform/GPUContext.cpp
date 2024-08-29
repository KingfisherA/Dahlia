//
// Created by charlie on 29/08/24.
//
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"

namespace GEng {
    GPUContext *GPUContext::sCurrentContext = nullptr;

    GPUContext* GPUContext::Create(const API api) {
        switch (api) {
            default:
            case API::OpenGL: {
                static OpenGLContext context([](int err, const char *mes) {
                    GENG_FATAL(OpenGLCore; GLEW API Initilisation Error,
                               std::string("Unable to intilise GLEW. Error Message: ") + mes);
                });

                sCurrentContext = reinterpret_cast<GPUContext *>(&context);
                return sCurrentContext;
            }
        }
    }
}
