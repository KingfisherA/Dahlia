#include "GraphicsEngine/3D/Buffers.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLBuffers.h"
#include "GraphicsEngine/Platform/GPUContext.h"
#include "GraphicsEngine/Utilities/Memory.h"
//
//

namespace GEng {
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
