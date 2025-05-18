#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// output vertex attributes
layout(location = 0) out vec2 outFragTexCoord;
layout(location = 1) out vec3 outFragNormal;

// entrypoint
void main()
{
    // set vertex position on world
    gl_Position = camera.proj * camera.view * pushConstant.model * vec4(inPosition, 1.0);

    // output variables for the fragment shader
    outFragTexCoord = inTexCoord;
    outFragNormal = mat3(pushConstant.model) * inNormal; // transform normal to world space
}