#include <GL/glew.h>

#include "GraphicsEngine/Platform/OpenGL/OpenGLContext.h"
#include "GraphicsEngine/Platform/OpenGL/OpenGLTexture.h"
#include "OpenGLShaderProgram.h"

//
//

namespace GEng {
    OpenGLTexture2D::OpenGLTexture2D(const Dimension2D &dimensions,
                                     const byte *textureData, PixelFormat texFormat) : mImageDimensions(dimensions),
        mPixelFormat(texFormat) {
        CreateTexture(dimensions, textureData, texFormat);
    }

    OpenGLTexture2D::OpenGLTexture2D(GLenum texID, const Dimension2D &dimensions,
                                     PixelFormat texFormat) : mTexID(texID),
                                                              mImageDimensions(dimensions),
                                                              mPixelFormat(texFormat) {
    }

    OpenGLTexture2D::OpenGLTexture2D(OpenGLTexture2D &&tex) noexcept : mImageDimensions(tex.mImageDimensions),
                                                              mMipMapsGenerated(tex.mMipMapsGenerated),
                                                              mPixelFormat(tex.mPixelFormat),
                                                              mTexID(tex.mTexID),
                                                              mTexUnit(tex.mTexUnit) {
        tex.mTexID = 0;
    }

    void OpenGLTexture2D::CreateTexture(const Dimension2D &dimensions, const byte *textureData,
                                        PixelFormat texFormat) {
        if (mTexID != 0) {
            glDeleteTextures(1, &mTexID);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &mTexID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, FormatToGL(texFormat), dimensions.x, dimensions.y,
                     0, FormatToGL(texFormat), GL_UNSIGNED_BYTE, (const void *) textureData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        GPUContext::Get<OpenGLContext>()->RegisterBind(GL_TEXTURE0, 0);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        glDeleteTextures(1, &mTexID);
        mTexID = 0;
    }

    void OpenGLTexture2D::Bind(unsigned short unit) {
        if (!GPUContext::Get<OpenGLContext>()->BindToContext(GL_TEXTURE0 + unit, mTexID)) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, mTexID);
            mTexUnit = unit;
        }
    }

    void OpenGLTexture2D::Unbind() {
        glActiveTexture(GL_TEXTURE0 + mTexUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
        GPUContext::Get<OpenGLContext>()->RegisterBind(GL_TEXTURE0 + mTexUnit, 0);
    }

    void OpenGLTexture2D::GenerateMipMaps(const MipmapFiltering filtering) {
        OpenGLTexture2D::Bind(0);
        if (!mMipMapsGenerated)
            glGenerateMipmap(GL_TEXTURE_2D);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            [filtering]() {
                                switch (filtering) {
                                    default:
                                    case MipmapFiltering::Bilinear:
                                        return GL_LINEAR_MIPMAP_NEAREST;
                                    case MipmapFiltering::Trilinear:
                                        return GL_LINEAR_MIPMAP_LINEAR;
                                }
                            }());

        mMipMapsGenerated = true;
    }

    void OpenGLTexture2D::DeleteMipMaps() {
        OpenGLTexture2D::Bind(0);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    void OpenGLTexture2D::UpdateData(const Dimension2D offset, const Dimension2D size, const byte *data) {
        OpenGLTexture2D::Bind(0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, FormatToGL(mPixelFormat),
                        GL_UNSIGNED_BYTE, (const void *) data);
    }
    //
    GLenum OpenGLTexture2D::FormatToGL(PixelFormat format) noexcept {
        switch (format) {
            case PixelFormat::None:
                return GL_NONE;

            case PixelFormat::RGB:
                return GL_RGB;

            case PixelFormat::RGBA8:
                return GL_RGBA;

            case PixelFormat::RG:
                return GL_RG;

            case PixelFormat::R:
            case PixelFormat::RED_INTEGER:
                return GL_RED;

            case PixelFormat::DEPTH24_STENCIL8:
                return GL_DEPTH24_STENCIL8;
        }
    }
}
