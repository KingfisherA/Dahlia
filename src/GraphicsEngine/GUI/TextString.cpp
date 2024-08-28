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

#include "TextString.h"

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

    TextString::TextString() {
        FT_Error error = FT_Init_FreeType(&mFontLibrary);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create a font library. ")+ GetFreeTypeErrorMsg(error));
            return;
        }

        mShader = ShaderProgram::CreateFromFile("/media/charlie/SSD/dev/C++/GraphicsEngine/Assets/Shaders/Text.glsl");
        mShader->Bind();
        mShader->UploadI("uFontAtlas", 0);
        mShader->UploadF("uTxtScaling", 0.8f);
        mShader->Unbind();


        sRef<Mesh> quadMesh = Mesh::RemoveIndexing(Geometry::CreateQuad());
        mTextVA = VertexArray::Create(quadMesh);

        mTextBuffer = VertexBuffer::Create(BufferUsage::DynamicUpdate);

        mTextVA->Bind();
        mTextVA->AddVertexBuffer(mTextBuffer, {4, 6, 0, 1});
        mTextVA->AddVertexBuffer(mTextBuffer, {2, 6, 4, 1});
        mTextVA->Unbind();
    }

    TextString::~TextString() {
        FT_Done_Face(mFontFace);
        FT_Done_FreeType(mFontLibrary);
    }

    void TextString::CreateFontAtlas(const char *fontFilePath, const FT_UInt textSize) {
        FT_Error error = FT_New_Face(mFontLibrary, fontFilePath, 0, &mFontFace);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create the required font face. ")+ GetFreeTypeErrorMsg(error));
            return;
        }

        FT_Set_Pixel_Sizes(mFontFace, 0, textSize);

        constexpr unsigned char START_GLYPH = 32;
        constexpr unsigned char END_GLYPH = 126;
        constexpr unsigned char NUM_GLYPHS = END_GLYPH - START_GLYPH;

        size_t texWidth = 1024;
        size_t texHeight = texWidth;
        unsigned char *atlasPixels = (unsigned char *) calloc(texWidth * texHeight * sizeof(unsigned char), 1);
        size_t penX = 0, penY = 0;

        for (unsigned char i = START_GLYPH; i <= END_GLYPH; i++) {
            FT_UInt glyphIndex = FT_Get_Char_Index(mFontFace, i);

            error = FT_Load_Glyph(mFontFace, glyphIndex, FT_LOAD_DEFAULT);
            if (error) continue;

            error = FT_Render_Glyph(mFontFace->glyph, FT_RENDER_MODE_NORMAL);
            if (error) continue;

            FT_GlyphSlot slot = mFontFace->glyph;
            FT_Bitmap *bmp = &slot->bitmap;

            glm::ivec2 glyphSize = {bmp->width, bmp->rows};
            glm::ivec2 glyphBearing = {slot->bitmap_left, slot->bitmap_top};
            size_t glyphAdvance = slot->advance.x >> 6;

            if (penY == 0)
                penY = glyphBearing.y;

            if (penX + glyphAdvance >= texWidth) {
                penX = 0;
                //Bitshift the height by 6 to get the value in pixels
                penY += ((mFontFace->size->metrics.height >> 6) + 1);
            }


            for (size_t row = 0; row < bmp->rows; ++row) {
                for (size_t col = 0; col < bmp->width; ++col) {
                    int x = penX + col + glyphBearing.x;
                    int y = penY + row - glyphBearing.y;
                    atlasPixels[y * texWidth + x] = bmp->buffer[row * bmp->pitch + col];
                }
            }

            Character charData = {
                glm::vec2{
                    (float) (penX + glyphBearing.x) / (float) texWidth,
                    (float) (penY - glyphBearing.y) / (float) texHeight
                },
                {(float) glyphSize.x / (float) texWidth, (float) glyphSize.y / (float) texHeight},
                {(float) glyphBearing.x / (float) texWidth, (float) glyphBearing.y / (float) texHeight},
                (float) glyphAdvance / (float) texWidth
            };

            mFontAtlasMap.insert(std::pair<char, Character>(i, charData));

            penX += glyphAdvance + 1;
        }

        mFontAtlasTex = Texture2D::Create({texWidth, texHeight},
                                          (const Texture2D::byte *) atlasPixels, PixelFormat::R);

        free(atlasPixels);
    }

    void TextString::SetText(const std::string_view text) {
        float *packedData = new float[6 * text.length()];

        mTextLength = text.length();

        float penX = -0.5f;
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            Character &charData = mFontAtlasMap.contains(c) ? mFontAtlasMap.at(c) : mFontAtlasMap.begin()->second;

            packedData[i * 6 + 0] = charData.mSize.x;
            packedData[i * 6 + 1] = charData.mSize.y;
            packedData[i * 6 + 2] = charData.mPosition.x;
            packedData[i * 6 + 3] = charData.mPosition.y;

            packedData[i * 6 + 4] = penX + charData.mBearing.x;
            packedData[i * 6 + 5] = -charData.mBearing.y;

            penX += charData.mAdvance;
        }

        mTextBuffer->Bind();
        mTextBuffer->StoreData(packedData, 6 * text.length());
        mTextBuffer->Unbind();

        delete[] packedData;
    }

    void TextString::DrawText(const glm::mat4 &pvMatrix) {
        mShader->Bind();
        mShader->UploadMatF("uPVMatrix", pvMatrix);

        mFontAtlasTex->Bind(0);

        mTextVA->Bind();
        mTextVA->SubmitArrays(GL_TRIANGLES, mTextLength);
        mTextVA->Unbind();

        mFontAtlasTex->Unbind();

        mShader->Unbind();
    }
} // GEng
