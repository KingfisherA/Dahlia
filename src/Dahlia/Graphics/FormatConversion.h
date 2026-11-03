#pragma once

// Helpers for backends whose texture formats differ from the common Format list.

#include <cstdint>
#include <cstring>
#include <vector>

#include "Dahlia/Graphics/GraphicsTypes.h"

namespace DH
{
    // Vulkan and Metal have no (portably) sampleable 3 component formats, so RGB textures are
    // stored as RGBA
    constexpr Format TextureStorageFormat(Format format) noexcept
    {
        switch (format)
        {
            case Format::RGB8:
                return Format::RGBA8;
            case Format::RGB32F:
                return Format::RGBA32F;
            default:
                return format;
        }
    }

    // Converts `pixelCount` tightly packed pixels of `format` to TextureStorageFormat(format).
    // Returns an empty vector when the format is already stored as is.
    inline std::vector<uint8_t> ExpandToStorageFormat(const void* data,
                                                      size_t pixelCount,
                                                      Format format)
    {
        if (TextureStorageFormat(format) == format || data == nullptr)
            return {};

        const auto* src = static_cast<const uint8_t*>(data);
        if (format == Format::RGB8)
        {
            std::vector<uint8_t> out(pixelCount * 4);
            for (size_t i = 0; i < pixelCount; i++)
            {
                out[i * 4 + 0] = src[i * 3 + 0];
                out[i * 4 + 1] = src[i * 3 + 1];
                out[i * 4 + 2] = src[i * 3 + 2];
                out[i * 4 + 3] = 255;
            }
            return out;
        }

        // RGB32F
        std::vector<uint8_t> out(pixelCount * 16);
        const float one = 1.0f;
        for (size_t i = 0; i < pixelCount; i++)
        {
            std::memcpy(out.data() + i * 16, src + i * 12, 12);
            std::memcpy(out.data() + i * 16 + 12, &one, 4);
        }
        return out;
    }
}
