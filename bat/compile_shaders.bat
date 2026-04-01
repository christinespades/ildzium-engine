@echo off
setlocal enabledelayedexpansion

echo Compiling shaders...

if not exist shaders mkdir shaders

set GLSLC=%VULKAN_SDK%\Bin\glslc.exe
set SHADER_DIR=%CD%\shaders

if not exist "%GLSLC%" (
    echo ERROR: glslc.exe not found!
    pause
    exit /b 1
)

for %%F in ("%SHADER_DIR%\*.vert" "%SHADER_DIR%\*.frag") do (
    set "INPUT=%%~fF"
    set "OUTPUT=%SHADER_DIR%\%%~nxF.spv"

    if not exist "!OUTPUT!" (
        echo Compiling %%~nxF...
        "%GLSLC%" "!INPUT!" -o "!OUTPUT!"
    ) else (
        rem Optional: recompile if source is newer
        for %%A in ("!INPUT!") do for %%B in ("!OUTPUT!") do (
            if %%~tA GTR %%~tB (
                echo Recompiling %%~nxF...
                "%GLSLC%" "!INPUT!" -o "!OUTPUT!"
            ) else (
                echo %%~nxF is up to date.
            )
        )
    )
)

echo Shader compilation done.