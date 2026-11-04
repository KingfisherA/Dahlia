#pragma once

// Conversions from the common graphics vocabulary to OpenGL 4.1 enums. Include only from
// OpenGL backend .cpp files (requires GL/glew.h).

#include <GL/glew.h>

#include "Dahlia/Graphics/GraphicsTypes.h"

namespace DH::GL
{
    struct TextureFormat
    {
        GLenum internalFormat = GL_NONE;
        GLenum pixelFormat = GL_NONE;
        GLenum pixelType = GL_NONE;
    };

    inline TextureFormat ToTextureFormat(Format format) noexcept
    {
        switch (format)
        {
            case Format::R8:
                return { GL_R8, GL_RED, GL_UNSIGNED_BYTE };
            case Format::RG8:
                return { GL_RG8, GL_RG, GL_UNSIGNED_BYTE };
            case Format::RGB8:
                return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
            case Format::RGBA8:
                return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
            case Format::BGRA8:
                return { GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE };
            case Format::R32F:
                return { GL_R32F, GL_RED, GL_FLOAT };
            case Format::RG32F:
                return { GL_RG32F, GL_RG, GL_FLOAT };
            case Format::RGB32F:
                return { GL_RGB32F, GL_RGB, GL_FLOAT };
            case Format::RGBA32F:
                return { GL_RGBA32F, GL_RGBA, GL_FLOAT };
            case Format::RGBA16F:
                return { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT };
            case Format::D24S8:
                return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
            case Format::D32F:
                return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };
            default:
                return {};
        }
    }

    struct VertexFormat
    {
        GLint components = 0;
        GLenum type = GL_NONE;
        GLboolean normalised = GL_FALSE;
    };

    inline VertexFormat ToVertexFormat(Format format) noexcept
    {
        switch (format)
        {
            case Format::R32F:
            case Format::RG32F:
            case Format::RGB32F:
            case Format::RGBA32F:
                return { (GLint)FormatComponentCount(format), GL_FLOAT, GL_FALSE };
            case Format::RGBA16F:
                return { 4, GL_HALF_FLOAT, GL_FALSE };
            case Format::R8:
            case Format::RG8:
            case Format::RGB8:
            case Format::RGBA8:
                return { (GLint)FormatComponentCount(format), GL_UNSIGNED_BYTE, GL_TRUE };
            case Format::BGRA8:
                return { GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE };
            default:
                return {};
        }
    }

    inline GLenum ToTopology(PrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
            default:
            case PrimitiveTopology::Triangles:
                return GL_TRIANGLES;
            case PrimitiveTopology::TriangleStrip:
                return GL_TRIANGLE_STRIP;
            case PrimitiveTopology::Lines:
                return GL_LINES;
            case PrimitiveTopology::LineStrip:
                return GL_LINE_STRIP;
            case PrimitiveTopology::Points:
                return GL_POINTS;
        }
    }

    inline GLenum ToCompareFunc(CompareOp op) noexcept
    {
        switch (op)
        {
            case CompareOp::Never:
                return GL_NEVER;
            default:
            case CompareOp::Less:
                return GL_LESS;
            case CompareOp::Equal:
                return GL_EQUAL;
            case CompareOp::LessOrEqual:
                return GL_LEQUAL;
            case CompareOp::Greater:
                return GL_GREATER;
            case CompareOp::NotEqual:
                return GL_NOTEQUAL;
            case CompareOp::GreaterOrEqual:
                return GL_GEQUAL;
            case CompareOp::Always:
                return GL_ALWAYS;
        }
    }

    inline GLenum ToPolygonMode(PolygonMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case PolygonMode::Solid:
                return GL_FILL;
            case PolygonMode::Lines:
                return GL_LINE;
            case PolygonMode::Points:
                return GL_POINT;
        }
    }

    inline GLenum ToIndexType(IndexType type) noexcept
    {
        return type == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    }

    inline size_t IndexByteSize(IndexType type) noexcept
    {
        return type == IndexType::UInt16 ? 2 : 4;
    }

    inline GLenum ToAddressMode(AddressMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case AddressMode::Repeat:
                return GL_REPEAT;
            case AddressMode::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case AddressMode::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case AddressMode::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
        }
    }

    struct BorderColourValue
    {
        GLfloat rgba[4];
    };

    inline BorderColourValue ToBorderColour(BorderColour colour) noexcept
    {
        switch (colour)
        {
            default:
            case BorderColour::TransparentBlack:
                return { { 0.0f, 0.0f, 0.0f, 0.0f } };
            case BorderColour::OpaqueBlack:
                return { { 0.0f, 0.0f, 0.0f, 1.0f } };
            case BorderColour::OpaqueWhite:
                return { { 1.0f, 1.0f, 1.0f, 1.0f } };
        }
    }

    inline GLenum ToBlendFactor(BlendFactor factor) noexcept
    {
        switch (factor)
        {
            case BlendFactor::Zero:
                return GL_ZERO;
            default:
            case BlendFactor::One:
                return GL_ONE;
            case BlendFactor::SrcColour:
                return GL_SRC_COLOR;
            case BlendFactor::OneMinusSrcColour:
                return GL_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColour:
                return GL_DST_COLOR;
            case BlendFactor::OneMinusDstColour:
                return GL_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:
                return GL_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return GL_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return GL_ONE_MINUS_DST_ALPHA;
        }
    }

    inline GLenum ToBlendOp(BlendOp op) noexcept
    {
        switch (op)
        {
            default:
            case BlendOp::Add:
                return GL_FUNC_ADD;
            case BlendOp::Subtract:
                return GL_FUNC_SUBTRACT;
            case BlendOp::ReverseSubtract:
                return GL_FUNC_REVERSE_SUBTRACT;
            case BlendOp::Min:
                return GL_MIN;
            case BlendOp::Max:
                return GL_MAX;
        }
    }

    inline GLenum ToMinFilter(Filter filter, MipmapMode mipmapMode) noexcept
    {
        const bool linear = filter == Filter::Linear;
        switch (mipmapMode)
        {
            default:
            case MipmapMode::None:
                return linear ? GL_LINEAR : GL_NEAREST;
            case MipmapMode::Nearest:
                return linear ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
            case MipmapMode::Linear:
                return linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    inline GLenum ToMagFilter(Filter filter) noexcept
    {
        return filter == Filter::Linear ? GL_LINEAR : GL_NEAREST;
    }

    // Native handle helpers: GL object names are carried in the void* native handle
    inline void* NameToHandle(GLuint name) noexcept
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(name));
    }
}
