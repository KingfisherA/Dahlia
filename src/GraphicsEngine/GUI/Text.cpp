//
// Created by charlie on 27/08/24.
//

#include "GraphicsEngine/GUI/Text.h"
#include "GraphicsEngine/3D/VertexArray.h"
#include "GraphicsEngine/3D/Buffers.h"
#include "GraphicsEngine/3D/ShaderProgram.h"
#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/Utilities/Assert.h"
#include "GraphicsEngine/3D/Texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtx/transform.hpp>

#include "GraphicsEngine/Platform/OpenGL/OpenGLShaderProgram.h"


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
            GENG_FATAL(GEng::TextString,
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
            GENG_FATAL(GEng::TextString,
                       std::string("FreeType could not create the required font face. ")+ GetFreeTypeErrorMsg(error));
            throw std::runtime_error("FreeType could not create the required font face.");
        }

        FT_Set_Pixel_Sizes(fontFace, 0, glyphResolution);

        size_t NUM_GLYPHS = strlen(glyphSet);

        size_t texWidth = 1024;
        size_t texHeight = texWidth;
        size_t penX = 0, penY = 100;

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

            AtlasGlyphInfo glyphData = {
                glm::vec2{
                    ((float) penX + (float) glyphBearing.x) / (float) texWidth,
                    ((float) penY - (float) glyphBearing.y) / (float) texHeight
                },
                {(float) glyphSize.x / (float) texWidth, (float) glyphSize.y / (float) texHeight},
                {(float) glyphBearing.x / (float) texWidth, (float) glyphBearing.y / (float) texHeight},
                (float) glyphAdvance / (float) texWidth
            };

            atlasBmp->GetAtlasMap().insert(std::pair<char, AtlasGlyphInfo>(character, glyphData));

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

    std::vector<glm::vec2> FontManager::LayoutText(const std::string_view text, const FontAtlasBase &atlas,
                                                   const TextLayoutInfo &layoutInfo) {
        std::vector<glm::vec2> glyphPositions(text.length());

        constexpr float VERTICAL_PADDING = 0.05f;
        float penX = layoutInfo.bounds.x, penY = layoutInfo.bounds.y, largestGlyph = 0.0f;
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];

            if (c == '\n') {
                penX = layoutInfo.bounds.x;
                penY += largestGlyph + VERTICAL_PADDING;
                continue;
            }

            if (penX >= layoutInfo.bounds.x + layoutInfo.bounds.z && layoutInfo.bounds.z > 0.0f && c == ' ') {
                penX = layoutInfo.bounds.x;
                penY += largestGlyph;
            }


            const AtlasGlyphInfo &glyphData = atlas.GetAtlasMap().contains(c)
                                                  ? atlas.GetAtlasMap().at(c)
                                                  : atlas.GetAtlasMap().begin()->second;

            largestGlyph = std::max(largestGlyph, glyphData.mSize.y + glyphData.mBearing.y);

            glyphPositions[i] = {penX + glyphData.mBearing.x, penY - glyphData.mBearing.y};

            penX += glyphData.mAdvance;
        }

        return glyphPositions;
    }

    glm::vec2 FontManager::MeasureText(const FontAtlasBase &atlas, std::string_view text) {
        std::vector<glm::vec2> glyphPositions = LayoutText(text, atlas);
        float width = 0.0f, height = 0.0f;

        for (size_t i = 0; i < glyphPositions.size(); i++) {
            glm::vec2 &currentPosition = glyphPositions[i];

            if (currentPosition.x > width)
                width = currentPosition.x;

            if (currentPosition.y > height)
                height = currentPosition.y;
        }

        return {width, height};
    }

    sRef<ShaderProgram> Text::mShader = nullptr;

    Text::Text() {
        if (mShader == nullptr) {
            mShader = ShaderProgram::CreateFromFile(
                "../../../../Assets/Shaders/Text.glsl");
            mShader->UploadI("uFontAtlas", 0);
        }

        mTextVA = VertexArray::Create(Mesh::RemoveIndexing(Geometry::CreateQuad()));
        mTextBuffer = VertexBuffer::Create(BufferUsage::DynamicUpdate);
        mTextVA->AddVertexBuffer(mTextBuffer, {4, 6, 0, 1});
        mTextVA->AddVertexBuffer(mTextBuffer, {2, 6, 4, 1});
    }

    Text::~Text() {
        mShader.reset();
        mTextVA.reset();
    }

    sRef<Text> Text::Create() {
        return std::make_shared<Text>();
    }

    void Text::AddText(const std::string_view text, const TextLayoutInfo &layoutInfo) {
        mTextStrings.emplace_back(text, TextLayoutInfo{
                                      glm::vec3{
                                          glm::vec2(layoutInfo.bounds.x, layoutInfo.bounds.y) / mScaling,
                                          layoutInfo.bounds.z
                                      },
                                      layoutInfo.alignment
                                  });
    }

    void Text::ClearText() {
        mTextStrings.clear();
    }

    void Text::LayoutText(const FontAtlas &atlas) {
        std::vector<std::vector<glm::vec2> > glyphPositions;
        glyphPositions.reserve(mTextStrings.size());

        size_t packedDataMaxSize = 0;
        for (const auto &textString: mTextStrings) {
            glyphPositions.emplace_back(FontManager::LayoutText(textString.first, atlas, textString.second));

            packedDataMaxSize += glyphPositions.back().size() * 6;
        }

        float *packedData = new float[packedDataMaxSize];

        size_t k = 0;

        for (size_t i = 0; i < mTextStrings.size(); i++) {
            const std::string &text = mTextStrings[i].first;
            const std::vector<glm::vec2> &currentGlyphPositions = glyphPositions[i];

            for (size_t j = 0; j < text.length(); j++) {
                char c = text[j];
                if (c == '\n') continue;
                const glm::vec2 &pos = currentGlyphPositions[j];
                const AtlasGlyphInfo &glyphData = atlas.GetAtlasMap().contains(c)
                                                      ? atlas.GetAtlasMap().at(c)
                                                      : atlas.GetAtlasMap().begin()->second;

                packedData[k++] = glyphData.mSize.x;
                packedData[k++] = glyphData.mSize.y;
                packedData[k++] = glyphData.mPosition.x;
                packedData[k++] = glyphData.mPosition.y;

                packedData[k++] = pos.x;
                packedData[k++] = pos.y;
            }
        }

        mNoOfGlyphs = k / 6;

        mTextBuffer->StoreData(packedData, k);

        delete[] packedData;

        mFontAtlasTex = atlas.GetTexture();
    }

    void Text::DrawText(const glm::mat4 &pvMatrix) const {
        glDisable(GL_DEPTH_TEST);
        mShader->Bind();
        mShader->UploadMatF("uPVMatrix", pvMatrix);
        mShader->UploadF("uTxtScaling", mScaling);

        mFontAtlasTex->Bind(0);

        mTextVA->SubmitArrays(Primitive::Triangles, mNoOfGlyphs);

        glEnable(GL_DEPTH_TEST);
    }
} // GEng
