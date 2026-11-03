#pragma once

// Shared helpers for the Vulkan backend. Include only from Vulkan backend files.

#include <stdexcept>
#include <string>

#include <vulkan/vulkan.h>

#include "Dahlia/Graphics/GraphicsTypes.h"

#define DH_VK_CHECK(expr, what)                                                                    \
    do                                                                                             \
    {                                                                                              \
        const VkResult dhVkResult = (expr);                                                        \
        if (dhVkResult != VK_SUCCESS)                                                              \
            throw std::runtime_error(std::string(what) + " failed (VkResult " +                   \
                                     std::to_string((int)dhVkResult) + ")");                       \
    } while (0)

namespace DH::VK
{
    // Vertex attribute formats are used directly (3 component formats are fine for vertices)
    inline VkFormat ToVertexFormat(Format format) noexcept
    {
        switch (format)
        {
            case Format::R8:
                return VK_FORMAT_R8_UNORM;
            case Format::RG8:
                return VK_FORMAT_R8G8_UNORM;
            case Format::RGB8:
                return VK_FORMAT_R8G8B8_UNORM;
            case Format::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case Format::BGRA8:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case Format::R32F:
                return VK_FORMAT_R32_SFLOAT;
            case Format::RG32F:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::RGB32F:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    inline VkImageAspectFlags ToAspect(Format format) noexcept
    {
        if (!IsDepthFormat(format))
            return VK_IMAGE_ASPECT_COLOR_BIT;
        return HasStencil(format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                  : VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    inline VkPrimitiveTopology ToTopology(PrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
            default:
            case PrimitiveTopology::Triangles:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopology::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case PrimitiveTopology::Lines:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopology::Points:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }
    }

    inline VkCompareOp ToCompareOp(CompareOp op) noexcept
    {
        switch (op)
        {
            case CompareOp::Never:
                return VK_COMPARE_OP_NEVER;
            default:
            case CompareOp::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOp::LessOrEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::GreaterOrEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;
        }
    }

    inline VkPolygonMode ToPolygonMode(PolygonMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case PolygonMode::Solid:
                return VK_POLYGON_MODE_FILL;
            case PolygonMode::Lines:
                return VK_POLYGON_MODE_LINE;
            case PolygonMode::Points:
                return VK_POLYGON_MODE_POINT;
        }
    }

    inline VkCullModeFlags ToCullMode(CullMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
        }
    }

    // The Vulkan backend flips the viewport (negative height) so NDC +y points up as in OpenGL.
    // That flip also preserves OpenGL's winding, so front faces map directly.
    inline VkFrontFace ToFrontFace(FrontFace face) noexcept
    {
        return face == FrontFace::Clockwise ? VK_FRONT_FACE_CLOCKWISE
                                            : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    inline VkFilter ToFilter(Filter filter) noexcept
    {
        return filter == Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }

    inline VkSamplerAddressMode ToAddressMode(AddressMode mode) noexcept
    {
        switch (mode)
        {
            default:
            case AddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case AddressMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case AddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    inline VkIndexType ToIndexType(IndexType type) noexcept
    {
        return type == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    }

    inline VkAttachmentLoadOp ToLoadOp(LoadOp op) noexcept
    {
        switch (op)
        {
            case LoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            default:
            case LoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case LoadOp::DontCare:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    // Full pipeline barrier transition. Deliberately conservative (all commands, all memory) to
    // keep the backend simple and correct.
    inline void TransitionImage(VkCommandBuffer cmd,
                                VkImage image,
                                VkImageAspectFlags aspect,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout,
                                uint32_t baseMip = 0,
                                uint32_t mipCount = VK_REMAINING_MIP_LEVELS)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = { aspect, baseMip, mipCount, 0, 1 };

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
    }
}
