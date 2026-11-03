#pragma once

#include <cstdint>
#include <string>

#include "Dahlia/Dahlia-API.h"

namespace DH
{
    DH_API enum class GraphicsAPI { OpenGL = 0, Vulkan = 1, Metal = 2 };

    struct DH_API GPULimits
    {
        uint32_t maxTextureSize = 0;
        uint32_t maxColourAttachments = 0;
        uint32_t maxBindGroups = 4;
        uint32_t maxPushConstantSize = 128;

        // True if row 0 of a rendered texture is the bottom of the image (OpenGL). Flip V when
        // displaying render target textures (e.g. with ImGui::Image).
        bool textureOriginBottomLeft = false;
    };

    // The device. Owns API level state (instance/device/queues, or the GL context) but no render
    // state: fixed function state lives in GraphicsPipeline objects and drawing goes through a
    // CommandList obtained from Display::BeginFrame.
    class DH_API GPUContext
    {
      public:
        GPUContext();
        virtual ~GPUContext() = default;

        static GPUContext* Create(const GraphicsAPI api, uint32_t major, uint32_t minor);

        template<typename T = GPUContext>
        static T* Get()
        {
            return static_cast<T*>(sCurrentContext);
        }
        static GraphicsAPI GetAPI() { return sCurrentContext->GetContextAPI(); }

        [[nodiscard]] virtual GraphicsAPI GetContextAPI() const = 0;

        // OpenGL: nullptr, Vulkan: VkDevice, Metal: MTL::Device*
        [[nodiscard]] virtual void* GetNativeDevice() = 0;
        [[nodiscard]] virtual const GPULimits& GetLimits() const = 0;

        // Block until the GPU has finished all submitted work
        virtual void WaitIdle() = 0;

        virtual std::string GetVersionStr() = 0;
        virtual std::string GetVendorStr() = 0;
        virtual std::string GetShadingVersionStr() = 0;

        virtual void DestroyContext() = 0;

      private:
        static GPUContext* sCurrentContext;
    };
}
