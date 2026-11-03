#pragma once

// Backend-neutral vocabulary shared by every graphics object. Everything here must be expressible
// on OpenGL 4.1 core (the macOS ceiling), Vulkan and Metal.

#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

#include "Dahlia/Dahlia-API.h"

namespace DH
{
    using Dimension2D = glm::vec<2, size_t>;

    enum class Format : uint8_t
    {
        None = 0,

        // Colour, 8 bits per channel (unsigned normalised)
        R8,
        RG8,
        RGB8, // OpenGL only: Vulkan/Metal have no 3 channel 8 bit formats and must expand to RGBA8
        RGBA8,
        BGRA8,

        // Colour, 32 bit float per channel
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,

        // Depth / stencil
        D24S8,
        D32F
    };

    [[nodiscard]] constexpr bool IsDepthFormat(Format format) noexcept
    {
        return format == Format::D24S8 || format == Format::D32F;
    }

    [[nodiscard]] constexpr bool HasStencil(Format format) noexcept
    {
        return format == Format::D24S8;
    }

    // Number of components in a colour/vertex format
    [[nodiscard]] constexpr uint32_t FormatComponentCount(Format format) noexcept
    {
        switch (format)
        {
            case Format::R8:
            case Format::R32F:
                return 1;
            case Format::RG8:
            case Format::RG32F:
                return 2;
            case Format::RGB8:
            case Format::RGB32F:
                return 3;
            case Format::RGBA8:
            case Format::BGRA8:
            case Format::RGBA32F:
                return 4;
            default:
                return 0;
        }
    }

    [[nodiscard]] constexpr bool IsFloatFormat(Format format) noexcept
    {
        return format == Format::R32F || format == Format::RG32F || format == Format::RGB32F ||
               format == Format::RGBA32F;
    }

    // Size in bytes of one element (pixel or vertex attribute) of the format
    [[nodiscard]] constexpr uint32_t FormatByteSize(Format format) noexcept
    {
        if (format == Format::D24S8 || format == Format::D32F)
            return 4;
        return FormatComponentCount(format) * (IsFloatFormat(format) ? 4u : 1u);
    }

    enum class ShaderStage : uint8_t
    {
        None = 0,
        Vertex = 1 << 0,
        Fragment = 1 << 1,
        // Geometry/compute are intentionally absent: Metal has no geometry stage and OpenGL 4.1 has
        // no compute. Add them behind GPULimits feature flags when needed.
        AllGraphics = Vertex | Fragment
    };

    constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) noexcept
    {
        return static_cast<ShaderStage>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr bool operator&(ShaderStage a, ShaderStage b) noexcept
    {
        return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
    }

    // Fans/quads are omitted because Metal cannot draw them
    enum class PrimitiveTopology : uint8_t
    {
        Triangles = 0,
        TriangleStrip,
        Lines,
        LineStrip,
        Points
    };

    enum class CullMode : uint8_t
    {
        None = 0,
        Front,
        Back
    };

    enum class FrontFace : uint8_t
    {
        CounterClockwise = 0,
        Clockwise
    };

    // Points is not supported by Metal; the Metal backend falls back to Solid
    enum class PolygonMode : uint8_t
    {
        Solid = 0,
        Lines,
        Points
    };

    enum class CompareOp : uint8_t
    {
        Never = 0,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class BlendPreset : uint8_t
    {
        None = 0, // Blending disabled
        Alpha,    // src * srcAlpha + dst * (1 - srcAlpha)
        Additive  // src * srcAlpha + dst
    };

    enum class IndexType : uint8_t
    {
        UInt16 = 0,
        UInt32
    };

    enum class LoadOp : uint8_t
    {
        Load = 0, // Keep the previous contents
        Clear,    // Clear to the render pass clear value
        DontCare  // Contents are undefined (fastest on tiled GPUs)
    };

    enum class Filter : uint8_t
    {
        Nearest = 0,
        Linear
    };

    enum class MipmapMode : uint8_t
    {
        None = 0, // Sample only the base level
        Nearest,  // Bilinear filtering
        Linear    // Trilinear filtering
    };

    enum class AddressMode : uint8_t
    {
        Repeat = 0,
        MirroredRepeat,
        ClampToEdge
    };

    // Describes the attachments a pipeline renders into. Vulkan and Metal bake this into the
    // pipeline object; OpenGL ignores it.
    struct DH_API RenderTargetLayout
    {
        std::vector<Format> colourFormats;
        Format depthFormat = Format::None;
        uint32_t samples = 1;
    };
}
