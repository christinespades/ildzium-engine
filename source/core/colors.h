#pragma once
#include <math.h>
#include <stdint.h>
#include "core/time.h"

// Macro to build colors explicitly
#define ARGB(a, r, g, b) ((uint32_t)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)))

// Alpha reference
#define A_33 0x55
#define A_50 0x80
#define A_70 0xB4

#define COLOR_TRANSPARENT ARGB(0x00, 0x00, 0x00, 0x00)

// Grayscale
#define COLOR_BLACK_33 ARGB(A_33, 0x00, 0x00, 0x00)
#define COLOR_BLACK_50 ARGB(A_50, 0x00, 0x00, 0x00)
#define COLOR_BLACK_70 ARGB(A_70, 0x00, 0x00, 0x00)
#define COLOR_BLACK    ARGB(0xFF, 0x00, 0x00, 0x00)

#define COLOR_GRAY_33  ARGB(0xFF, 0x55, 0x55, 0x55)
#define COLOR_GRAY_50  ARGB(0xFF, 0x80, 0x80, 0x80)
#define COLOR_GRAY_70  ARGB(0xFF, 0xB4, 0xB4, 0xB4)

#define COLOR_WHITE_33 ARGB(A_33, 0xFF, 0xFF, 0xFF)
#define COLOR_WHITE_50 ARGB(A_50, 0xFF, 0xFF, 0xFF)
#define COLOR_WHITE_70 ARGB(A_70, 0xFF, 0xFF, 0xFF)
#define COLOR_WHITE    ARGB(0xFF, 0xFF, 0xFF, 0xFF)

// Red
#define COLOR_RED_33   ARGB(A_33, 0xFF, 0x00, 0x00)
#define COLOR_RED_50   ARGB(A_50, 0xFF, 0x00, 0x00)
#define COLOR_RED_70   ARGB(A_70, 0xFF, 0x00, 0x00)

// Green
#define COLOR_GREEN_33 ARGB(A_33, 0x00, 0xFF, 0x00)
#define COLOR_GREEN_50 ARGB(A_50, 0x00, 0xFF, 0x00)
#define COLOR_GREEN_70 ARGB(A_70, 0x00, 0xFF, 0x00)

// Blue
#define COLOR_BLUE_33  ARGB(A_33, 0x00, 0x00, 0xFF)
#define COLOR_BLUE_50  ARGB(A_50, 0x00, 0x00, 0xFF)
#define COLOR_BLUE_70  ARGB(A_70, 0x00, 0x00, 0xFF)

// Yellow
#define COLOR_YELLOW_33 ARGB(A_33, 0xFF, 0xFF, 0x00)
#define COLOR_YELLOW_50 ARGB(A_50, 0xFF, 0xFF, 0x00)
#define COLOR_YELLOW_70 ARGB(A_70, 0xFF, 0xFF, 0x00)

// Cyan
#define COLOR_CYAN_33  ARGB(A_33, 0x00, 0xFF, 0xFF)
#define COLOR_CYAN_50  ARGB(A_50, 0x00, 0xFF, 0xFF)
#define COLOR_CYAN_70  ARGB(A_70, 0x00, 0xFF, 0xFF)

// Magenta
#define COLOR_MAGENTA_33 ARGB(A_33, 0xFF, 0x00, 0xFF)
#define COLOR_MAGENTA_50 ARGB(A_50, 0xFF, 0x00, 0xFF)
#define COLOR_MAGENTA_70 ARGB(A_70, 0xFF, 0x00, 0xFF)

// Orange
#define COLOR_ORANGE_33 ARGB(A_33, 0xFF, 0xAA, 0x00)
#define COLOR_ORANGE_50 ARGB(A_50, 0xFF, 0xAA, 0x00)
#define COLOR_ORANGE_70 ARGB(A_70, 0xFF, 0xAA, 0x00)

// Helper linear interpolation macro for single bytes
#define LERP_BYTE(start, end, t) (uint8_t)((start) + (((end) - (start)) * (t)))

static inline uint32_t get_dynamic_color(void)
{
    // 1. Get current engine time in seconds. 
    extern double ildz_get_time(void); 
    float time = (float)ildz_get_time();

    // 2. Define our cycle speed (slow it down by dividing time)
    // Changing the denominator controls the overall speed of the shift
    float cycle = fmodf(time * 0.5f, 3.0f); 

    uint8_t a = 0xFF;
    uint8_t r = 0, g = 0, b = 0;

    // 3. Smoothly cycle between Red -> Green -> Blue -> Red
    if (cycle < 1.0f) {
        // Red to Green
        float t = cycle;
        r = LERP_BYTE(0xFF, 0x00, t);
        g = LERP_BYTE(0x00, 0xFF, t);
        b = 0x00;
    } 
    else if (cycle < 2.0f) {
        // Green to Blue
        float t = cycle - 1.0f;
        r = 0x00;
        g = LERP_BYTE(0xFF, 0x00, t);
        b = LERP_BYTE(0x00, 0xFF, t);
    } 
    else {
        // Blue back to Red
        float t = cycle - 2.0f;
        r = LERP_BYTE(0x00, 0xFF, t);
        g = 0x00;
        b = LERP_BYTE(0xFF, 0x00, t);
    }

    // Pack back specifically using your ABGR macro footprint
    return ARGB(a, r, g, b);
}

#define COLOR_RGB_DYNAMIC get_dynamic_color()