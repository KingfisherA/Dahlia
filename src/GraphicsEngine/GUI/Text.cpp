//
// Created by charlie on 27/08/24.
//

#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/3D/Buffers.h"
#include "GraphicsEngine/3D/ShaderProgram.h"
#include "GraphicsEngine/Maths/Geometry.hpp"
#include "GraphicsEngine/Utilities/Assert.h"
#include "GraphicsEngine/3D/Texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Text.h"

#include <glm/gtx/transform.hpp>


namespace GEng {
    static const char *GetFreeTypeErrorMsg(FT_Error err) {
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
        return "(Unknown error)";
    }

    FontManager::FontManager() {
        FT_Error error = FT_Init_FreeType(&mFontLibrary);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create a font library. ")+ GetFreeTypeErrorMsg(error));
            return;
        }
    }

    FontManager::~FontManager() {
        FT_Done_FreeType(mFontLibrary);
    }

    sRef<FontAtlasBitmap> FontManager::CreateFontAtlasBitmap(const char *fontFilePath,
                                                             const FT_UInt glyphResolution,
                                                             const char *glyphSet,
                                                             const GlyphRenderingMode mode) const {
        FT_Face fontFace = nullptr;
        FT_Error error = FT_New_Face(mFontLibrary, fontFilePath, 0, &fontFace);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create the required font face. ")+ GetFreeTypeErrorMsg(error));
            throw std::runtime_error("FreeType could not create the required font face.");
        }

        FT_Set_Pixel_Sizes(fontFace, 0, glyphResolution);

        size_t NUM_GLYPHS = strlen(glyphSet);

        size_t texWidth = 1024;
        size_t texHeight = texWidth;
        size_t penX = 0, penY = 0;

        sRef<FontAtlasBitmap> atlasBmp = std::make_shared<FontAtlasBitmap>();
        atlasBmp->GetBitmap() = (unsigned char *) malloc(texWidth * texHeight * sizeof(unsigned char));
        atlasBmp->GetDimensions() = {texWidth, texHeight};

        FT_Render_Mode glyphRenderMode = [mode]() {
            switch (mode) {
                case GlyphRenderingMode::SignedDistanceField:
                    return FT_RENDER_MODE_SDF;
                default:
                case GlyphRenderingMode::Normal:
                    return FT_RENDER_MODE_NORMAL;
            }
        }();


        for (size_t i = 0; i < NUM_GLYPHS; i++) {
            char character = glyphSet[i];

            FT_UInt glyphIndex = FT_Get_Char_Index(fontFace, character);

            error = FT_Load_Glyph(fontFace, glyphIndex, FT_LOAD_DEFAULT);
            if (error) continue;

            error = FT_Render_Glyph(fontFace->glyph, glyphRenderMode);
            if (error) continue;

            FT_GlyphSlot slot = fontFace->glyph;
            FT_Bitmap *bmp = &slot->bitmap;

            glm::ivec2 glyphSize = {bmp->width, bmp->rows};
            glm::ivec2 glyphBearing = {slot->bitmap_left, slot->bitmap_top};
            size_t glyphAdvance = slot->advance.x >> 6;

            if (penY == 0)
                penY = glyphBearing.y;

            if (penX + glyphAdvance >= texWidth) {
                penX = 0;
                //Bitshift the height by 6 to get the value in pixels
                penY += ((fontFace->size->metrics.height >> 6) + 1);
            }


            for (size_t row = 0; row < bmp->rows; ++row) {
                for (size_t col = 0; col < bmp->width; ++col) {
                    int x = penX + col + glyphBearing.x;
                    int y = penY + row - glyphBearing.y;
                    atlasBmp->GetBitmap()[y * texWidth + x] = bmp->buffer[row * bmp->pitch + col];
                }
            }

            AtlasGlyph glyphData = {
                glm::vec2{
                    ((float) penX + (float) glyphBearing.x) / (float) texWidth,
                    ((float) penY - (float) glyphBearing.y) / (float) texHeight
                },
                {(float) glyphSize.x / (float) texWidth, (float) glyphSize.y / (float) texHeight},
                {(float) glyphBearing.x / (float) texWidth, (float) glyphBearing.y / (float) texHeight},
                (float) glyphAdvance / (float) texWidth
            };

            atlasBmp->GetAtlasMap().insert(std::pair<char, AtlasGlyph>(character, glyphData));

            penX += glyphAdvance + 1;
        }

        FT_Done_Face(fontFace);
        return atlasBmp;
    }

    sRef<FontAtlas> FontManager::CreateFontAtlas(const char *fontFilePath, const FT_UInt glyphResolution,
                                                 const char *glyphSet,
                                                 const GlyphRenderingMode mode) const {
        return std::make_shared<FontAtlas>(*CreateFontAtlasBitmap(fontFilePath, glyphResolution, glyphSet, mode));
    }

    sRef<FontAtlas> FontManager::CreateFontAtlas(const FontAtlasBitmap &bitmap) {
        return std::make_shared<FontAtlas>(bitmap);
    }

    glm::vec2 FontManager::MeasureText(const FontAtlasBase &atlas, std::string_view text) {
        float width = 0.0f, height = 0.0f, lineWidth = 0.0f, lineHeight = 0.0f;

        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (!atlas.GetAtlasMap().contains(c))
                continue;
            const AtlasGlyph &glyphData = atlas.GetAtlasMap().at(c);

            lineHeight = std::max(lineHeight, glyphData.mSize.y);

            lineWidth += glyphData.mAdvance;

            if (c == '\n') {
                width = std::max(width, lineWidth);
                lineWidth = 0.0f;
                height += lineHeight;
                lineHeight = 0.0f;
            }
        }

        width = std::max(width, lineWidth);
        height += lineHeight;

        return {width, height};
    }

    sRef<ShaderProgram> Text::mShader = nullptr;

    Text::Text() {
        if (mShader == nullptr) {
            mShader = ShaderProgram::CreateFromFile(
                "/media/charlie/SSD/dev/C++/GraphicsEngine/Assets/Shaders/Text.glsl");
            mShader->Bind();
            mShader->UploadI("uFontAtlas", 0);
            mShader->Unbind();
        }

        mTextVA = VertexArray::Create(Mesh::RemoveIndexing(Geometry::CreateQuad()));

        mTextBuffer = VertexBuffer::Create(BufferUsage::DynamicUpdate);

        mTextVA->Bind();
        mTextVA->AddVertexBuffer(mTextBuffer, {4, 6, 0, 1});
        mTextVA->AddVertexBuffer(mTextBuffer, {2, 6, 4, 1});
        mTextVA->Unbind();
    }

    Text::~Text() {
        mShader.reset();
        mTextVA.reset();
    }

    void Text::LayoutText(const std::string_view text, const FontAtlas &atlas, const glm::vec2 &pos) {
        float *packedData = new float[6 * text.length()];

        mNoOfGlyphs = text.length();

        float penX = pos.x, penY = pos.y, largestGlyph = 0.0f;
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];

            if (c == '\n') {
                penX = pos.x;
                penY += largestGlyph;
                continue;
            }

            const AtlasGlyph &glyphData = atlas.GetAtlasMap().contains(c)
                                              ? atlas.GetAtlasMap().at(c)
                                              : atlas.GetAtlasMap().begin()->second;

            packedData[i * 6 + 0] = glyphData.mSize.x;
            packedData[i * 6 + 1] = glyphData.mSize.y;
            packedData[i * 6 + 2] = glyphData.mPosition.x;
            packedData[i * 6 + 3] = glyphData.mPosition.y;

            largestGlyph = std::max(largestGlyph, glyphData.mSize.y);

            packedData[i * 6 + 4] = penX + glyphData.mBearing.x;
            packedData[i * 6 + 5] = penY - glyphData.mBearing.y;

            penX += glyphData.mAdvance;
        }

        mTextBuffer->Bind();
        mTextBuffer->StoreData(packedData, 6 * text.length());
        mTextBuffer->Unbind();

        delete[] packedData;

        mFontAtlasTex = atlas.GetTexture();
    }

    void Text::DrawText(const glm::mat4 &pvMatrix) const {
        mShader->Bind();
        mShader->UploadMatF("uPVMatrix", pvMatrix);
        mShader->UploadF("uTxtScaling", mScaling);

        mFontAtlasTex->Bind(0);

        mTextVA->Bind();
        mTextVA->SubmitArrays(GL_TRIANGLES, mNoOfGlyphs);
        mTextVA->Unbind();

        mFontAtlasTex->Unbind();

        mShader->Unbind();
    }
} // GEng
