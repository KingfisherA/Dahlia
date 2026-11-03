#include <stdexcept>

#include "Dahlia/Graphics/Buffer.h"
#include "Dahlia/Platform/GPUContext.h"
#include "Dahlia/Platform/OpenGL/OpenGLBuffer.h"
#ifdef DH_VULKAN_ENABLED
#include "Dahlia/Platform/Vulkan/VulkanBuffer.h"
#endif
#ifdef DH_METAL_ENABLED
#include "Dahlia/Platform/Metal/MetalBuffer.h"
#endif

namespace DH
{
    sRef<Buffer> Buffer::Create(const BufferDesc& desc, const void* initialData)
    {
        switch (GPUContext::GetAPI())
        {
            case GraphicsAPI::OpenGL:
                return make<OpenGLBuffer>(desc, initialData);
#ifdef DH_VULKAN_ENABLED
            case GraphicsAPI::Vulkan:
                return make<VulkanBuffer>(desc, initialData);
#endif
#ifdef DH_METAL_ENABLED
            case GraphicsAPI::Metal:
                return make<MetalBuffer>(desc, initialData);
#endif
            default:
                throw std::runtime_error("Buffer is not implemented for this graphics API yet");
        }
    }
}
