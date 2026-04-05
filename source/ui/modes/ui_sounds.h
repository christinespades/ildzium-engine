#pragma once
#include "ui/ui.h"

void setup_sounds_controls(UI_Context* ctx);

// this context will enable visualization for 3d sound sources
// so you get gizmos, you select one and get an outline too
// and the params change and affect the one source you select
// so you can adjust the panning, volume, filepath, etc..
// you also get sound controls for the currently loaded tile/area/map
// so you can have a playlist of songs just for a specific location
// or background ambience
// probabilities/variables triggering music etc, specific rules for areas