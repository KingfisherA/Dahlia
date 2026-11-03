#version 450

layout(location = 0) in vec2 aVertex;

layout(push_constant) uniform PushConstants
{
    mat4 uPVMatrix;
    mat4 uTransformMatrix;
} pc;

void main()
{
    gl_Position = pc.uPVMatrix * pc.uTransformMatrix * vec4(aVertex, 0.0, 1.0);
}
