#pragma once

// Shared helpers for the Metal backend (metal-cpp). Include only from Metal backend files.

#include <Metal/Metal.hpp>

#include "Dahlia/Graphics/FormatConversion.h"
#include "Dahlia/Graphics/GraphicsTypes.h"

namespace DH::MTLConv
{
    // Format a texture is stored in. 3 component formats become 4 component and D24S8 is
    // substituted on GPUs without it (all Apple silicon).
    inline MTL::PixelFormat ToPixelFormat(Format format, MTL::Device* device) noexcept
    {
        switch (TextureStorageFormat(format))
        {
            case Format::R8:
                return MTL::PixelFormatR8Unorm;
            case Format::RG8:
                return MTL::PixelFormatRG8Unorm;
            case Format::RGBA8:
                return MTL::PixelFormatRGBA8Unorm;
            case Format::BGRA8:
                return MTL::PixelFormatBGRA8Unorm;
            case Format::R32F:
                return MTL::PixelFormatR32Float;
            case Format::RG32F:
                return MTL::PixelFormatRG32Float;
            case Format::RGBA32F:
                return MTL::PixelFormatRGBA32Float;
            case Format::RGBA16F:
                return MTL::PixelFormatRGBA16Float;
            case Format::D24S8:
                return device->depth24Stencil8PixelFormatSupported()
                         ? MTL::PixelFormatDepth24Unorm_Stencil8
                         : MTL::PixelFormatDepth32Float_Stencil8;
            case Format::D32F:
                return MTL::PixelFormatDepth32Float;
            default:
                return MTL::PixelFormatInvalid;
        }
    }

    inline MTL::VertexFormat ToVertexFormat(Format format) noexcept
    {
        switch (format)
        {
            case Format::R8:
                return MTL::VertexFormatUCharNormalized;
            case Format::RG8:
                return MTL::VertexFormatUChar2Normalized;
            case Format::RGB8:
                return MTL::VertexFormatUChar3Normalized;
            case Format::RGBA8:
                return MTL::VertexFormatUChar4Normalized;
            case Format::BGRA8:
                return MTL::VertexFormatUChar4Normalized_BGRA;
            case Format::R32F:
                return MTL::VertexFormatFloat;
            case Format::RG32F:
                return MTL::VertexFormatFloat2;
            case Format::RGB32F:
                return MTL::VertexFormatFloat3;
            case Format::RGBA32F:
                return MTL::VertexFormatFloat4;
            case Format::RGBA16F:
                return MTL::VertexFormatHalf4;
            default:
                return MTL::VertexFormatInvalid;
        }
    }

