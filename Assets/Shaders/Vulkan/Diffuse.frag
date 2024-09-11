#version 460

layout(location = 0) in vec3 fNormal;
//
layout(push_constant) uniform PushConstants {
    mat4 projection;
    vec3 lightPos;
} pcs;
//const vec3 lightPos = vec3(-1.0f, -1.0f, 0.7f);
//
layout(location = 0) out vec4 fragColour;

void main()
{
    float diffuse = clamp(dot(fNormal, normalize(pcs.lightPos)), 0.005f, 1.0f);
    fragColour = vec4(1.0f);
    fragColour.rgb *= diffuse;
}
