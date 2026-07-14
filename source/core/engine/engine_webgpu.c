#include "pch.h"
#ifdef __EMSCRIPTEN__
	#include "engine_webgpu.h"

	double lastTickTime;

	void engine_tick()
	{
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
	    webgpu_draw();
	}

	void engine_init()
	{
        init_macro_param_registry();
        project_init();
        physics_init();
        init_window();
        webgpu_init();
	    init_input();
	    g_ui_ctx = LOG_MALLOC(sizeof(UI_Context));
	    ui_init(g_ui_ctx);
	    init_camera();
	    lastTickTime = ildz_get_time();
	    g_last_render_time = lastTickTime;
	    g_target_fps = get_param_color(PARAM_RENDER_TARGET_FPS);
	    g_frame_duration = 1.0 / (double)g_target_fps;
    	emscripten_set_main_loop(engine_tick, 0, 1);   // 0 = use requestAnimationFrame
	}
#endif