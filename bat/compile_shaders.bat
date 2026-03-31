@echo off
setlocal

echo Compiling shaders...

if not exist shaders mkdir shaders

set GLSLC="%VULKAN_SDK%\Bin\glslc.exe"

if not exist %GLSLC% (
    echo ERROR: glslc.exe not found! Make sure Vulkan SDK is installed and VULKAN_SDK is set.
    pause
    exit /b 1
)

:: Only compile if source exists and spv is missing or older
set SHADERS=sky.vert sky.frag ui.vert ui.frag model.vert model.frag

for %%S in (%SHADERS%) do (
    if exist shaders\%%S (
        if not exist shaders\%%S.spv (
            echo Compiling %%S...
            %GLSLC% shaders\%%S -o shaders\%%S.spv
        ) else (
            echo %%S.spv already exists.
        )
    ) else (
        echo WARNING: shaders/%%S not found!
    )
)
echo Shader compilation done.