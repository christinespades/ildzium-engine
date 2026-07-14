void process_line(Primitive p)
{
    if(gl_LocalInvocationID.x != 0)
        return;

    write_line(
        p.lineStart,
        p.p0_radius.xyz,
        p.p1_width.xyz,
        p.p1_width.w,
        p.color.rgb);
}

void process_box(Primitive p)
{
    emit_box_edges(
        p.lineStart,
        p.p0_radius.xyz,
        p.p1_width.xyz,
        p.p1_width.w,
        p.color.rgb);
}

void process_sphere(Primitive p)
{
    // Dynamically scale wireframe density matching the CPU allocation
    uint latitudeBands = p.segments / 4;
    uint longitudeBands = p.segments / 2;
    
    if (latitudeBands < 3) latitudeBands = 3;
    if (longitudeBands < 4) longitudeBands = 4;

    vec3 center = p.p0_radius.xyz;
    float radius = p.p0_radius.w;
    uint offset = p.lineStart;

    for(uint ring=1; ring<latitudeBands; ring++)
    {
        float phi = float(ring) / float(latitudeBands) * PI;
        float rr = radius * sin(phi);
        float rz = center.z + radius * cos(phi);

        emit_circle(offset, vec3(center.x, center.y, rz), vec3(1,0,0), vec3(0,1,0), rr, p.segments, p.p1_width.w, p.color.rgb);
        offset += p.segments;
    }

    for(uint ring=0; ring<longitudeBands; ring++)
    {
        float theta = float(ring) / float(longitudeBands) * PI;

        emit_circle(offset, center, vec3(cos(theta), sin(theta), 0), vec3(0,0,1), radius, p.segments, p.p1_width.w, p.color.rgb);
        offset += p.segments;
    }
}

void process_grid(Primitive p)
{
    uint size = p.segments * 2;
    float spacing = 20.0;
    uint halfSize = p.segments;
    float extent = (float(halfSize) * 0.5) * spacing;

    for(uint i = gl_LocalInvocationID.x;
        i < size;
        i += gl_WorkGroupSize.x)
    {
        if(i < halfSize)
        {
            float z =
                (float(i)-float(halfSize)/2.0)
                * spacing;

            write_line(
                p.lineStart+i,
                vec3(-extent,0,z),
                vec3(extent,0,z),
                p.p1_width.w,
                p.color.xyz);
        }
        else
        {
            float x =
                (float(i-halfSize)-float(halfSize)/2.0)
                * spacing;

            write_line(
                p.lineStart+i,
                vec3(x,0,-extent),
                vec3(x,0,extent),
                p.p1_width.w,
                p.color.xyz);
        }
    }
}

void process_capsule(Primitive p)
{
    vec3 start = p.p0_radius.xyz;
    vec3 end = p.p1_width.xyz;
    float radius = p.p0_radius.w;

    vec3 axis = normalize(end - start);
    float len = distance(start, end);

    vec3 center = (start + end) * 0.5;
    vec3 a = center - axis * (len * 0.5);
    vec3 b = center + axis * (len * 0.5);

    uint offset = p.lineStart;

    // 1. Cylinder core (consumes 3 * p.segments)
    emit_cylinder(offset, a, b, radius, p.segments, p.p1_width.w, p.color.rgb);
    offset += p.segments * 3;

    // DYNAMIC DETAIL: Scale density with segment configurations
    uint rings  = p.segments / 4;
    uint slices = p.segments / 8;
    
    // Maintain safety bounds identical to CPU limits
    if (rings < 2)  rings = 2;
    if (slices < 2) slices = 2;

    // 2. Bottom Cap
    emit_hemisphere(offset, a, -axis, radius, rings, slices, p.segments, p.p1_width.w, p.color.rgb);
    offset += (rings + slices) * p.segments;

    // 3. Top Cap
    emit_hemisphere(offset, b, axis, radius, rings, slices, p.segments, p.p1_width.w, p.color.rgb);
}

void process_cylinder(Primitive p)
{
    vec3 a = p.p0_radius.xyz;
    vec3 b = p.p1_width.xyz;

    emit_cylinder(
        p.lineStart,
        a,
        b,
        p.p0_radius.w,
        p.segments,
        p.p1_width.w,
        p.color.rgb);
}

void process_cone(Primitive p)
{
    emit_cone(
        p.lineStart,
        p.p0_radius.xyz, // base
        p.p1_width.xyz,  // tip
        p.p0_radius.w,   // radius
        p.segments,
        p.p1_width.w,    // line width
        p.color.rgb
    );
}

