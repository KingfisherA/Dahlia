#pragma once

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Texture.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    // GL texture name and bind target shared by every OpenGL texture type
    class DH_API OpenGLTextureBase
    {
      public:
        [[nodiscard]] GLuint GetID() const { return mTexID; }
        [[nodiscard]] GLenum GetTarget() const { return mTarget; }

      protected:
        OpenGLTextureBase(GLuint texID, GLenum target)
          : mTexID(texID)
          , mTarget(target)
        {
        }
        ~OpenGLTextureBase();

        GLuint mTexID = 0;
        GLenum mTarget = 0;
    };

    // The OpenGL part of any texture created by the OpenGL backend
    [[nodiscard]] DH_API const OpenGLTextureBase& AsOpenGLTexture(const Texture& texture);

    class DH_API OpenGLTexture2D : public Texture2D, public OpenGLTextureBase
    {
      public:
        OpenGLTexture2D(const Dimension2D& dimensions,
                        const byte* textureData,
                        Format format,
                        TextureUsage usage = TextureUsage::Sampled);
        // Adopts an existing GL texture name (used for framebuffer attachments). The texture is
        // deleted with this object. `target` is GL_TEXTURE_2D or GL_TEXTURE_2D_MULTISAMPLE.
        OpenGLTexture2D(GLuint texID, GLenum target, const Dimension2D& dimensions, Format format);
        OpenGLTexture2D(const OpenGLTexture2D&) = delete;
        ~OpenGLTexture2D() override = default;

        void GenerateMipMaps() override;
        void UpdateData(Dimension2D offset, Dimension2D size, const byte* data) override;

        [[nodiscard]] const Dimension2D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] uint32_t GetMipLevelCount() const override { return mMipLevels; }
        [[nodiscard]] void* GetNativeHandle() const override;
        [[nodiscard]] uint64_t GetImGuiID() const override { return mTexID; }

      private:
        Dimension2D mDimensions;
        Format mFormat;
        TextureUsage mUsage;
        uint32_t mMipLevels = 1;
    };

    class DH_API OpenGLTexture3D : public Texture3D, public OpenGLTextureBase
    {
      public:
        OpenGLTexture3D(const Dimension3D& dimensions,
                        const byte* textureData,
                        Format format,
                        TextureUsage usage = TextureUsage::Sampled);
        OpenGLTexture3D(const OpenGLTexture3D&) = delete;
        ~OpenGLTexture3D() override = default;

        void UpdateData(Dimension3D offset, Dimension3D size, const byte* data) override;

        [[nodiscard]] const Dimension3D& GetDimensions() const override { return mDimensions; }
        [[nodiscard]] Format GetFormat() const override { return mFormat; }
        [[nodiscard]] TextureUsage GetUsage() const override { return mUsage; }
        [[nodiscard]] void* GetNativeHandle() const override;

      private:
        Dimension3D mDimensions;
        Format mFormat;
        TextureUsage mUsage;
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
