#version 410

#pragma remove
#include "GLSL Language Specification.h.glsl"
#pragma endremove

#begin(vShader)

layout(location = 0) in vec2 aVertex;
//
uniform mat4 uPVMatrix;
uniform vec2 uSize = vec2(1.0f, 1.0f);
//
out vec2 texCoords;

void main()
{
    texCoords = (aVertex + vec2(1.0f)) * 0.5f;
    gl_Position = uPVMatrix * vec4(aVertex.x * uSize.x, aVertex.y * uSize.y, 0.0f, 1.0f);
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
