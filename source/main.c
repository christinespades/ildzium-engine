#include "pch.h"
#include "main.h"
#include "core/debug.h"
#include "scene/camera.h"
#include "input/input.h"
#include "ui/ui.h"
#include "scene/model.h"
#include "rendering/surface.h"
#include "core/watcher.h"

#pragma comment(lib, "dbghelp.lib")

void PrintStack(CONTEXT* ctx)
{
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 frame = {0};
    DWORD machine;

#ifdef _M_X64
    machine = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = ctx->Rip;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrStack.Offset = ctx->Rsp;
#elif _M_IX86
    machine = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrStack.Offset = ctx->Esp;
#endif

    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    for (int i = 0; i < 32; i++)
    {
        if (!StackWalk64(machine, process, GetCurrentThread(),
            &frame, ctx, NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL))
            break;

        DWORD64 addr = frame.AddrPC.Offset;

        // Function name
        char buffer[sizeof(SYMBOL_INFO) + 256];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;

        DWORD64 displacement = 0;

        if (SymFromAddr(process, addr, &displacement, symbol))
        {
            printf("%s + 0x%llX", symbol->Name, displacement);
        }
        else
        {
            printf("0x%llX", addr);
        }

        // File + line
        IMAGEHLP_LINE64 line = {0};
        line.SizeOfStruct = sizeof(line);
        DWORD lineDisplacement = 0;

        if (SymGetLineFromAddr64(process, addr, &lineDisplacement, &line))
        {
            printf(" (%s:%lu)", line.FileName, line.LineNumber);
        }

        printf("\n");
    }
}

const char* ExceptionToString(DWORD code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION: return "ACCESS_VIOLATION";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "ILLEGAL_INSTRUCTION";
        case EXCEPTION_STACK_OVERFLOW: return "STACK_OVERFLOW";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "INT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "FLOAT_DIVIDE_BY_ZERO";
        case EXCEPTION_IN_PAGE_ERROR: return "IN_PAGE_ERROR";
        default: return "UNKNOWN";
    }
}

LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;

    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_STACK_OVERFLOW:
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        case EXCEPTION_DATATYPE_MISALIGNMENT:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_PRIV_INSTRUCTION:
        case EXCEPTION_IN_PAGE_ERROR:
            break; // real crash
        default:
            return EXCEPTION_CONTINUE_SEARCH; // ignore noise
    }

    if (code == EXCEPTION_ACCESS_VIOLATION)
    {
        ULONG_PTR type = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        ULONG_PTR addr = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];

        const char* op =
            (type == 0) ? "READ" :
            (type == 1) ? "WRITE" :
            (type == 8) ? "EXECUTE" : "UNKNOWN";

        printf("Access violation: %s at %p\n", op, (void*)addr);
    }
    else {
        printf("Type: %s\n", ExceptionToString(code));
        printf("Address: %p\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
    }
    // Optional: inspect registers
    CONTEXT* ctx = ExceptionInfo->ContextRecord;
    PrintStack(ExceptionInfo->ContextRecord);
#ifdef _M_X64
    printf("RIP: %p\n", (void*)ctx->Rip);
#elif _M_IX86
    printf("EIP: %p\n", (void*)ctx->Eip);
#endif

    return EXCEPTION_CONTINUE_SEARCH; // let normal crash handling continue
}

static int g_target_fps = 60;

// Forward declarations for renderer
void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer();
void draw_frame();
extern GLFWwindow* g_window;
extern VkInstance vk_instance;
extern VkSurfaceKHR vk_surface;

int main()
{
    AddVectoredExceptionHandler(1, VectoredHandler);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    SymInitialize(process, "srv*C:\\symbols*https://msdl.microsoft.com/download/symbols", TRUE);
    surface_init();
    init_input();
    g_ui_ctx = malloc(sizeof(UI_Context));
    ui_init(g_ui_ctx);
    init_renderer(vk_instance, vk_surface);
    watcher_init();
    init_camera();

    double lastTime = glfwGetTime();
    double last_render_time = glfwGetTime();
    double frame_duration = 1.0 / (double)g_target_fps;

    while (!glfwWindowShouldClose(g_window))
    {
        _CrtCheckMemory();
        glfwPollEvents();
        
        watcher_update();

        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        update_camera(deltaTime);

        int left_pressed = glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (g_ui_ctx->cursor_captured) {
            // When UI is active (cursor visible), use absolute cursor position
            double mx, my;
            glfwGetCursorPos(g_window, &mx, &my);
            
            // Optional: clamp to window size to be safe
            int win_w, win_h;
            glfwGetWindowSize(g_window, &win_w, &win_h);
            if (mx < 0) mx = 0;
            if (my < 0) my = 0;
            if (mx >= win_w) mx = win_w - 1;
            if (my >= win_h) my = win_h - 1;

            ui_update(g_ui_ctx, (int)mx, (int)my, left_pressed, mouse_wheel, deltaTime);
            mouse_wheel = 0.0;  // reset each frame after consuming
        }
        else {
            // Game mode - mouse movement already handled in callback
            // You can still poll left click here if you want camera actions
        }

        currentTime = glfwGetTime();

        if ((currentTime - last_render_time) >= frame_duration)
        {
            last_render_time = currentTime;
            draw_frame();
        }
    }

    watcher_cleanup();
    cleanup_renderer();
    vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
    vkDestroyInstance(vk_instance, NULL);
    glfwDestroyWindow(g_window);
    ui_cleanup(g_ui_ctx);
    free(g_ui_ctx);
    glfwTerminate();

    return 0;
}