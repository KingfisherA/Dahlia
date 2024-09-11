#version 460

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
//
layout(push_constant) uniform PushConstants {
    mat4 projection;
    vec3 lightPos;
} pcs;
//
layout(location = 0) out vec3 fNormal;

void main()
{
    fNormal = aNormal;
    gl_Position = pcs.projection * vec4(aPosition, 1.0f);
}

