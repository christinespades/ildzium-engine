#include "pch.h"
#include "renderer_debug_add.h"

extern Camera camera;
GizmoState g_gizmo_state = GIZMO_TRANSFORM;

uint32_t debug_line_count(
    const DebugPrimitiveCommand* cmd)
{
    switch(cmd->type)
    {
        case DEBUG_LINE:
            return 1;
        case DEBUG_BOX:
            return 12;
		case DEBUG_SPHERE:
		{
		    // Derive density dynamically from segments
		    uint32_t latBands = cmd->segments / 4;
		    uint32_t lonBands = cmd->segments / 2;
		    
		    if (latBands < 3) latBands = 3;
		    if (lonBands < 4) lonBands = 4;

		    // Total rings = (latBands - 1) + lonBands
		    return ((latBands - 1) + lonBands) * cmd->segments;
		}
		case DEBUG_CAPSULE:
		{
			// Dynamically derive detail levels to match the shader logic
		    uint32_t rings  = cmd->segments / 4;
		    uint32_t slices = cmd->segments / 8;
		    
		    // Clamp to minimum values so low-segment shapes don't collapse to 0
		    if (rings < 2)  rings = 2;
		    if (slices < 2) slices = 2;

		    return
		        cmd->segments +               // cylinder base circle A
		        cmd->segments +               // cylinder base circle B
		        cmd->segments +               // cylinder side links
		        (rings * cmd->segments) +     // bottom hemisphere rings
		        (slices * cmd->segments) +    // bottom hemisphere slices
		        (rings * cmd->segments) +     // top hemisphere rings
		        (slices * cmd->segments);     // top hemisphere slices
		}
        case DEBUG_CYLINDER:
            return cmd->segments * 3;
        case DEBUG_CONE:
            return cmd->segments * 2;
		case DEBUG_GRID:
		    return cmd->segments * 2;
        case DEBUG_GIZMO:
        {
            // Core Sphere lines
            uint32_t latBands = cmd->segments / 4;
            uint32_t lonBands = cmd->segments / 2;
            if (latBands < 3) latBands = 3;
            if (lonBands < 4) lonBands = 4;
            uint32_t sphereLines = ((latBands - 1) + lonBands) * cmd->segments;

            // 3 Cylinders (stems)
            uint32_t stemLines = 3 * (cmd->segments * 3);

            // Outer edges depend on GizmoState (passed in cmd->flags)
            uint32_t toolLines = 0;
            uint32_t state = cmd->flags; // GIZMO_TRANSFORM, GIZMO_ROTATE, GIZMO_SCALE
            if (state == GIZMO_TRANSFORM)
            {
                toolLines = 3 * (cmd->segments * 2); // 3 Cones
            }
            else if (state == GIZMO_ROTATE)
            {
                toolLines = 3 * cmd->segments;       // 3 Arcs (each has 'segments' lines)
            }
            else if (state == GIZMO_SCALE)
            {
                toolLines = 3 * 12;                  // 3 Boxes (each has 12 lines)
            }

            return sphereLines + stemLines + toolLines;
        }
        case DEBUG_ARROW:
            return cmd->segments * 5; // Cylinder (segments * 3) + Cone (segments * 2)
        default:
            return 0;
    }
}

static void submit_primitive(DebugPrimitiveCommand* cmd)
{
    if(g_renderer_debug.primitiveCount >= MAX_DEBUG_PRIMITIVES) return;
    cmd->lineStart = g_renderer_debug.generatedLineCount;
    cmd->lineCount = debug_line_count(cmd);
    if(g_renderer_debug.generatedLineCount + cmd->lineCount > MAX_DEBUG_LINES) return;
    g_renderer_debug.generatedLineCount += cmd->lineCount;
    g_renderer_debug.primitiveCPU[
        g_renderer_debug.primitiveCount++
    ] = *cmd;
}

void debug_add_line(
    float sx,float sy,float sz,
    float ex,float ey,float ez,
    float width,
    float r,float g,float b)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_LINE;
    cmd.p0_radius = (vec4){sx, sy, sz, 0.0f};
	cmd.p1_width = (vec4){ex, ey, ez, width};
	cmd.color = (vec4){r,g,b,1.0f};
    submit_primitive(&cmd);
}

void debug_add_grid(float width)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_GRID;
    cmd.p1_width.w = width;
    cmd.segments = get_param_float(PARAM_RENDER_DEBUG_GRID_EXTENT);
    cmd.color = get_param_vec4(PARAM_RENDER_DEBUG_GRID_COLOR);
    submit_primitive(&cmd);
}

void debug_add_gizmo(
    vec3 center,
    float scale,
    uint32_t segments,
    float width)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_GIZMO;
    // We can pass scale inside p0_radius.w
    cmd.p0_radius = (vec4){ center.x, center.y, center.z, scale };
    cmd.p1_width = (vec4){ width, width, width, width };
    cmd.segments = segments;
    cmd.flags = (uint32_t)g_gizmo_state; 
    // Gizmo uses standard RGB axis colors internally, color field can act as alpha / brightness modifier
    cmd.color = (vec4){ 1.0f, 1.0f, 1.0f, 1.0f }; 
    submit_primitive(&cmd);
}

void debug_add_arrow(
    vec3 start,
    vec3 end,
    float radius,
    uint32_t segments,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_ARROW;
    cmd.p0_radius = (vec4){ start.x, start.y, start.z, radius };
    cmd.p1_width = (vec4){ end.x, end.y, end.z, width };
    cmd.segments = segments;
    cmd.color = color;
    submit_primitive(&cmd);
}

