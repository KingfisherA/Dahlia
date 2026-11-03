#pragma once

#include <cstddef>
#include <cstdint>

#include "Dahlia/Dahlia-API.h"
#include "Dahlia/Utilities/Memory.h"

namespace DH
{
    enum class BufferUsage : uint8_t
    {
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2
    };

    constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) noexcept
    {
        return static_cast<BufferUsage>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr bool operator&(BufferUsage a, BufferUsage b) noexcept
    {
        return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
    }

    enum class BufferMemory : uint8_t
    {
        // Written rarely (typically once). Vulkan places it in device-local memory and uploads via
        // a staging buffer.
        Static = 0,
        // Written often from the CPU (per frame uniforms, streaming vertices). Host visible memory.
        Dynamic
    };

    struct DH_API BufferDesc
    {
        size_t size = 0; // In bytes
        BufferUsage usage = BufferUsage::Vertex;
        BufferMemory memory = BufferMemory::Static;
    };

    class DH_API Buffer
    {
      public:
        virtual ~Buffer() = default;

        // Copy `size` bytes from `data` into the buffer at `offset`. The range must lie inside the
        // buffer; use Resize to grow it.
        virtual void Update(const void* data, size_t size, size_t offset = 0) = 0;

        // Reallocate the buffer. The previous contents are discarded. If data is non-null it is
        // uploaded as the new contents.
        virtual void Resize(size_t newSize, const void* data = nullptr) = 0;

        [[nodiscard]] virtual size_t GetSize() const = 0;
        [[nodiscard]] virtual const BufferDesc& GetDesc() const = 0;

        // OpenGL: buffer name, Vulkan: VkBuffer, Metal: MTL::Buffer*
        [[nodiscard]] virtual void* GetNativeHandle() const = 0;

        static sRef<Buffer> Create(const BufferDesc& desc, const void* initialData = nullptr);
    };
}
