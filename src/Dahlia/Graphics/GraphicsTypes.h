#pragma once

// Backend-neutral vocabulary shared by every graphics object. Everything here must be expressible
// on OpenGL 4.1 core (the macOS ceiling), Vulkan and Metal.

#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Dahlia/Dahlia-API.h"

namespace DH
{
    using Dimension2D = glm::vec<2, size_t>;
    using Dimension3D = glm::vec<3, size_t>;

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

        // Colour, 16 bit float per channel. Prefer this to RGBA32F for filtered render targets:
        // 32 bit float filtering is optional on Vulkan and missing on Apple GPUs before M3.
        // Upload data as IEEE half floats.
        RGBA16F,

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
            case Format::RGBA16F:
                return 4;
            default:
                return 0;
        }
    }

    [[nodiscard]] constexpr bool IsFloatFormat(Format format) noexcept
    {
        return format == Format::R32F || format == Format::RG32F || format == Format::RGB32F ||
               format == Format::RGBA32F || format == Format::RGBA16F;
    }

    // Size in bytes of one element (pixel or vertex attribute) of the format
    [[nodiscard]] constexpr uint32_t FormatByteSize(Format format) noexcept
    {
        if (format == Format::D24S8 || format == Format::D32F)
            return 4;
        if (format == Format::RGBA16F)
            return 8;
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
        None = 0,   // Blending disabled
        Alpha,      // src * srcAlpha + dst * (1 - srcAlpha)
        Additive,   // src * srcAlpha + dst (alpha weighted)
        AdditiveOne // src + dst, for every channel (accumulation; ignores alpha)
    };

    enum class BlendFactor : uint8_t
    {
        Zero = 0,
        One,
        SrcColour,
        OneMinusSrcColour,
        DstColour,
        OneMinusDstColour,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    // Min and Max ignore the blend factors on every backend
    enum class BlendOp : uint8_t
    {
        Add = 0,
        Subtract,        // src - dst
        ReverseSubtract, // dst - src
        Min,
        Max
    };

    // Blending for one colour attachment: result = src * srcFactor <op> dst * dstFactor, with
    // separate factors for the colour (RGB) and alpha channels. Implicitly constructible from a
    // BlendPreset.
    struct DH_API BlendState
    {
        bool enabled = false;
        BlendFactor srcColour = BlendFactor::One;
        BlendFactor dstColour = BlendFactor::Zero;
        BlendOp colourOp = BlendOp::Add;
        BlendFactor srcAlpha = BlendFactor::One;
        BlendFactor dstAlpha = BlendFactor::Zero;
        BlendOp alphaOp = BlendOp::Add;

        constexpr BlendState() = default;

        // Same factors and op for colour and alpha
        constexpr BlendState(BlendFactor src, BlendFactor dst, BlendOp op = BlendOp::Add)
          : enabled(true)
          , srcColour(src)
          , dstColour(dst)
          , colourOp(op)
          , srcAlpha(src)
          , dstAlpha(dst)
          , alphaOp(op)
        {
        }

        constexpr BlendState(BlendFactor srcColour,
                             BlendFactor dstColour,
                             BlendOp colourOp,
                             BlendFactor srcAlpha,
                             BlendFactor dstAlpha,
                             BlendOp alphaOp)
          : enabled(true)
          , srcColour(srcColour)
          , dstColour(dstColour)
          , colourOp(colourOp)
          , srcAlpha(srcAlpha)
          , dstAlpha(dstAlpha)
          , alphaOp(alphaOp)
        {
        }

        constexpr BlendState(BlendPreset preset) // NOLINT(google-explicit-constructor)
        {
            switch (preset)
            {
                case BlendPreset::None:
                    break;
                case BlendPreset::Alpha:
                    *this = BlendState(BlendFactor::SrcAlpha,
                                       BlendFactor::OneMinusSrcAlpha,
                                       BlendOp::Add,
                                       BlendFactor::One,
                                       BlendFactor::OneMinusSrcAlpha,
                                       BlendOp::Add);
                    break;
                case BlendPreset::Additive:
                    *this = BlendState(BlendFactor::SrcAlpha, BlendFactor::One);
                    break;
                case BlendPreset::AdditiveOne:
                    *this = BlendState(BlendFactor::One, BlendFactor::One);
                    break;
            }
        }

        constexpr bool operator==(const BlendState&) const = default;
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
        ClampToEdge,
        ClampToBorder // Samples outside [0, 1] return SamplerDesc::borderColour
    };

    // Limited to the border colours Metal supports
    enum class BorderColour : uint8_t
    {
        TransparentBlack = 0, // (0, 0, 0, 0)
        OpaqueBlack,          // (0, 0, 0, 1)
        OpaqueWhite           // (1, 1, 1, 1)
    };

    enum class TextureType : uint8_t
    {
        Texture2D = 0,
        Texture3D
    };

    // How a texture may be used. Every texture can be sampled; RenderTarget additionally lets it
    // be a framebuffer attachment (FramebufferAttachment with an existing texture).
    enum class TextureUsage : uint8_t
    {
        Sampled = 1 << 0,
        RenderTarget = 1 << 1,
        SampledRenderTarget = Sampled | RenderTarget
    };

    constexpr TextureUsage operator|(TextureUsage a, TextureUsage b) noexcept
    {
        return static_cast<TextureUsage>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr bool operator&(TextureUsage a, TextureUsage b) noexcept
    {
        return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
    }

    // Describes the attachments a pipeline renders into. Vulkan and Metal bake this into the
    // pipeline object; OpenGL ignores it.
    struct DH_API RenderTargetLayout
    {
        std::vector<Format> colourFormats;
        Format depthFormat = Format::None;
        uint32_t samples = 1;
    };
}
