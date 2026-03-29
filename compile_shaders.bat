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
if exist shaders\sky.vert (
    if not exist shaders\sky.vert.spv (
        echo Compiling sky.vert...
        %GLSLC% shaders\sky.vert -o shaders\sky.vert.spv
    ) else (
        echo sky.vert.spv already exists.
    )
) else (
    echo WARNING: shaders/sky.vert not found!
)

if exist shaders\sky.frag (
    if not exist shaders\sky.frag.spv (
        echo Compiling sky.frag...
        %GLSLC% shaders\sky.frag -o shaders\sky.frag.spv
    ) else (
        echo sky.frag.spv already exists.
    )
) else (
    echo WARNING: shaders/sky.frag not found!
)

echo Shader compilation done. Copying shaders to build directory...
if not exist %BUILD_DIR%\shaders mkdir %BUILD_DIR%\shaders
copy /Y shaders\*.spv %BUILD_DIR%\shaders\