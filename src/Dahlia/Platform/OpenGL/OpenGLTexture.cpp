#include "Dahlia/Platform/OpenGL/OpenGLTexture.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"

namespace DH
{
    // Texture uploads bind to GL_TEXTURE0. The OpenGL command list rebinds textures on every
    // draw, so this does not need to be restored.
    OpenGLTexture2D::OpenGLTexture2D(const Dimension2D& dimensions,
                                     const byte* textureData,
                                     Format format)
      : mDimensions(dimensions)
      , mFormat(format)
      , mTarget(GL_TEXTURE_2D)
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

        // Defaults for when no sampler object is bound. Only the base level exists until
        // GenerateMipMaps is called.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    OpenGLTexture2D::OpenGLTexture2D(GLuint texID,
                                     GLenum target,
                                     const Dimension2D& dimensions,
                                     Format format)
      : mDimensions(dimensions)
      , mFormat(format)
      , mTexID(texID)
      , mTarget(target)
    {
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &mTexID);
        mTexID = 0;
    }

    void OpenGLTexture2D::GenerateMipMaps()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
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
    OpenGLSampler::OpenGLSampler(const SamplerDesc& desc)
      : mDesc(desc)
    {
        glGenSamplers(1, &mID);
        glSamplerParameteri(
          mID, GL_TEXTURE_MIN_FILTER, (GLint)GL::ToMinFilter(desc.minFilter, desc.mipmapMode));
        glSamplerParameteri(mID, GL_TEXTURE_MAG_FILTER, (GLint)GL::ToMagFilter(desc.magFilter));
        glSamplerParameteri(mID, GL_TEXTURE_WRAP_S, (GLint)GL::ToAddressMode(desc.addressU));
        glSamplerParameteri(mID, GL_TEXTURE_WRAP_T, (GLint)GL::ToAddressMode(desc.addressV));
    }

    OpenGLSampler::~OpenGLSampler()
    {
        glDeleteSamplers(1, &mID);
        mID = 0;
    }
}
