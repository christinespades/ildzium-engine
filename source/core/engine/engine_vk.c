#include "pch.h"
#ifndef __EMSCRIPTEN__
	#include "engine_vk.h"

	double lastTickTime;

	void engine_tick()
	{
	    _CrtCheckMemory();
	    watcher_update();
	    glfwPollEvents();
	    input_poll();
	    double currentTickTime = ildz_get_time();
	    g_dt = (float)(currentTickTime - lastTickTime);
	    lastTickTime = currentTickTime;
	    update_camera();
	    update_models(lastTickTime);
	    int left_pressed = platform_get_mouse_button(0);
	    if (g_ui_ctx->cursor_captured) {
	        // LOGI("Cursor captured");
	        ui_update(g_ui_ctx, left_pressed, mouse_wheel);
	        mouse_wheel = 0.0;
	    }
	    g_target_fps = get_param_color(PARAM_RENDER_TARGET_FPS);
	    if ((currentTickTime - g_last_render_time) >= (1.0 / g_target_fps))
	    {
	        g_last_render_time = currentTickTime;  
	        vulkan_draw();
	    }
	}

	void engine_init()
	{
        AddVectoredExceptionHandler(1, VectoredHandler);
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
        HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        SymInitialize(process, "srv*C:\\symbols*https://msdl.microsoft.com/download/symbols", TRUE);
        watcher_init();
        init_macro_param_registry();
        project_init();
        physics_init();
        init_window();
        create_vk_instance();
        create_vk_surface();
        vulkan_init();
	    init_input();
	    g_ui_ctx = LOG_MALLOC(sizeof(UI_Context));
	    ui_init(g_ui_ctx);
	    init_camera();
	    lastTickTime = ildz_get_time();
	    g_last_render_time = lastTickTime;
	    g_frame_duration = 1.0 / (double)g_target_fps;
	    while (!ildz_should_window_close())
	    {
	        engine_tick();
	    }

	    watcher_cleanup();

	    if (g_window) {
	        glfwDestroyWindow(g_window);
	        g_window = NULL;
	    }
	    glfwTerminate();
	}
#endif