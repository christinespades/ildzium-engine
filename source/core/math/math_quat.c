#include "pch.h"
#include "core/math/math_quat.h"

// assumes quat is normalized
vec3 quat_rotate(vec4 q, vec3 v)
{
    vec3 qv = { q.x, q.y, q.z };
    vec3 t = vec3_scale(vec3_cross(qv, v), 2.0f);

    return vec3_add(
        vec3_add(v, vec3_scale(t, q.w)),
        vec3_cross(qv, t));
}

// assumes rotation.x = qx
// rotation.y = qy
// rotation.z = qz
// rotation.w = qw
vec4 quat_from_axis_angle(vec3 axis, float angle)
{
    float half = angle * 0.5f;
    float s = sinf(half);

    return (vec4){
        axis.x * s,
        axis.y * s,
        axis.z * s,
        cosf(half)
    };
}