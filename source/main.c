#include "pch.h"
#include "main.h"
#include "rendering/renderer_webgpu_draw.h"
static int g_target_fps = 60;
double lastTime;
double last_render_time;
double frame_duration;
float g_fps = 0.0f;

void init()
{
    #ifndef __EMSCRIPTEN__
        AddVectoredExceptionHandler(1, VectoredHandler);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
        HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        SymInitialize(process, "srv*C:\\symbols*https://msdl.microsoft.com/download/symbols", TRUE);
        watcher_init();
        init_window();
        create_vk_instance();
        create_vk_surface();
        vulkan_init();
    #else
        init_window();
        webgpu_init();
    #endif

    init_input();
    g_ui_ctx = malloc(sizeof(UI_Context));
    ui_init(g_ui_ctx);
    init_camera();
    lastTime = ildz_get_time();
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
    double currentTime = ildz_get_time();
    float deltaTime = (float)(currentTime - lastTime);
    lastTime = currentTime;
    update_camera(deltaTime);
    int left_pressed = platform_get_mouse_button(0);

    if (g_ui_ctx->cursor_captured) {
        //printf("Cursor captured\n");

        double mx, my;
        platform_get_mouse_pos(&mx, &my);

        int win_w, win_h;
        ildz_get_window_size(&win_w, &win_h);

        if (mx < 0) mx = 0;
        if (my < 0) my = 0;
        if (mx >= win_w) mx = win_w - 1;
        if (my >= win_h) my = win_h - 1;

        ui_update(g_ui_ctx, (int)mx, (int)my, left_pressed, mouse_wheel, deltaTime);
        mouse_wheel = 0.0;
    }

#ifdef __EMSCRIPTEN__
    webgpu_draw(deltaTime);
#else  
    if ((currentTime - last_render_time) >= (1.0 / g_target_fps))
    {
        last_render_time = currentTime;  
        vulkan_draw();
    }
#endif
}

int main()
{
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(engine_tick, 0, 1);   // 0 = use requestAnimationFrame
#else
    while (!ildz_should_close())
    {
        engine_tick();
    }

    watcher_cleanup();
#endif

    ui_cleanup(g_ui_ctx);
    free(g_ui_ctx);
    
#ifndef __EMSCRIPTEN__
    vulkan_glfw_shutdown();
#endif
    return 0;
}