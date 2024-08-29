#version 410

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

    //gl_Position = uPVMatrix * vec4(aVertex, 0.0f, 1.0f);
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

#end(fShader)
