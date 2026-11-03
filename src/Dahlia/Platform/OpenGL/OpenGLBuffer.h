#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Buffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    class DH_API OpenGLBuffer : public Buffer
    {
      public:
        OpenGLBuffer(const BufferDesc& desc, const void* initialData);
        OpenGLBuffer(const OpenGLBuffer&) = delete;
        ~OpenGLBuffer() override;

        void Update(const void* data, size_t size, size_t offset = 0) override;
        void Resize(size_t newSize, const void* data = nullptr) override;

        [[nodiscard]] size_t GetSize() const override { return mDesc.size; }
        [[nodiscard]] const BufferDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] void* GetNativeHandle() const override;

        [[nodiscard]] GLuint GetID() const { return mID; }

      private:
        BufferDesc mDesc;
        GLuint mID = 0;
    };
}
