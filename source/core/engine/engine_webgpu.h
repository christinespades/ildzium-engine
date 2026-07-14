#pragma once
#include "core/veh.h"
#include "core/time.h"
#include "core/project.h"
#include "input/input.h"
#include "rendering/surface.h"
#include "scene/camera.h"
#include "scene/model_webgpu.h"
#include "rendering/renderer_webgpu_draw.h"
#include "ui/ui.h"
#include "scene/camera.h"
#include "core/params/params.h"
#include "physics/physics.h"
#include "input/input_poll.h"

void engine_init();
void engine_tick();