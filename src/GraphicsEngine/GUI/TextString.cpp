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

    TextString::~TextString() {
        FT_Done_Face(mFontFace);
        FT_Done_FreeType(mFontLibrary);
    }

    bool TextString::Init() {
        FT_Error error = FT_Init_FreeType(&mFontLibrary);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create a font library. ")+ GetFreeTypeErrorMsg(error));
            return false;
        }

        error = FT_New_Face(mFontLibrary, "/usr/share/fonts/truetype/ubuntu/Ubuntu-Th.ttf", 0, &mFontFace);
        if (error) {
            UNI_FATAL(GEng::TextString,
                      std::string("FreeType could not create the required font face. ")+ GetFreeTypeErrorMsg(error));
            return false;
        }

        mShader = ShaderProgram::CreateFromFile("/media/charlie/SSD/dev/C++/GraphicsEngine/Assets/Shaders/Text.glsl");
        mShader->Bind();
        mShader->UploadI("uFontAtlas", 0);
        mShader->Unbind();


        sRef<Mesh> quadMesh = Mesh::RemoveIndexing(Geometry::CreateQuad());
        mTextVA = VertexArray::Create(quadMesh);

        return true;
    }

    void TextString::UploadText(const std::string_view text, const float textSize) {
        FT_Set_Pixel_Sizes(mFontFace, 0, textSize);

        constexpr unsigned char START_GLYPH = 32;
        constexpr unsigned char END_GLYPH = 126;
        constexpr unsigned char NUM_GLYPHS = END_GLYPH - START_GLYPH;

        size_t maxDim = (1 + (mFontFace->size->metrics.height >> 6)) * ceilf(sqrtf(NUM_GLYPHS));
        size_t texWidth = 1;
        while (texWidth < maxDim) texWidth <<= 1;

        unsigned char *atlasPixels = (unsigned char *) calloc(texWidth * texWidth * sizeof(unsigned char), 1);
        size_t penX = 0, penY = 0;

        for (unsigned char i = START_GLYPH; i <= END_GLYPH; i++) {
            FT_UInt glyphIndex = FT_Get_Char_Index(mFontFace, i);

            FT_Error error = FT_Load_Glyph(mFontFace, glyphIndex, FT_LOAD_DEFAULT);
            if (error) continue;

            error = FT_Render_Glyph(mFontFace->glyph, FT_RENDER_MODE_NORMAL);
            if (error) continue;

            FT_GlyphSlot slot = mFontFace->glyph;
            FT_Bitmap *bmp = &slot->bitmap;

            glm::ivec2 glyphSize = {bmp->width, bmp->rows};
            glm::ivec2 glyphBearing = {slot->bitmap_left, slot->bitmap_top};

            if (penX + bmp->width + glyphBearing.x >= texWidth) {
                penX = 0;
                penY += ((mFontFace->size->metrics.height >> 6) +glyphBearing.y+ 1);
            }


            for (size_t row = 0; row < bmp->rows; ++row) {
                for (size_t col = 0; col < bmp->width; ++col) {
                    int x = penX + col + glyphBearing.x;
                    int y = penY + row + (glyphSize.y - glyphBearing.y);
                    atlasPixels[y * texWidth + x] = bmp->buffer[row * bmp->pitch + col];
                }
            }

            penX += bmp->width + glyphBearing.x + 1;
        }

        mFontAtlas = Texture2D::Create({(float) texWidth, (float) texWidth},
                                       (const Texture2D::byte *) atlasPixels, PixelFormat::R);

        free(atlasPixels);
        //mShader->Bind();
        //mShader->UploadVecF("uSize", glm::vec2(1.0f, (float) slot->bitmap.rows / (float) slot->bitmap.width) * 0.01f);
        // mShader->Unbind();
    }

    void TextString::DrawText(const glm::mat4 &pvMatrix) {
        mShader->Bind();
        mShader->UploadMatF("uPVMatrix", pvMatrix * glm::translate(glm::vec3{0.0f, 0.0f, 0.0f}));

        mFontAtlas->Bind(0);

        mTextVA->Bind();
        mTextVA->SubmitArrays();
        mTextVA->Unbind();

        mFontAtlas->Unbind();

        mShader->Unbind();
    }
} // GEng
