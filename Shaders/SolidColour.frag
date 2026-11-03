#version 450

layout(set = 0, binding = 0) uniform Material
{
    vec4 uColour; // rgb colour, a = opacity
} material;

layout(location = 0) out vec4 fragColour;

void main()
{
    fragColour = material.uColour;
}
