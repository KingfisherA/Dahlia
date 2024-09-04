//
// Created by charlie on 29/08/24.
//
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "Vulkan/VulkanGPUContext.h"

namespace GEng {
    GPUContext *GPUContext::sCurrentContext = nullptr;

    GPUContext *GPUContext::Create(const API api, uint32_t major, uint32_t minor) {
        if (sCurrentContext != nullptr)
            throw std::runtime_error("Cannot create two instances of a GPU context");

        switch (api) {
            case API::Vulkan: {
                sCurrentContext = reinterpret_cast<GPUContext *>(new VulkanGPUContext());
                return sCurrentContext;
            }

            default:
            case API::OpenGL: {
                sCurrentContext = reinterpret_cast<GPUContext *>(new OpenGLContext(major, minor));
                return sCurrentContext;
            }
        }
    }
}
