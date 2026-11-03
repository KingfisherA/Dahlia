#version 450

layout(location = 0) in vec3 aVertex;

layout(push_constant) uniform PushConstants
{
    mat4 uPVMatrix;
    mat4 uTransformMatrix;
} pc;

layout(location = 0) out vec3 vNormal;

void main()
{
    vec4 transformedV = pc.uTransformMatrix * vec4(aVertex, 1.0);
    vNormal = normalize(transformedV.xyz);
    gl_Position = pc.uPVMatrix * transformedV;
}
