#pragma once

#include"GraphicsEngine/GEng_API.h"

namespace GEng {
    class Font;
}

namespace GEng::Res {
    class GENG_API ShaderSources {
    public:
        static const char *GetColourDiffuseGLSL();
        static const char *GetSolidColourGLSL();
        static const char *GetTextGLSL();
    };

    class GENG_API FontRes {
    public:
        static Font GetDefaultFont();
    };
}
