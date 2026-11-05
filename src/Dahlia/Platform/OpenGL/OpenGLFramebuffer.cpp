#include <stdexcept>
#include <string>

#include "Dahlia/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLTexture.h"
#include "Dahlia/Platform/OpenGL/OpenGLTypes.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    static const char* FramebufferErrorToStr(GLenum errorCode)
    {
        switch (errorCode)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                return "Default framebuffer is not available.";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                return "An attachment point to the framebuffer is incomplete";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                return "The framebuffer has no attachments. Add at least one attachment.";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                return "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for a colour attachment "
                       "named by GL_DRAW_BUFFERi";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            case GL_FRAMEBUFFER_UNSUPPORTED:
                return "Combination of attachment internal formats is not supported by the "
                       "OpenGL driver.";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                return "Multisampled attachments do not all use the same number of samples.";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            default:
                return "Unknown framebuffer error";
        }
    }

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
      : mSpecification(Prepare(spec))
    {
        CreateFramebuffer();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        DestroyAttachments();
        glDeleteFramebuffers(1, &mFBO);
        mFBO = 0;
    }

    void OpenGLFramebuffer::CreateFramebuffer()
    {
        if (mFBO != 0)
        {
            DestroyAttachments();
            glDeleteFramebuffers(1, &mFBO);
            mFBO = 0;
        }

        const auto width = (GLsizei)mSpecification.width;
        const auto height = (GLsizei)mSpecification.height;
        const auto samples = (GLsizei)mSpecification.samples;
        const bool multisampled = samples > 1;

        GLint previousFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        mHasDepth = false;
        mHasStencil = false;
        GLenum colourIndex = 0;
        std::vector<GLenum> drawBuffers;

        for (size_t i = 0; i < mSpecification.attachments.size(); i++)
        {
            const FramebufferAttachment& attachment = mSpecification.attachments[i];
            const bool isDepth = IsDepthFormat(attachment.format);
            const GL::TextureFormat glFormat = GL::ToTextureFormat(attachment.format);

            if (isDepth && mHasDepth)
                throw std::runtime_error("An OpenGL framebuffer can only have one depth attachment");

            GLenum attachPoint;
            if (isDepth)
            {
                attachPoint = DH::HasStencil(attachment.format) ? GL_DEPTH_STENCIL_ATTACHMENT
                                                            : GL_DEPTH_ATTACHMENT;
                mHasDepth = true;
                mHasStencil = DH::HasStencil(attachment.format);
            }
            else
            {
                attachPoint = GL_COLOR_ATTACHMENT0 + colourIndex++;
                drawBuffers.push_back(attachPoint);
            }

            if (attachment.IsExisting())
            {
                // The specification keeps the texture alive for as long as it is attached
                const OpenGLTextureBase& texture = AsOpenGLTexture(*attachment.texture);
                if (attachment.texture->GetType() == TextureType::Texture3D)
                    glFramebufferTextureLayer(GL_FRAMEBUFFER,
                                              attachPoint,
                                              texture.GetID(),
                                              (GLint)attachment.mipLevel,
                                              (GLint)attachment.layer);
                else
                    glFramebufferTexture2D(GL_FRAMEBUFFER,
                                           attachPoint,
                                           GL_TEXTURE_2D,
                                           texture.GetID(),
                                           (GLint)attachment.mipLevel);
                continue;
            }

            if (attachment.renderbuffer)
            {
                GLuint rbo = 0;
                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                if (multisampled)
                    glRenderbufferStorageMultisample(
                      GL_RENDERBUFFER, samples, glFormat.internalFormat, width, height);
                else
                    glRenderbufferStorage(GL_RENDERBUFFER, glFormat.internalFormat, width, height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER, rbo);
                mRenderbufferAttachments.emplace(i, rbo);
                continue;
            }

            GLuint tex = 0;
            glGenTextures(1, &tex);
            const GLenum target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            glBindTexture(target, tex);
            if (multisampled)
            {
                glTexImage2DMultisample(
                  target, samples, glFormat.internalFormat, width, height, GL_TRUE);
            }
            else
            {
                glTexImage2D(target,
                             0,
                             (GLint)glFormat.internalFormat,
                             width,
                             height,
                             0,
                             glFormat.pixelFormat,
                             glFormat.pixelType,
                             nullptr);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
            }
            glBindTexture(target, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachPoint, target, tex, 0);
            mTextureAttachments.emplace(
              i,
              make<OpenGLTexture2D>(tex,
                                    target,
                                    Dimension2D{ mSpecification.width, mSpecification.height },
                                    attachment.format));
        }

        if (drawBuffers.empty())
            glDrawBuffer(GL_NONE);
        else
            glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)previousFBO);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            DestroyAttachments();
            glDeleteFramebuffers(1, &mFBO);
            mFBO = 0;

            DH_FATAL(DH::OpenGLFramebuffer::CreateFramebuffer,
                     "Framebuffer creation failed: " << FramebufferErrorToStr(status));
        }

        CheckGLErrors("OpenGLFramebuffer::CreateFramebuffer");
    }

    void OpenGLFramebuffer::DestroyAttachments()
    {
        for (auto& [index, rbo] : mRenderbufferAttachments)
            glDeleteRenderbuffers(1, &rbo);

        // Textures are deleted when the last reference (e.g. a BindGroup) is released
        mTextureAttachments.clear();
        mRenderbufferAttachments.clear();
    }

    void OpenGLFramebuffer::Resize(size_t newWidth, size_t newHeight)
    {
        if (HasExistingAttachments())
            throw std::logic_error(
              "Framebuffers with existing attachments cannot be resized; create a new one");
        mSpecification.width = newWidth;
        mSpecification.height = newHeight;

        CreateFramebuffer();
    }

    sRef<Texture2D> OpenGLFramebuffer::GetAttachmentTexture(size_t index) const
    {
        if (auto it = mTextureAttachments.find(index); it != mTextureAttachments.end())
            return it->second;

        return GetExistingTexture2D(index);
    }
}
