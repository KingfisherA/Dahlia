#include <stdexcept>

#include "Dahlia/Platform/OpenGL/OpenGLBuffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"

namespace DH
{
    // Buffers are created and written through GL_COPY_WRITE_BUFFER so that neither the current
    // VAO's element array binding nor GL_ARRAY_BUFFER is disturbed.
    static GLenum ToUsageHint(BufferMemory memory)
    {
        return memory == BufferMemory::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    }

    OpenGLBuffer::OpenGLBuffer(const BufferDesc& desc, const void* initialData)
      : mDesc(desc)
    {
        glGenBuffers(1, &mID);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mID);
        glBufferData(
          GL_COPY_WRITE_BUFFER, (GLsizeiptr)mDesc.size, initialData, ToUsageHint(mDesc.memory));
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    OpenGLBuffer::~OpenGLBuffer()
    {
        glDeleteBuffers(1, &mID);
        mID = 0;
    }

    void OpenGLBuffer::Update(const void* data, size_t size, size_t offset)
    {
        if (offset + size > mDesc.size)
            throw std::out_of_range("OpenGLBuffer::Update writes past the end of the buffer");

        glBindBuffer(GL_COPY_WRITE_BUFFER, mID);
        glBufferSubData(GL_COPY_WRITE_BUFFER, (GLintptr)offset, (GLsizeiptr)size, data);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    void OpenGLBuffer::Resize(size_t newSize, const void* data)
    {
        mDesc.size = newSize;
        glBindBuffer(GL_COPY_WRITE_BUFFER, mID);
        glBufferData(GL_COPY_WRITE_BUFFER, (GLsizeiptr)newSize, data, ToUsageHint(mDesc.memory));
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    void* OpenGLBuffer::GetNativeHandle() const
    {
        return GL::NameToHandle(mID);
    }
}
