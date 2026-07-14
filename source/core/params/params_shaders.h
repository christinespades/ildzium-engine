#pragma once

#define SHADERS_SKY_AURORA_COLOR (vec3){0.35f, 0.9f, 0.75f}
#define SHADERS_SKY_AURORA_POWER 0.11f
#define SHADERS_SKY_AURORA_SPEED 1.1f
#define SHADERS_SKY_BRIGHTNESS 0.2f
#define SHADERS_SKY_DAY_NIGHT_CYCLE_SPEED 0.25f
#define SHADERS_SKY_NEBULA_SPEED 1.0f
#define SHADERS_SKY_NEBULA_SCALE 2.8f
#define SHADERS_SKY_NEBULA_POWER 12.6f
#define SHADERS_SKY_NEBULA_LAYERS -4.0f
#define SHADERS_SKY_NEBULA_COLOR_DAY (vec3){0.6f, 0.3f, 0.9f}
#define SHADERS_SKY_NEBULA_COLOR_NIGHT (vec3){0.2f, 0.7f, 1.0f}
#define SHADERS_SKY_STAR_COUNT 0.01f
#define SHADERS_SKY_STAR_POWER 0.7f
#define SHADERS_SKY_STAR_TWINKLE_SPEED 0.2f
#define SHADERS_SKY_STAR_SIZE 0.0012f
#define SHADERS_SKY_TIME_OF_DAY 0.3f
#define SHADERS_SKY_VERTICAL_BAND_SPEED 1.0f
#define SHADERS_SKY_VERTICAL_BAND_FREQUENCY 7.0f
#define SHADERS_SKY_VIGNETTE_POWER 0.4f

#define SHADERS_SKY_PARAMS_MAP \
    X(f, SHADERS_SKY_AURORA_POWER, "Aurora Power", -20.0f, 20.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_AURORA_SPEED, "Aurora Speed", 0.0f, 5.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_BRIGHTNESS, "Brightness", 0.0f, 5.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_DAY_NIGHT_CYCLE_SPEED, "Day/Night Cycle Speed", -30.0f, 30.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_NEBULA_POWER, "Nebula Power", -20.0f, 20.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_NEBULA_LAYERS, "Nebula Layers", -20.0f,   20.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_NEBULA_SPEED, "Nebula Speed", -5.0f, 5.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_NEBULA_SCALE, "Nebula Scale", -5.0f, 5.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_STAR_POWER, "Star Power", 0.0f, 5.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_STAR_COUNT, "Star Count", 0.0f, 25.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_STAR_TWINKLE_SPEED, "Star Twinkle Speed", -2.0f,   2.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_TIME_OF_DAY, "Time of Day", 0.0f, 1.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_VERTICAL_BAND_SPEED, "Vertical Band Speed", -10.0f, 10.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_VERTICAL_BAND_FREQUENCY, "Vertical Band Frequency", -90.0f, 90.0f, tooltip_EMPTY)  \
    X(f, SHADERS_SKY_VIGNETTE_POWER, "Vignette Power", -12.5f, 12.5f, tooltip_EMPTY)

#define SHADERS_SKY_PARAMS_V3_MAP \
    X(v3, SHADERS_SKY_AURORA_COLOR, "Aurora Color", V3_BLACK, V3_WHITE, tooltip_EMPTY)  \
    X(v3, SHADERS_SKY_NEBULA_COLOR_DAY, "Nebula Color Day", V3_BLACK, V3_WHITE, tooltip_EMPTY)  \
    X(v3, SHADERS_SKY_NEBULA_COLOR_NIGHT, "Nebula Color Night", V3_BLACK, V3_WHITE, tooltip_EMPTY)