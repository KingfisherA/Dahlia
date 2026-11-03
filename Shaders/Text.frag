#version 450

layout(location = 0) in vec2 vTexCoords;

layout(set = 0, binding = 0) uniform sampler2D uFontAtlas;

layout(location = 0) out vec4 fragColour;

const float GLYPH_WIDTH = 0.5;
const float GLYPH_EDGE = 0.1;

vec4 SignedDistance()
{
    float glyphDistance = texture(uFontAtlas, vTexCoords).r;
    return vec4(1.0, 1.0, 1.0, smoothstep(GLYPH_WIDTH, GLYPH_WIDTH + GLYPH_EDGE, glyphDistance));
}

vec4 Bitmap()
{
    return vec4(1.0, 1.0, 1.0, texture(uFontAtlas, vTexCoords).r);
}

void main()
{
    fragColour = Bitmap();
}
