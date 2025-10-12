#include "Dahlia/3D/Buffers.h"
#include "Dahlia/Platform/OpenGL/OpenGLBuffers.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Utilities/Memory.h"
//
//

namespace DH {
    sRef<VertexBuffer> VertexBuffer::Create(BufferUsage usage) {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                return (sRef<VertexBuffer>) make<OpenGLVertexBuffer>(usage);
        }

        return nullptr;
    }

    sRef<VertexBuffer> VertexBuffer::Create(const float *data, size_t floatCount, BufferUsage usage) {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                return (sRef<VertexBuffer>) make<OpenGLVertexBuffer>(usage, data, floatCount);
        }

        return nullptr;
    }

    //
    //
    sRef<IndexBuffer> IndexBuffer::Create(BufferUsage usage) {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                return (sRef<IndexBuffer>) make<OpenGLIndexBuffer>(usage);
        }

        return nullptr;
    }

    sRef<IndexBuffer> IndexBuffer::Create(const unsigned int *data, size_t intCount, BufferUsage usage) {
        switch (GPUContext::Get()->GetContextAPI()) {
            case GraphicsAPI::OpenGL:
                return make<OpenGLIndexBuffer>(usage, data, intCount);
        }
        return nullptr;
    }
}
