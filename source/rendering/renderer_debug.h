#pragma once
#include "core/params/params.h"

typedef struct
{
    float x;
    float y;
} DebugQuadVertex;

static const DebugQuadVertex debug_quad_vertices[4] =
{
    {-1.0f, -1.0f},
    { 1.0f, -1.0f},
    {-1.0f,  1.0f},
    { 1.0f,  1.0f}
};

typedef struct DebugLineInstance
{
    vec4 start;
    vec4 end;
    vec4 color;
    vec4 padding;
} DebugLineInstance;

#define MAX_DEBUG_PRIMITIVES 16384
#define MAX_DEBUG_LINES      262144

enum DebugPrimitiveType
{
    DEBUG_LINE,
    DEBUG_BOX,
    DEBUG_SPHERE,
    DEBUG_GRID,
    DEBUG_CAPSULE,
    DEBUG_CYLINDER,
    DEBUG_CONE,
    DEBUG_GIZMO,
    DEBUG_ARROW,
};

typedef enum GizmoState
{
    GIZMO_TRANSFORM = 0,
    GIZMO_ROTATE    = 1,
    GIZMO_SCALE     = 2
} GizmoState;
extern GizmoState g_gizmo_state;

typedef struct DebugPrimitiveCommand
{
    uint32_t type;
    uint32_t _pad0[3];
    vec4 p0_radius;
    vec4 p1_width;
    vec4 direction_axisLength;
    vec4 color;
    uint32_t segments;
    uint32_t lineStart;
    uint32_t lineCount;
    uint32_t flags;
} DebugPrimitiveCommand;

typedef struct
{
    VkPipeline pipeline;
    // Static quad
    VkBuffer quadVertexBuffer;
    VkDeviceMemory quadVertexMemory;
    // Per frame buffers
    VkBuffer instanceBuffers[RENDER_MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory instanceMemory[RENDER_MAX_FRAMES_IN_FLIGHT];
    void* mappedInstanceBuffers[RENDER_MAX_FRAMES_IN_FLIGHT];
	VkBuffer primitiveBuffers[RENDER_MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory primitiveMemory[RENDER_MAX_FRAMES_IN_FLIGHT];
	void* mappedPrimitiveBuffers[RENDER_MAX_FRAMES_IN_FLIGHT];

	VkShaderModule computeModule;
	VkPipeline computePipeline;
	VkPipelineLayout computePipelineLayout;
	VkDescriptorSetLayout computeSetLayout;
	VkDescriptorPool computeDescriptorPool;
	VkDescriptorSet computeDescriptorSets[RENDER_MAX_FRAMES_IN_FLIGHT];

	DebugPrimitiveCommand primitiveCPU[MAX_DEBUG_PRIMITIVES];
	uint32_t primitiveCount;
	uint32_t generatedLineCount;
} DebugRenderer;

// Global or system instance
extern DebugRenderer g_renderer_debug;
