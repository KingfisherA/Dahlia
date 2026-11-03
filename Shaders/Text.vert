#version 450

// Per vertex: unit quad corner in [-1, 1]
layout(location = 0) in vec2 aVertex;
// Per instance (one glyph): xy = glyph size in the atlas, zw = glyph offset in the atlas
layout(location = 1) in vec4 aGlyph;
// Per instance: glyph position in text space
layout(location = 2) in vec2 aPosition;

layout(push_constant) uniform PushConstants
{
    mat4 uPVMatrix;
    float uTxtScaling;
} pc;

layout(location = 0) out vec2 vTexCoords;

void main()
{
    vec2 corner = (aVertex + vec2(1.0)) * 0.5;
    vec2 glyphExtent = corner * aGlyph.xy;

    gl_Position = pc.uPVMatrix * vec4((glyphExtent + aPosition) * pc.uTxtScaling, -1.0, 1.0);
    vTexCoords = glyphExtent + aGlyph.zw;
}
