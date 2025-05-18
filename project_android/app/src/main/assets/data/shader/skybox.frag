#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// 6-faces skybox cubemap
layout (binding = 1) uniform samplerCube cubemapSampler;

// fragment input attributes
layout (location = 0) in vec3 inUVW;

// fragment output color
layout (location = 0) out vec4 outColor;

// entrypoint
void main() 
{
	outColor = texture(cubemapSampler, inUVW);
}