void process_gizmo(Primitive p)
{
    vec3 center = p.p0_radius.xyz;
    float scale = p.p0_radius.w;
    float width = p.p1_width.w;
    uint segments = p.segments;
    uint state = p.flags;

    // Axis Directions
    vec3 axes[3] = vec3[](
        vec3(1.0, 0.0, 0.0), // X
        vec3(0.0, 1.0, 0.0), // Y
        vec3(0.0, 0.0, 1.0)  // Z
    );

    // Axis Colors (Red, Green, Blue)
    vec3 colors[3] = vec3[](
        vec3(1.0, 0.1, 0.1),
        vec3(0.1, 1.0, 0.1),
        vec3(0.1, 0.1, 1.0)
    );

    uint offset = p.lineStart;

    // 1. Render Core Sphere
    uint latitudeBands = segments / 4;
    uint longitudeBands = segments / 2;
    if (latitudeBands < 3) latitudeBands = 3;
    if (longitudeBands < 4) longitudeBands = 4;
    
    // Core sphere is small (15% of scale) and gray
    float sphereRadius = scale * 0.15; 
    vec3 coreColor = vec3(0.6);

    for(uint ring = 1; ring < latitudeBands; ring++)
    {
        float phi = float(ring) / float(latitudeBands) * PI;
        float rr = sphereRadius * sin(phi);
        float rz = center.z + sphereRadius * cos(phi);

        emit_circle(offset, vec3(center.x, center.y, rz), vec3(1,0,0), vec3(0,1,0), rr, segments, width, coreColor);
        offset += segments;
    }
    for(uint ring = 0; ring < longitudeBands; ring++)
    {
        float theta = float(ring) / float(longitudeBands) * PI;
        emit_circle(offset, center, vec3(cos(theta), sin(theta), 0), vec3(0,0,1), sphereRadius, segments, width, coreColor);
        offset += segments;
    }

    // 2. Render Stems (Cylinders)
    float stemRadius = scale * 0.025;
    float stemLength = scale * 0.75;

    for (uint i = 0; i < 3; ++i)
    {
        vec3 endPoint = center + axes[i] * stemLength;
        emit_cylinder(offset, center, endPoint, stemRadius, segments, width, colors[i]);
        offset += segments * 3;
    }

    // 3. Render Outer Edges (Cones, Arcs, or Boxes)
    if (state == 0) // GIZMO_TRANSFORM
    {
        float coneRadius = scale * 0.08;
        float coneLength = scale * 0.25;

        for (uint i = 0; i < 3; ++i)
        {
            vec3 startPoint = center + axes[i] * stemLength;
            vec3 endPoint = center + axes[i] * (stemLength + coneLength);
            emit_cone(offset, startPoint, endPoint, coneRadius, segments, width, colors[i]);
            offset += segments * 2;
        }
    }
    else if (state == 1) // GIZMO_ROTATE
    {
        // 3 Arcs corresponding to local rotation planes (XY, YZ, ZX)
        float arcRadius = scale * 0.85;

        // XY plane rotation arc (Around Z)
        emit_arc(offset, center, axes[0], axes[1], arcRadius, 0.0, PI * 0.5, segments, width, colors[2]); 
        offset += segments;

        // YZ plane rotation arc (Around X)
        emit_arc(offset, center, axes[1], axes[2], arcRadius, 0.0, PI * 0.5, segments, width, colors[0]); 
        offset += segments;

        // ZX plane rotation arc (Around Y)
        emit_arc(offset, center, axes[2], axes[0], arcRadius, 0.0, PI * 0.5, segments, width, colors[1]); 
        offset += segments;
    }
    else if (state == 2) // GIZMO_SCALE
    {
        float boxHalfSize = scale * 0.05;

        for (uint i = 0; i < 3; ++i)
        {
            vec3 boxCenter = center + axes[i] * (stemLength + boxHalfSize);
            emit_box_edges(offset, boxCenter, vec3(boxHalfSize), width, colors[i]);
            offset += 12;
        }
    }
}

void process_arrow(Primitive p)
{
    vec3 start = p.p0_radius.xyz;
    vec3 end = p.p1_width.xyz;
    float radius = p.p0_radius.w;
    float width = p.p1_width.w;
    vec3 color = p.color.rgb;

    vec3 dir = end - start;
    float len = length(dir);
    if (len < 0.0001) return;
    vec3 axis = dir / len;

    // Allocate 80% of length for stem, 20% for the cone tip
    float splitFactor = 0.8;
    vec3 mid = start + axis * (len * splitFactor);

    uint offset = p.lineStart;

    // 1. Stem (Cylinder) -> Consumes (segments * 3) lines
    emit_cylinder(offset, start, mid, radius, p.segments, width, color);
    offset += p.segments * 3;

    // 2. Tip (Cone) -> Consumes (segments * 2) lines
    // Base of cone starts at the stem end, flared out to double the radius
    emit_cone(offset, mid, end, radius * 2.0, p.segments, width, color);
}