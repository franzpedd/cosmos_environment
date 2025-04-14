#ifndef CREN_DEFINES_INCLUDED
#define CREN_DEFINES_INCLUDED

/// @brief How many descriptors sets at max a layout binding may have
#define CREN_PIPELINE_DESCRIPTOR_SET_LAYOUT_BINDING_MAX 32

/// @brief How many push constants at max may exist for a given Pipeline
#define CREN_PIPELINE_PUSH_CONSTANTS_MAX 8

/// @brief How many shader stages a pipeline may have, since we only support Vertex and Fragment for now, 2
#define CREN_PIPELINE_SHADER_STAGES_COUNT 2

/// @brief The quad's default pipeline name, used for hashtable look-ups
#define CREN_PIPELINE_QUAD_DEFAULT_NAME "Quad:Default"

/// @brief The quad's picking pipeline name, used for hashtable look-ups
#define CREN_PIPELINE_QUAD_PICKING_NAME "Quad:Picking"

#endif // CREN_DEFINES_INCLUDED