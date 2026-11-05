// Engine shaders, compiled from Shaders/*.vert|frag to SPIR-V at build time by
// cmake/DahliaShaders.cmake (glslc -mfmt=c emits each module as a C initializer list).

#include "Resources.h"

namespace DH::Res
{
    namespace
    {
        constexpr uint32_t COLOUR_DIFFUSE_VERT[] =
#include "ColourDiffuse.vert.inc"
          ;
        constexpr uint32_t COLOUR_DIFFUSE_FRAG[] =
#include "ColourDiffuse.frag.inc"
          ;
        constexpr uint32_t SOLID_COLOUR_VERT[] =
#include "SolidColour.vert.inc"
          ;
        constexpr uint32_t SOLID_COLOUR_FRAG[] =
#include "SolidColour.frag.inc"
          ;
        constexpr uint32_t TEXT_VERT[] =
#include "Text.vert.inc"
          ;
        constexpr uint32_t TEXT_FRAG[] =
#include "Text.frag.inc"
          ;
    }

    std::span<const uint32_t> ShaderSources::GetColourDiffuseVert()
    {
        return COLOUR_DIFFUSE_VERT;
    }

    std::span<const uint32_t> ShaderSources::GetColourDiffuseFrag()
    {
        return COLOUR_DIFFUSE_FRAG;
    }

    std::span<const uint32_t> ShaderSources::GetSolidColourVert()
    {
        return SOLID_COLOUR_VERT;
    }

    std::span<const uint32_t> ShaderSources::GetSolidColourFrag()
    {
        return SOLID_COLOUR_FRAG;
    }

    std::span<const uint32_t> ShaderSources::GetTextVert()
    {
        return TEXT_VERT;
    }

    std::span<const uint32_t> ShaderSources::GetTextFrag()
    {
        return TEXT_FRAG;
    }
}
