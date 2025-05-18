#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// mesh samplers
layout(set = 0, binding = 2) uniform sampler2D colorMapSampler;

// input fragment attributes
layout(location = 0) in vec2 inFragTexCoord;
layout(location = 1) in vec3 inFragNormal;

// output fragment color
layout(location = 0) out vec4 outColor;

// entrypoint
void main()
{
    outColor = texture(colorMapSampler, inFragTexCoord);

    // discard full transparent pixels
    if(outColor.a == 0.0) {
        discard;
    }
}