    inline MTL::PrimitiveType ToPrimitiveType(PrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
            default:
            case PrimitiveTopology::Triangles:
                return MTL::PrimitiveTypeTriangle;
            case PrimitiveTopology::TriangleStrip:
                return MTL::PrimitiveTypeTriangleStrip;
            case PrimitiveTopology::Lines:
                return MTL::PrimitiveTypeLine;
            case PrimitiveTopology::LineStrip:
                return MTL::PrimitiveTypeLineStrip;
            case PrimitiveTopology::Points:
                return MTL::PrimitiveTypePoint;
        }
    }

    inline MTL::CompareFunction ToCompareFunction(CompareOp op) noexcept
    {
        switch (op)
        {
            case CompareOp::Never:
                return MTL::CompareFunctionNever;
            default:
            case CompareOp::Less:
                return MTL::CompareFunctionLess;
            case CompareOp::Equal:
                return MTL::CompareFunctionEqual;
            case CompareOp::LessOrEqual:
                return MTL::CompareFunctionLessEqual;
            case CompareOp::Greater:
                return MTL::CompareFunctionGreater;
            case CompareOp::NotEqual:
                return MTL::CompareFunctionNotEqual;
            case CompareOp::GreaterOrEqual:
                return MTL::CompareFunctionGreaterEqual;
            case CompareOp::Always:
                return MTL::CompareFunctionAlways;
        }
    }

    inline MTL::CullMode ToCullMode(CullMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case CullMode::None:
                return MTL::CullModeNone;
            case CullMode::Front:
                return MTL::CullModeFront;
            case CullMode::Back:
                return MTL::CullModeBack;
        }
    }

    // Metal's NDC already has +y up like OpenGL, so winding maps directly
    inline MTL::Winding ToWinding(FrontFace face) noexcept
    {
        return face == FrontFace::Clockwise ? MTL::WindingClockwise : MTL::WindingCounterClockwise;
    }

    inline MTL::LoadAction ToLoadAction(LoadOp op) noexcept
    {
        switch (op)
        {
            case LoadOp::Load:
                return MTL::LoadActionLoad;
            default:
            case LoadOp::Clear:
                return MTL::LoadActionClear;
            case LoadOp::DontCare:
                return MTL::LoadActionDontCare;
        }
    }

    inline MTL::SamplerAddressMode ToAddressMode(AddressMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case AddressMode::Repeat:
                return MTL::SamplerAddressModeRepeat;
            case AddressMode::MirroredRepeat:
                return MTL::SamplerAddressModeMirrorRepeat;
            case AddressMode::ClampToEdge:
                return MTL::SamplerAddressModeClampToEdge;
            case AddressMode::ClampToBorder:
                return MTL::SamplerAddressModeClampToBorderColor;
        }
    }

    inline MTL::SamplerBorderColor ToBorderColor(BorderColour colour) noexcept
    {
        switch (colour)
        {
            default:
            case BorderColour::TransparentBlack:
                return MTL::SamplerBorderColorTransparentBlack;
            case BorderColour::OpaqueBlack:
                return MTL::SamplerBorderColorOpaqueBlack;
            case BorderColour::OpaqueWhite:
                return MTL::SamplerBorderColorOpaqueWhite;
        }
    }

    inline MTL::BlendFactor ToBlendFactor(BlendFactor factor) noexcept
    {
        switch (factor)
        {
            case BlendFactor::Zero:
                return MTL::BlendFactorZero;
            default:
            case BlendFactor::One:
                return MTL::BlendFactorOne;
            case BlendFactor::SrcColour:
                return MTL::BlendFactorSourceColor;
            case BlendFactor::OneMinusSrcColour:
                return MTL::BlendFactorOneMinusSourceColor;
            case BlendFactor::DstColour:
                return MTL::BlendFactorDestinationColor;
            case BlendFactor::OneMinusDstColour:
                return MTL::BlendFactorOneMinusDestinationColor;
            case BlendFactor::SrcAlpha:
                return MTL::BlendFactorSourceAlpha;
            case BlendFactor::OneMinusSrcAlpha:
                return MTL::BlendFactorOneMinusSourceAlpha;
            case BlendFactor::DstAlpha:
                return MTL::BlendFactorDestinationAlpha;
            case BlendFactor::OneMinusDstAlpha:
                return MTL::BlendFactorOneMinusDestinationAlpha;
        }
    }

    inline MTL::BlendOperation ToBlendOperation(BlendOp op) noexcept
    {
        switch (op)
        {
            default:
            case BlendOp::Add:
                return MTL::BlendOperationAdd;
            case BlendOp::Subtract:
                return MTL::BlendOperationSubtract;
            case BlendOp::ReverseSubtract:
                return MTL::BlendOperationReverseSubtract;
            case BlendOp::Min:
                return MTL::BlendOperationMin;
            case BlendOp::Max:
                return MTL::BlendOperationMax;
        }
    }

    inline MTL::SamplerMinMagFilter ToMinMagFilter(Filter filter) noexcept
    {
        return filter == Filter::Linear ? MTL::SamplerMinMagFilterLinear
                                        : MTL::SamplerMinMagFilterNearest;
    }

    // Resource storage for CPU written resources: shared on unified memory, managed otherwise
    inline MTL::ResourceOptions CPUWriteStorage(MTL::Device* device) noexcept
    {
        return device->hasUnifiedMemory() ? MTL::ResourceStorageModeShared
                                          : MTL::ResourceStorageModeManaged;
    }
}
