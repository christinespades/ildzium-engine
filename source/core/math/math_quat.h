#pragma once

vec3 quat_rotate(vec4 q, vec3 v);
vec4 quat_from_axis_angle(vec3 axis, float angle);