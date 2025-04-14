#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/primitives.glsl"
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// entrypoint
void main()
{
    // set vertex position on world
    gl_Position = camera.proj * camera.view * pushConstant.model * vec4(SquareVertices[gl_VertexIndex].xyz, 1.0);
}