#pragma once

void init_platform();
void platform_shutdown();
int  platform_should_close();
double platform_get_time();
void platform_get_window_size(int* w, int* h);