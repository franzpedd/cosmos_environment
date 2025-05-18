// this is included per mesh/drawable and contains information about the material

layout(set = 0, binding = 1) uniform ubo_mesh
{
    float uv_rotation;
    float stupid_padding;
    vec2 uv_offset;
    vec2 uv_scale;
} meshParams;