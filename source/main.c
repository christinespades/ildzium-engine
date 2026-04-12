#include "pch.h"
#include "main.h"
#include "core/debug.h"
#include "core/exceptions.h"
#include "core/platform.h"
#include "core/watcher.h"
#include "input/input.h"
#include "rendering/surface.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "ui/ui.h"

static int g_target_fps = 60;
void draw_frame();

double lastTime;
double last_render_time;
double frame_duration;

void init()
{
    #ifndef __EMSCRIPTEN__
        AddVectoredExceptionHandler(1, VectoredHandler);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
        HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        SymInitialize(process, "srv*C:\\symbols*https://msdl.microsoft.com/download/symbols", TRUE);
        watcher_init();
    #endif

    init_platform();
    init_camera();
    lastTime = platform_get_time();
    last_render_time = lastTime;
    frame_duration = 1.0 / (double)g_target_fps;
}

void engine_tick()
{
#ifndef __EMSCRIPTEN__
    _CrtCheckMemory();
    watcher_update();
    glfwPollEvents();
#endif
    double currentTime = platform_get_time();
    float deltaTime = (float)(currentTime - lastTime);
    lastTime = currentTime;
    update_camera(deltaTime);
    int left_pressed = platform_get_mouse_button(0);

    if (g_ui_ctx->cursor_captured) {
        double mx, my;
        platform_get_mouse_pos(&mx, &my);

        int win_w, win_h;
        platform_get_window_size(&win_w, &win_h);

        if (mx < 0) mx = 0;
        if (my < 0) my = 0;
        if (mx >= win_w) mx = win_w - 1;
        if (my >= win_h) my = win_h - 1;

        ui_update(g_ui_ctx, (int)mx, (int)my, left_pressed, mouse_wheel, deltaTime);
        mouse_wheel = 0.0;
    }

    currentTime = platform_get_time();

    if ((currentTime - last_render_time) >= (1.0 / g_target_fps))
    {
        last_render_time = currentTime;
    #ifdef __EMSCRIPTEN__
        webgpu_draw();
    #else    
        vulkan_draw();
    #endif
    }
}

int main()
{
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(engine_tick, 0, 1);
#else
    while (!platform_should_close())
    {
        engine_tick();
    }

    watcher_cleanup();
#endif

    ui_cleanup(g_ui_ctx);
    free(g_ui_ctx);
    platform_shutdown();

    return 0;
}