void debug_add_box(
    vec3 center,
    vec3 size,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_BOX;
    cmd.p0_radius = (vec4){
        center.x,
        center.y,
        center.z,
        10.0f
    };
    cmd.p1_width = (vec4){
        size.x,
        size.y,
        size.z,
        width
    };
    cmd.color = color;
    submit_primitive(&cmd);
}

void debug_add_sphere(
    vec3 center,
    float radius,
    uint32_t segments,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_SPHERE;
    cmd.p0_radius = (vec4){
        center.x,
        center.y,
        center.z,
        radius
    };
    cmd.p1_width = (vec4){ width, width, width, width };
    cmd.segments = segments;
    cmd.color = color;
    submit_primitive(&cmd);
}

void debug_add_cylinder(
    vec3 start,
    vec3 end,
    float radius,
    uint32_t segments,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_CYLINDER;
    cmd.p0_radius = (vec4){
        start.x,
        start.y,
        start.z,
        radius
    };
    cmd.p1_width = (vec4){
        end.x,
        end.y,
        end.z,
        width
    };
    cmd.segments = segments;
    cmd.color = color;
    submit_primitive(&cmd);
}

void debug_add_cone(
    vec3 start,
    vec3 end,
    float radius,
    uint32_t segments,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_CONE;
    cmd.p0_radius = (vec4){
        start.x,
        start.y,
        start.z,
        radius
    };
    cmd.p1_width = (vec4){
        end.x,
        end.y,
        end.z,
        width
    };
    cmd.segments = segments;
    cmd.color = color;
    submit_primitive(&cmd);
}

void debug_add_capsule(
    vec3 start,
    vec3 end,
    float radius,
    uint32_t segments,
    float width,
    vec4 color)
{
    DebugPrimitiveCommand cmd = {0};
    cmd.type = DEBUG_CAPSULE;
    cmd.p0_radius = (vec4){
        start.x,
        start.y,
        start.z,
        radius
    };
    cmd.p1_width = (vec4){
        end.x,
        end.y,
        end.z,
        width
    };
    cmd.segments = segments;
    cmd.color = color;

    submit_primitive(&cmd);
}

#include "physics/capsule.h"

void renderer_debug_add_primitives()
{
    debug_add_grid(get_param_float(PARAM_RENDER_DEBUG_GRID_LINE_WIDTH));

    debug_add_gizmo(V3_BLACK, 20.0f, 16, 0.6f);

    debug_add_arrow((vec3){40.f,11.f,110.f}, (vec3){40.f, 221.f, 210.f}, 20.0f, 32, 0.5f, V4_RED);

    // TODO: make this project dependent and read from the project binary file. capsules should be parented to some meshes..
    for (uint32_t i = 0; i < g_comp_capsule_count; i++)
    {
        Capsule* cap = &g_comp_capsule[i];

		vec3 axis = quat_rotate(cap->rotation, (vec3){0,1,0});
		vec3 pos = vec4_xyz(cap->position_radius);
		vec3 half = vec3_scale(axis, cap->height * 0.5f);

		debug_add_capsule(
		    vec3_sub(pos, half),
		    vec3_add(pos, half),
		    cap->position_radius.w,
		    get_param_float(PARAM_RENDER_DEBUG_CAPSULE_SEGMENTS),
		    get_param_float(PARAM_RENDER_DEBUG_CAPSULE_LINE_WIDTH),
		    get_param_vec4(PARAM_RENDER_DEBUG_CAPSULE_COLOR));
    }
	debug_add_box(
	    (vec3){10.0f,10.0f,390.0f},
	    (vec3){10.0f,10.0f,10.0f},
	    get_param_float(PARAM_RENDER_DEBUG_BOX_LINE_WIDTH),
	    get_param_vec4(PARAM_RENDER_DEBUG_BOX_COLOR));

	debug_add_sphere(
	    (vec3){90.0f,310.0f,10.0f},
	    get_param_float(PARAM_RENDER_DEBUG_SPHERE_RADIUS),
	    get_param_float(PARAM_RENDER_DEBUG_SPHERE_SEGMENTS),
	    get_param_float(PARAM_RENDER_DEBUG_SPHERE_LINE_WIDTH),
	    get_param_vec4(PARAM_RENDER_DEBUG_SPHERE_COLOR));

	debug_add_cylinder(
	    (vec3){80.f,80.f,80.f},
	    (vec3){10.0f,10.0f,10.0f},
	    get_param_float(PARAM_RENDER_DEBUG_CYLINDER_RADIUS),
	    get_param_float(PARAM_RENDER_DEBUG_CYLINDER_SEGMENTS),
	    get_param_float(PARAM_RENDER_DEBUG_CYLINDER_LINE_WIDTH),
	    get_param_vec4(PARAM_RENDER_DEBUG_CYLINDER_COLOR));

	debug_add_cone(
	    (vec3){80.f,80.f,80.f},
	    (vec3){10.0f,10.0f,10.0f},
	    30.0f,
	    get_param_float(PARAM_RENDER_DEBUG_CONE_SEGMENTS),
	    get_param_float(PARAM_RENDER_DEBUG_CONE_LINE_WIDTH),
	    get_param_vec4(PARAM_RENDER_DEBUG_CONE_COLOR));
}