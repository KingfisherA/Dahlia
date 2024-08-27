#pragma once

#include<string>
#include <glm/matrix.hpp>
#include "GraphicsEngine/Utilities/Memory.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace GEng {
    class VertexBuffer;
    class VertexArray;
    class ShaderProgram;
    class Texture2D;

    class FontManager {
    public:

    private:
    };

    class Font {
    public:
        Font();

    private:
    };

    class TextString {
    public:

        ~TextString();

        bool Init();

        void UploadText(const std::string_view text, const float textSize);

        void DrawText(const glm::mat4& pvMatrix);

    private:
        FT_Library mFontLibrary;
        FT_Face mFontFace;

        //std::string mText;
        sRef<Texture2D> mFontAtlas;
        sRef<VertexBuffer> mTextBuffer;
        sRef<VertexArray> mTextVA;
        sRef<ShaderProgram> mShader;
    };
}
