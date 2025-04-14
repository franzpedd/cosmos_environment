#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/primitives.glsl"
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// output vertex attributes
layout(location = 0) out vec3 outUVW;

// entrypoint
void main() 
{
    // get the correct vertex position
    vec3 vertexPosition = CubeVertices[CubeIndices[gl_VertexIndex]];

    // remove translation from the view
    vec3 position = mat3(pushConstant.model * camera.view) * vertexPosition;

    // get the correct position
    gl_Position = (camera.proj * vec4( position, 0.0 )).xyzz;

    // pass the vertex coordinates
    outUVW = vertexPosition;
}