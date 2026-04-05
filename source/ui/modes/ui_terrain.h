#pragma once
#include "ui/ui.h"

void setup_terrain_controls(UI_Context* ctx);

//  this will let you manipulate the terrain in the loaded area/map
// so sculpt, paint, inbuilt height lerps and stuff like that, level of detail/random noise gen for colors
// and for topology/dynomorph stuff