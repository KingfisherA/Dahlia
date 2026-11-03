#pragma once

#include <map>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Graphics/Framebuffer.h"
#include "Dahlia/Platform/OpenGL/OpenGLContext.h"

namespace DH
{
    class OpenGLTexture2D;

    class DH_API OpenGLFramebuffer : public Framebuffer
    {
      public:
        explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
        OpenGLFramebuffer(const OpenGLFramebuffer&) = delete;
        ~OpenGLFramebuffer() override;

        void Resize(size_t newWidth, size_t newHeight) override;

        [[nodiscard]] sRef<Texture2D> GetAttachmentTexture(size_t index = 0) const override;
        [[nodiscard]] const FramebufferSpecification& GetSpecification() const override
        {
            return mSpecification;
        }

        [[nodiscard]] GLuint GetID() const { return mFBO; }
        [[nodiscard]] bool HasDepth() const { return mHasDepth; }
        [[nodiscard]] bool HasStencil() const { return mHasStencil; }

      private:
        void CreateFramebuffer();
        void DestroyAttachments();

        FramebufferSpecification mSpecification;
        GLuint mFBO = 0;
        bool mHasDepth = false, mHasStencil = false;

        std::map<size_t, sRef<OpenGLTexture2D>> mTextureAttachments;
        std::map<size_t, GLuint> mRenderbufferAttachments;
    };
}
