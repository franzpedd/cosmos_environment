#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// this is here while I don't create separate descriptors for picking render pass
layout(set = 0, binding = 2) uniform sampler2D colorMapSampler;

// fragment output color
layout(location = 0) out uvec2 outColor;

// entrypoint
void main()
{
    // we're going to separate our uint64_t into a vec2 of float, latter on CPU code we're going to read it back
    // extract lower 32 bits
    uint lowerBits = uint(pushConstant.id & 0xFFFFFFFFUL);

    // extract upper 32 bits
    uint upperBits = uint(pushConstant.id >> 32);

    // convert to uvec2 by packing the parts as floats
    outColor = uvec2(uint(lowerBits), uint(upperBits));
}