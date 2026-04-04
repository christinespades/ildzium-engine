#pragma once
#include <stdio.h>    // FILE, fopen, fread, fseek, ftell, fclose, printf
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "ui.h"

void on_skybox_settings_button_clicked(void);
void on_skybox_settings_button_released(void);
void setup_skybox_controls(UI_Context* ctx);