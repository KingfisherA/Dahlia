#include <algorithm>
#include <cmath>

#include "Dahlia/Platform/OpenGL/OpenGLTexture.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"

namespace DH
{
    static uint32_t FullMipCount(size_t largestDimension)
    {
        return (uint32_t)std::floor(std::log2((double)std::max<size_t>(largestDimension, 1))) + 1;
    }

    OpenGLTextureBase::~OpenGLTextureBase()
    {
        glDeleteTextures(1, &mTexID);
        mTexID = 0;
    }

    const OpenGLTextureBase& AsOpenGLTexture(const Texture& texture)
    {
        if (texture.GetType() == TextureType::Texture3D)
            return static_cast<const OpenGLTexture3D&>(texture);
        return static_cast<const OpenGLTexture2D&>(texture);
    }

    // Texture uploads bind to GL_TEXTURE0. The OpenGL command list rebinds textures on every
    // draw, so this does not need to be restored.
    OpenGLTexture2D::OpenGLTexture2D(const Dimension2D& dimensions,
                                     const byte* textureData,
                                     Format format,
                                     TextureUsage usage)
      : OpenGLTextureBase(0, GL_TEXTURE_2D)
      , mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
      , mMipLevels(FullMipCount(std::max(dimensions.x, dimensions.y)))
    {
        const GL::TextureFormat glFormat = GL::ToTextureFormat(format);

        glGenTextures(1, &mTexID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     (GLint)glFormat.internalFormat,
                     (GLsizei)dimensions.x,
                     (GLsizei)dimensions.y,
                     0,
                     glFormat.pixelFormat,
                     glFormat.pixelType,
                     textureData);

        // Render targets get every level up front so any level can be attached to a framebuffer.
        // Sampled textures only have the base level until GenerateMipMaps is called.
        GLint maxLevel = 0;
        if (usage & TextureUsage::RenderTarget)
        {
            for (uint32_t level = 1; level < mMipLevels; level++)
            {
                glTexImage2D(GL_TEXTURE_2D,
                             (GLint)level,
                             (GLint)glFormat.internalFormat,
                             (GLsizei)std::max<size_t>(dimensions.x >> level, 1),
                             (GLsizei)std::max<size_t>(dimensions.y >> level, 1),
                             0,
                             glFormat.pixelFormat,
                             glFormat.pixelType,
                             nullptr);
            }
            maxLevel = (GLint)mMipLevels - 1;
        }

        // Defaults for when no sampler object is bound
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLevel);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    OpenGLTexture2D::OpenGLTexture2D(GLuint texID,
                                     GLenum target,
                                     const Dimension2D& dimensions,
                                     Format format)
      : OpenGLTextureBase(texID, target)
      , mDimensions(dimensions)
      , mFormat(format)
      , mUsage(TextureUsage::SampledRenderTarget)
    {
    }

    void OpenGLTexture2D::GenerateMipMaps()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)mMipLevels - 1);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture2D::UpdateData(Dimension2D offset, Dimension2D size, const byte* data)
    {
        const GL::TextureFormat glFormat = GL::ToTextureFormat(mFormat);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        (GLint)offset.x,
                        (GLint)offset.y,
                        (GLsizei)size.x,
                        (GLsizei)size.y,
                        glFormat.pixelFormat,
                        glFormat.pixelType,
                        data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void* OpenGLTexture2D::GetNativeHandle() const
    {
        return GL::NameToHandle(mTexID);
    }

    //
    OpenGLTexture3D::OpenGLTexture3D(const Dimension3D& dimensions,
                                     const byte* textureData,
                                     Format format,
                                     TextureUsage usage)
      : OpenGLTextureBase(0, GL_TEXTURE_3D)
      , mDimensions(dimensions)
      , mFormat(format)
      , mUsage(usage)
    {
        const GL::TextureFormat glFormat = GL::ToTextureFormat(format);

        glGenTextures(1, &mTexID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, mTexID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D,
                     0,
                     (GLint)glFormat.internalFormat,
                     (GLsizei)dimensions.x,
                     (GLsizei)dimensions.y,
                     (GLsizei)dimensions.z,
                     0,
                     glFormat.pixelFormat,
                     glFormat.pixelType,
                     textureData);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

        glBindTexture(GL_TEXTURE_3D, 0);
        CheckGLErrors("OpenGLTexture3D::OpenGLTexture3D");
    }

    void OpenGLTexture3D::UpdateData(Dimension3D offset, Dimension3D size, const byte* data)
    {
        const GL::TextureFormat glFormat = GL::ToTextureFormat(mFormat);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, mTexID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage3D(GL_TEXTURE_3D,
                        0,
                        (GLint)offset.x,
                        (GLint)offset.y,
                        (GLint)offset.z,
                        (GLsizei)size.x,
                        (GLsizei)size.y,
                        (GLsizei)size.z,
                        glFormat.pixelFormat,
                        glFormat.pixelType,
                        data);
        glBindTexture(GL_TEXTURE_3D, 0);
    }

    void* OpenGLTexture3D::GetNativeHandle() const
    {
        return GL::NameToHandle(mTexID);
    }

    //
    OpenGLSampler::OpenGLSampler(const SamplerDesc& desc)
      : mDesc(desc)
    {
        glGenSamplers(1, &mID);
        glSamplerParameteri(
          mID, GL_TEXTURE_MIN_FILTER, (GLint)GL::ToMinFilter(desc.minFilter, desc.mipmapMode));
        glSamplerParameteri(mID, GL_TEXTURE_MAG_FILTER, (GLint)GL::ToMagFilter(desc.magFilter));
        glSamplerParameteri(mID, GL_TEXTURE_WRAP_S, (GLint)GL::ToAddressMode(desc.addressU));
        glSamplerParameteri(mID, GL_TEXTURE_WRAP_T, (GLint)GL::ToAddressMode(desc.addressV));
        glSamplerParameteri(mID, GL_TEXTURE_WRAP_R, (GLint)GL::ToAddressMode(desc.addressW));

        const GL::BorderColourValue border = GL::ToBorderColour(desc.borderColour);
        glSamplerParameterfv(mID, GL_TEXTURE_BORDER_COLOR, border.rgba);
    }

    OpenGLSampler::~OpenGLSampler()
    {
        glDeleteSamplers(1, &mID);
        mID = 0;
    }
}
