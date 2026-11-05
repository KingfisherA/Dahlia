#pragma once

#include <cstdint>
#include <span>

#include "Dahlia/Dahlia-API.h"

namespace DH {
    class Font;
}

namespace DH::Res {
    // SPIR-V for the engine's built-in shaders (see Shaders/ and cmake/DahliaShaders.cmake)
    class DH_API ShaderSources {
    public:
        static std::span<const uint32_t> GetColourDiffuseVert();
        static std::span<const uint32_t> GetColourDiffuseFrag();
        static std::span<const uint32_t> GetSolidColourVert();
        static std::span<const uint32_t> GetSolidColourFrag();
        static std::span<const uint32_t> GetTextVert();
        static std::span<const uint32_t> GetTextFrag();
    };

    class DH_API FontRes {
    public:
        static Font GetDefaultFont();
    };
}
