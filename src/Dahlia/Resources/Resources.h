#pragma once

#include "Dahlia/Dahlia-API.h"

namespace DH {
    class Font;
}

namespace DH::Res {
    class DH_API ShaderSources {
    public:
        static const char *GetColourDiffuseGLSL();
        static const char *GetSolidColourGLSL();
        static const char *GetTextGLSL();
    };

    class DH_API FontRes {
    public:
        static Font GetDefaultFont();
    };
}
