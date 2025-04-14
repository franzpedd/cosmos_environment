// this is defined per object/drawable and contains unique information about an object

layout(push_constant) uniform constants
{
    uint64_t id;
	mat4 model;
} pushConstant;