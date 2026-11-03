#pragma once

#include <cstdint>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Platform/GPUContext.h"

namespace DH
{
    using GLenum = unsigned int;
    using GLuint = unsigned int;
    using GLint = int;

    // Targets OpenGL 4.1 core, the highest version available on macOS. Do not use DSA (4.5),
    // layout(binding) (4.2), glTexStorage (4.2), vertex attrib binding (4.3), compute/SSBOs (4.3)
    // or KHR_debug (4.3) anywhere in the OpenGL backend.
    class DH_API OpenGLContext : public GPUContext
    {
      public:
        OpenGLContext(uint32_t major, uint32_t minor);
        OpenGLContext(const OpenGLContext&) = delete;
        OpenGLContext(OpenGLContext&&) noexcept = delete;

        // Called by OpenGLDisplay once the GL context is current
        void InitOpenGLFunctionBindings();

        [[nodiscard]] GraphicsAPI GetContextAPI() const override { return GraphicsAPI::OpenGL; }
        [[nodiscard]] void* GetNativeDevice() override { return nullptr; }
        [[nodiscard]] const GPULimits& GetLimits() const override { return mLimits; }

        void WaitIdle() override;

        std::string GetVersionStr() override;
        std::string GetVendorStr() override;
        std::string GetShadingVersionStr() override;

        void DestroyContext() override;

        [[nodiscard]] uint32_t GetMajorVersion() const { return mMajorVersion; }
        [[nodiscard]] uint32_t GetMinorVersion() const { return mMinorVersion; }

        // Uniform buffer that emulates Vulkan push constants (MAX_PUSH_CONSTANT_SIZE bytes)
        [[nodiscard]] GLuint GetPushConstantBuffer() const { return mPushConstantBuffer; }

      private:
        uint32_t mMajorVersion, mMinorVersion;
        GPULimits mLimits;
        GLuint mPushConstantBuffer = 0;
    };

    // Drains glGetError and reports each error with `where`. Compiled out in release builds.
    void CheckGLErrors(const char* where);
}
