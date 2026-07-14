#pragma once
#if defined(_WIN32)
    #pragma comment(lib, "dbghelp.lib")

    inline void PrintStack(CONTEXT* ctx, const char* crashInfoPrefix)
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

        char reportBuffer[4096] = {0};
        size_t len = snprintf(reportBuffer, sizeof(reportBuffer), "%s", crashInfoPrefix);

        for (int i = 0; i < 32; i++)
        {
            if (!StackWalk64(machine, process, GetCurrentThread(),
                &frame, ctx, NULL,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                NULL))
                break;

            DWORD64 addr = frame.AddrPC.Offset;

            char buffer[sizeof(SYMBOL_INFO) + 256];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = 255;

            DWORD64 displacement = 0;
            char symbolStr[256] = "";
            char lineStr[256] = "";

            if (SymFromAddr(process, addr, &displacement, symbol))
            {
                snprintf(symbolStr, sizeof(symbolStr), "%s + 0x%llX", symbol->Name, displacement);
            }
            else
            {
                snprintf(symbolStr, sizeof(symbolStr), "0x%llX", addr);
            }

            // --- FIXED: Re-added the missing struct definitions here ---
            IMAGEHLP_LINE64 line = {0};
            line.SizeOfStruct = sizeof(line);
            DWORD lineDisplacement = 0;

            if (SymGetLineFromAddr64(process, addr, &lineDisplacement, &line))
            {
                const char* relativePath = RELEST_PATH(line.FileName);
                snprintf(lineStr, sizeof(lineStr), " AT LINE %s:%lu", relativePath, line.LineNumber);
            }

            if (i == 0)
            {
                len += snprintf(reportBuffer + len, sizeof(reportBuffer) - len, 
                                "\nDURING %s%s", symbolStr, lineStr);
            }
            else if (i == 1)
            {
                len += snprintf(reportBuffer + len, sizeof(reportBuffer) - len, 
                                " IN FUNCTION %s%s", symbolStr, lineStr);
            }
            else
            {
                int indentLevel = i - 1;
                char indentStr[128] = "";
                for (int s = 0; s < indentLevel * 4 && s < 120; s++) {
                    indentStr[s] = ' ';
                }

                len += snprintf(reportBuffer + len, sizeof(reportBuffer) - len, 
                                "\n%s[%d] %s%s", indentStr, indentLevel, symbolStr, lineStr);
            }
        }

        LOGE("%s", reportBuffer);
    }

    inline const char* ExceptionToString(DWORD code)
    {
        switch (code)
        {
            case EXCEPTION_ACCESS_VIOLATION: return "Access violation";
            case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
            case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
            case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer divide by zero";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Float divide by zero";
            case EXCEPTION_IN_PAGE_ERROR: return "In page error";
            default: return "Unknown exception";
        }
    }

    inline LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
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
                return EXCEPTION_CONTINUE_SEARCH;
        }

        char crashInfoPrefix[512] = {0};

        if (code == EXCEPTION_ACCESS_VIOLATION)
        {
            ULONG_PTR type = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
            ULONG_PTR addr = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];

            const char* op =
                (type == 0) ? "READ" :
                (type == 1) ? "WRITE" :
                (type == 8) ? "EXECUTE" : "UNKNOWN";

            snprintf(crashInfoPrefix, sizeof(crashInfoPrefix), "Access violation: %s at 0x%p", op, (void*)addr);
        }
        else 
        {
            snprintf(crashInfoPrefix, sizeof(crashInfoPrefix), "%s at 0x%p", ExceptionToString(code), ExceptionInfo->ExceptionRecord->ExceptionAddress);
        }

        // Pass the prepared string prefix down to get processed inline
        PrintStack(ExceptionInfo->ContextRecord, crashInfoPrefix);

        return EXCEPTION_CONTINUE_SEARCH;
    }
#endif