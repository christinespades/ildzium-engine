#pragma once

#define ENGINE_WINDOW_SIZE_PERCENTAGE_OF_MONITOR 75
#define ENGINE_WINDOW_BORDERLESS false

#define ENGINE_PARAMS_MAP \
    X(b, ENGINE_WINDOW_BORDERLESS, "Engine Main Window Borderless", false, true, tooltip_EMPTY) \
    X(u, ENGINE_WINDOW_SIZE_PERCENTAGE_OF_MONITOR, "Engine Main Window Size Percentage of Monitor at Startup", 1, 100, tooltip_EMPTY)
