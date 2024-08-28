#pragma once

#include<string>
#include<unordered_map>
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
        struct Character {
            glm::vec2 mPosition;
            glm::vec2 mSize;
            glm::vec2 mBearing;
            float mAdvance;
        };

        using FontAtlasMap = std::unordered_map<char, Character>;

        TextString();
        ~TextString();

        void CreateFontAtlas(const char* fontFilePath, const FT_UInt textSize);

        void SetText(const std::string_view text);

        void DrawText(const glm::mat4& pvMatrix);

    private:
size_t mTextLength =0;

        FT_Library mFontLibrary;
        FT_Face mFontFace;
        FontAtlasMap mFontAtlasMap;

        //std::string mText;
        sRef<Texture2D> mFontAtlasTex;
        sRef<VertexBuffer> mTextBuffer;
        sRef<VertexArray> mTextVA;
        sRef<ShaderProgram> mShader;
    };
}
