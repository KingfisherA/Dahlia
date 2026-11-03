#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Texture.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    class DH_API OpenGLTexture2D : public Texture2D
    {
      public:
        OpenGLTexture2D(const Dimension2D& dimensions, const byte* textureData, Format format);
        // Adopts an existing GL texture name (used for framebuffer attachments). The texture is
        // deleted with this object. `target` is GL_TEXTURE_2D or GL_TEXTURE_2D_MULTISAMPLE.
        OpenGLTexture2D(GLuint texID, GLenum target, const Dimension2D& dimensions, Format format);
        OpenGLTexture2D(const OpenGLTexture2D&) = delete;
        ~OpenGLTexture2D() override;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] void* GetNativeHandle() const override;
        [[nodiscard]] uint64_t GetImGuiID() const override { return mTexID; }

        [[nodiscard]] GLuint GetID() const { return mTexID; }
        [[nodiscard]] GLenum GetTarget() const { return mTarget; }

      private:
        Dimension2D mDimensions;
        Format mFormat;
        GLuint mTexID = 0;
        GLenum mTarget;
    };

    class DH_API OpenGLSampler : public Sampler
    {
      public:
        explicit OpenGLSampler(const SamplerDesc& desc);
        OpenGLSampler(const OpenGLSampler&) = delete;
        ~OpenGLSampler() override;

        [[nodiscard]] const SamplerDesc& GetDesc() const override { return mDesc; }
        [[nodiscard]] GLuint GetID() const { return mID; }

      private:
        SamplerDesc mDesc;
        GLuint mID = 0;
    };
}
