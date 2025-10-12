#include "Resources.h"

namespace DH::Res {
        static const char * COLOUR_DIFFUSE_GLSL = R"(#version 410

    #pragma remove
    #include "GLSL Language Specification.h.glsl"
    #pragma endremove

    #begin(vShader)

    layout(location = 0) in vec3 aVertex;
    //
    uniform mat4 uTransformMatrix;
    uniform mat4 uPVMatrix;
    //
    out vec3 vNormal;

    void main()
    {
        vec4 transformedV = uTransformMatrix * vec4(aVertex, 1.0f);
        vNormal = normalize(transformedV.xyz);
        gl_Position = uPVMatrix * transformedV;
    }

    #end(vShader)
    #begin(fShader)

    in vec3 vNormal;
    //
    out vec4 fragColour;

    uniform vec3 uLightPos = vec3(10.0f, 10.0f, 10.0f);
    uniform vec3 uColour = vec3(1.0f, 1.0f, 1.0f);
    uniform float uOpacity = 1.0f;

    void main()
    {
        vec3 unitLightVector = normalize(uLightPos);
        float diffuseAmount = clamp(dot(unitLightVector, vNormal), 0.1f, 1.0f);

        fragColour = vec4(uColour * diffuseAmount, uOpacity);
    }
    #end(fShader))";

         static const char *SOLID_COLOUR_GLSL = R"(#version 410

    #pragma remove
    #include "GLSL Language Specification.h.glsl"
    #pragma endremove

    #begin(vShader)

    layout(location = 0) in vec2 aVertex;
    //
    uniform mat4 uTransformMatrix;
    uniform mat4 uPVMatrix;
    //

    void main()
    {
        gl_Position = uPVMatrix * uTransformMatrix * vec4(aVertex, 0.0f, 1.0f);
    }

    #end(vShader)
    #begin(fShader)

    //
    out vec4 fragColour;

    uniform vec3 uColour = vec3(1.0f, 1.0f, 1.0f);
    uniform float uOpacity = 1.0f;

    void main()
    {
        fragColour = vec4(uColour, uOpacity);
    }
    #end(fShader))";

        static const char * TEXT_GLSL = R"(#version 410

    #pragma remove
    #include "GLSL Language Specification.h.glsl"
    #pragma endremove

    #begin(vShader)

    layout(location = 0) in vec2 aVertex;
    layout(location = 1) in vec4 aGlyph;
    layout(location = 2) in vec2 aPosition;
    //
    uniform mat4 uPVMatrix;
    // uniform vec4 uGlyph;
    // uniform vec2 uPosition;
    uniform float uTxtScaling = 1.0f;
    //
    out vec2 texCoords;

    void main()
    {
        texCoords = ((aVertex + vec2(1.0f)) * 0.5f);

        // gl_Position = uPVMatrix * vec4(aVertex, 0.0f, 1.0f);
        gl_Position = uPVMatrix * vec4((vec2(texCoords.x * aGlyph.x, texCoords.y * aGlyph.y) + aPosition) * uTxtScaling, 0.0f, 1.0f);
        texCoords = vec2(texCoords.x * aGlyph.x, texCoords.y * aGlyph.y) + aGlyph.zw;
    }

    #end(vShader)
    #begin(fShader)

    in vec2 texCoords;
    //
    uniform sampler2D uFontAtlas;
    //
    out vec4 fragColour;

    const float glyphWidth = 0.5f;
    const float glyphEdge = 0.1f;

    void SignedDistance()
    {
        float glyphDistance = texture(uFontAtlas, texCoords).r;
        fragColour = vec4(1.0f);
        fragColour.a = smoothstep(glyphWidth, glyphWidth + glyphEdge, glyphDistance);
    }

    void Bitmap()
    {
        fragColour = vec4(1.0f);
        fragColour.a = texture(uFontAtlas, texCoords).r;
    }

    void main()
    {
        Bitmap();
    }

    #end(fShader))";

    const char *ShaderSources::GetColourDiffuseGLSL() {
        return COLOUR_DIFFUSE_GLSL;
    }

    const char *ShaderSources::GetSolidColourGLSL() {
        return SOLID_COLOUR_GLSL;
    }

    const char *ShaderSources::GetTextGLSL() {
        return TEXT_GLSL;
    }
}
