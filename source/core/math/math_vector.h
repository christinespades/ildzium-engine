#pragma once

static inline vec3 vec4_xyz(vec4 v)
{
    return (vec3){ v.x, v.y, v.z };
}

static inline vec3 vec3_cross(vec3 a, vec3 b)
{
    return (vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline vec3 vec3_add(vec3 a, vec3 b)
{
    return (vec3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

static inline vec3 vec3_sub(vec3 a, vec3 b)
{
    return (vec3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

static inline vec3 vec3_scale(vec3 v, float s)
{
    return (vec3){
        v.x * s,
        v.y * s,
        v.z * s
    };
}

static inline float vec3_length(vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline vec3 vec3_normalize(vec3 v)
{
    float len = vec3_length(v);

    if (len < 0.000001f)
        return (vec3){0,0,0};

    float inv = 1.0f / len;

    return (vec3){
        v.x * inv,
        v.y * inv,
        v.z * inv
    };
}