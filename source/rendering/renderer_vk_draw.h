#pragma once
#include "core/params/params.h"
#include "core/window/window_vk.h"
#include "rendering/device.h"
#include "rendering/fps.h"
#include "rendering/renderer_debug_vk.h"
#include "scene/camera.h"
#include "scene/lights_vk.h"
#include "scene/model_vk.h"
#include "scene/model_draw_vk.h"
#include "scene/sky_vk.h"
#include "ui/ui.h"
#include "ui/ui_draw.h"
#include "ui/ui_renderer_vk.h"

void vulkan_draw();