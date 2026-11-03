#version 450

layout(location = 0) in vec3 vNormal;

layout(set = 0, binding = 0) uniform Material
{
    vec4 uLightPos; // xyz used
    vec4 uColour;   // rgb colour, a = opacity
} material;

layout(location = 0) out vec4 fragColour;

void main()
{
    vec3 unitLightVector = normalize(material.uLightPos.xyz);
    float diffuseAmount = clamp(dot(unitLightVector, vNormal), 0.1, 1.0);

    fragColour = vec4(material.uColour.rgb * diffuseAmount, material.uColour.a);
}
