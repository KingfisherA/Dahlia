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

    struct AtlasGlyph {
        glm::vec2 mPosition;
        glm::vec2 mSize;
        glm::vec2 mBearing;
        float mAdvance;
    };

    using FontAtlasMap = std::unordered_map<char, AtlasGlyph>;

    class FontAtlasBase {
    public:
        virtual ~FontAtlasBase() = default;
        [[nodiscard]] virtual const FontAtlasMap &GetAtlasMap() const =0;
        [[nodiscard]] virtual FontAtlasMap &GetAtlasMap() =0;
    };

    struct FontAtlasBitmap : public FontAtlasBase {
    public:
        FontAtlasBitmap(): mDimensions(0) {
        }

        FontAtlasBitmap(unsigned char *bitmap, Dimension2D dimensions, const FontAtlasMap &map)
            : mDimensions(dimensions), mAtlasMap(map) {
            size_t noOfBytes = dimensions.x * dimensions.y * sizeof(unsigned char);
            mBitmap = (unsigned char *) malloc(noOfBytes);
            memcpy(mBitmap, bitmap, noOfBytes);
        }

        FontAtlasBitmap(move_construct_t, unsigned char *bitmap, Dimension2D dimensions, FontAtlasMap &&map)
            : mBitmap(bitmap), mDimensions(dimensions), mAtlasMap(std::move(map)) {
        }

        FontAtlasBitmap(const FontAtlasBitmap &other): mDimensions(other.mDimensions), mAtlasMap(other.mAtlasMap) {
            size_t noOfBytes = other.mDimensions.x * other.mDimensions.y * sizeof(unsigned char);
            mBitmap = (unsigned char *) malloc(noOfBytes);
            memcpy(mBitmap, other.mBitmap, noOfBytes);
        }

        FontAtlasBitmap(FontAtlasBitmap &&other) noexcept: mBitmap(other.mBitmap), mDimensions(other.mDimensions),
                                                           mAtlasMap(std::move(other.mAtlasMap)) {
            other.mBitmap = nullptr;
        }

        virtual ~FontAtlasBitmap() {
            free(mBitmap);
        }

        FontAtlasBitmap &operator=(const FontAtlasBitmap &other) {
            mDimensions = other.mDimensions;
            mAtlasMap = other.mAtlasMap;
            size_t noOfBytes = other.mDimensions.x * other.mDimensions.y * sizeof(unsigned char);
            mBitmap = (unsigned char *) malloc(noOfBytes);
            memcpy(mBitmap, other.mBitmap, noOfBytes);

            return *this;
        }

        FontAtlasBitmap &operator=(FontAtlasBitmap &&other) {
            mDimensions = other.mDimensions;
            mAtlasMap = std::move(other.mAtlasMap);
            mBitmap = other.mBitmap;
            other.mBitmap = nullptr;

            return *this;
        }

        virtual const FontAtlasMap &GetAtlasMap() const override { return mAtlasMap; }
        virtual FontAtlasMap &GetAtlasMap() override { return mAtlasMap; }

        const Dimension2D &GetDimensions() const { return mDimensions; }
        Dimension2D &GetDimensions() { return mDimensions; }
        const unsigned char *GetBitmap() const { return mBitmap; }
        unsigned char *&GetBitmap() { return mBitmap; }

    private:
        FontAtlasMap mAtlasMap;
        unsigned char *mBitmap = nullptr;
        Dimension2D mDimensions;
    };

    struct FontAtlas : public FontAtlasBase {
    public:
        FontAtlas() = default;

        explicit FontAtlas(const FontAtlasBitmap &bmp): mAtlasMap(bmp.GetAtlasMap()) {
            mTexture = Texture2D::Create(bmp.GetDimensions(), bmp.GetBitmap(), PixelFormat::R);
        }

        FontAtlas(const unsigned char *bitmap, Dimension2D dimensions, const FontAtlasMap &map)
            : mAtlasMap(map) {
            mTexture = Texture2D::Create(dimensions, bitmap, PixelFormat::R);
        }

        FontAtlas(move_construct_t, const unsigned char *bitmap, Dimension2D dimensions, FontAtlasMap &&map)
            : mAtlasMap(std::move(map)) {
            mTexture = Texture2D::Create(dimensions, bitmap, PixelFormat::R);
        }

        virtual const FontAtlasMap &GetAtlasMap() const override { return mAtlasMap; }
        virtual FontAtlasMap &GetAtlasMap() override { return mAtlasMap; }
        const sRef<Texture2D> &GetTexture() const { return mTexture; }

    private:
        FontAtlasMap mAtlasMap;
        sRef<Texture2D> mTexture;
    };

    class FontManager {
    public:
        FontManager();
        ~FontManager();

        enum class GlyphRenderingMode {
            Normal = 0,
            SignedDistanceField
        };

        constexpr static const char *ASCII_GLYPH_SET =
                " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

        sRef<FontAtlasBitmap> CreateFontAtlasBitmap(const char *fontFilePath, const FT_UInt glyphResolution,
                                                    const char *glyphSet = ASCII_GLYPH_SET,
                                                    const GlyphRenderingMode mode = GlyphRenderingMode::Normal) const;

        sRef<FontAtlas> CreateFontAtlas(const char *fontFilePath, const FT_UInt glyphResolution,
                                        const char *glyphSet = ASCII_GLYPH_SET,
                                        const GlyphRenderingMode mode = GlyphRenderingMode::Normal) const;
        static sRef<FontAtlas> CreateFontAtlas(const FontAtlasBitmap &bitmap);

        static const AtlasGlyph &MeasureGlyph(const FontAtlasBase &atlas, const char c) {
            return atlas.GetAtlasMap().at(c);
        }

        static glm::vec2 MeasureText(const FontAtlasBase &atlas, std::string_view text);

    private:
        FT_Library mFontLibrary = nullptr;
    };

    class Text {
    public:
        Text();
        ~Text();

        void SetScaling(const float scaling) { mScaling = scaling; }
        void LayoutText(const std::string_view text, const FontAtlas &atlas, const glm::vec2& pos);
        void DrawText(const glm::mat4 &pvMatrix) const;

    private:
        size_t mNoOfGlyphs = 0;
        float mScaling = 0.8f;

        sRef<Texture2D> mFontAtlasTex;
        sRef<VertexBuffer> mTextBuffer;
        sRef<VertexArray> mTextVA;
        static sRef<ShaderProgram> mShader;
    };
}
