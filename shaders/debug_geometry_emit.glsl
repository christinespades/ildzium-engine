void emit_arc(
    uint lineStart,
    vec3 center,
    vec3 axisX,
    vec3 axisY,
    float radius,
    float startAngle,
    float endAngle,
    uint segments,
    float width,
    vec3 color)
{
    for(uint i=gl_LocalInvocationID.x;
        i<segments;
        i+=gl_WorkGroupSize.x)
    {
        float t0=float(i)/float(segments);
        float t1=float(i+1)/float(segments);

        float a0=mix(startAngle,endAngle,t0);
        float a1=mix(startAngle,endAngle,t1);

        vec3 p0=
            center+
            cos(a0)*radius*axisX+
            sin(a0)*radius*axisY;

        vec3 p1=
            center+
            cos(a1)*radius*axisX+
            sin(a1)*radius*axisY;

        write_line(
            lineStart+i,
            p0,
            p1,
            width,
            color);
    }
}

void emit_circle(
    uint lineStart,
    vec3 center,
    vec3 axisX,
    vec3 axisY,
    float radius,
    uint segments,
    float width,
    vec3 color)
{
    for(uint i = gl_LocalInvocationID.x;
        i < segments;
        i += gl_WorkGroupSize.x)
    {
        float a0 =
            float(i) /
            float(segments) *
            PI2;

        float a1 =
            float(i+1) /
            float(segments) *
            PI2;

        vec3 p0 =
            center +
            cos(a0)*radius*axisX +
            sin(a0)*radius*axisY;

        vec3 p1 =
            center +
            cos(a1)*radius*axisX +
            sin(a1)*radius*axisY;

        write_line(
            lineStart+i,
            p0,
            p1,
            width,
            color);
    }
}

void emit_cone(
    uint lineStart,
    vec3 base,
    vec3 tip,
    float radius,
    uint segments,
    float width,
    vec3 color)
{
    vec3 axis = normalize(tip - base);

    vec3 x, y;
    build_basis(axis, x, y);

    // 1. Draw the circular base (consumes 'segments' lines)
    emit_circle(
        lineStart,
        base,
        x,
        y,
        radius,
        segments,
        width,
        color);

    uint offset = lineStart + segments;

    // 2. Draw the lines from the circle edge to the tip (consumes 'segments' lines)
    for(uint i = gl_LocalInvocationID.x;
        i < segments;
        i += gl_WorkGroupSize.x)
    {
        float angle = float(i) / float(segments) * PI2;

        vec3 edge = base + (cos(angle) * x + sin(angle) * y) * radius;

        write_line(
            offset + i,
            edge,
            tip,
            width,
            color);
    }
}

void emit_cylinder(
    uint lineStart,
    vec3 a,
    vec3 b,
    float radius,
    uint segments,
    float width,
    vec3 color)
{
    vec3 axis=normalize(b-a);
    vec3 x,y;

    build_basis(axis,x,y);

    emit_circle(
        lineStart,
        a,
        x,
        y,
        radius,
        segments,
        width,
        color);

    lineStart+=segments;

    emit_circle(
        lineStart,
        b,
        x,
        y,
        radius,
        segments,
        width,
        color);

    lineStart+=segments;

    for(uint i=gl_LocalInvocationID.x;
        i<segments;
        i+=gl_WorkGroupSize.x)
    {
        float angle=
            float(i)/
            float(segments)*
            PI2;

        vec3 dir=
            cos(angle)*x+
            sin(angle)*y;

        write_line(
            lineStart+i,
            a+dir*radius,
            b+dir*radius,
            width,
            color);
    }
}

void emit_hemisphere(
    uint lineStart,
    vec3 center,
    vec3 axis, // Direction the dome points (e.g. forward or backward)
    float radius,
    uint rings,
    uint slices,
    uint segments,
    float width,
    vec3 color)
{
    vec3 x, y;
    build_basis(axis, x, y);

    // --- 1. Latitude Rings (Horizontal Circles) ---
    for(uint r = 0; r < rings; r++)
    {
        float t = float(r + 1) / float(rings);
        float angle = t * PI * 0.5;
        float rr = cos(angle) * radius;
        float h = sin(angle) * radius;

        emit_circle(
            lineStart,
            center + axis * h,
            x,
            y,
            rr,
            segments,
            width,
            color);

        lineStart += segments;
    }

    // --- 2. Longitude Slices (Vertical Arcs to Tip) ---
    for(uint s = 0; s < slices; s++)
    {
        float theta = (float(s) / float(slices)) * PI;
        
        // Calculate the direction vector for this specific slice wall
        vec3 sliceDir = cos(theta) * x + sin(theta) * y;

        // Emit an arc from 0 to PI/2 (from base edge up to the dome apex)
        emit_arc(
            lineStart,
            center,
            sliceDir, // Ground vector
            axis,     // Up vector (Apex)
            radius,
            0.0,
            PI * 0.5,
            segments,
            width,
            color);

        lineStart += segments;
    }
}

const ivec2 BOX_EDGES[12] = ivec2[](

    ivec2(0,1),
    ivec2(1,2),
    ivec2(2,3),
    ivec2(3,0),

    ivec2(4,5),
    ivec2(5,6),
    ivec2(6,7),
    ivec2(7,4),

    ivec2(0,4),
    ivec2(1,5),
    ivec2(2,6),
    ivec2(3,7)
);

void emit_box_edges(
    uint lineStart,
    vec3 center,
    vec3 extent,
    float width,
    vec3 color)
{
    uint edge = gl_LocalInvocationID.x;

    if(edge >= 12)
        return;

    vec3 c[8];

    vec3 mn = center - extent;
    vec3 mx = center + extent;

    c[0]=vec3(mn.x,mn.y,mn.z);
    c[1]=vec3(mx.x,mn.y,mn.z);
    c[2]=vec3(mx.x,mx.y,mn.z);
    c[3]=vec3(mn.x,mx.y,mn.z);

    c[4]=vec3(mn.x,mn.y,mx.z);
    c[5]=vec3(mx.x,mn.y,mx.z);
    c[6]=vec3(mx.x,mx.y,mx.z);
    c[7]=vec3(mn.x,mx.y,mx.z);

    ivec2 e = BOX_EDGES[edge];

    write_line(
        lineStart + edge,
        c[e.x],
        c[e.y],
        width,
        color);
}