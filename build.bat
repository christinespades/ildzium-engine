@echo off
setlocal

:: === Configuration ===
set BUILD_DIR=builds\debug
set OUT_EXE=ildzium.exe

:: Vulkan
set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
set "VULKAN_LIB=%VULKAN_SDK%\Lib"
set "GLSLC=%VULKAN_SDK%\Bin\glslc.exe"

:: Dependency paths
set THIRDPARTY_INCLUDE=thirdparty
set GLFW_LIB=thirdparty\GLFW\glfw3dll.lib
set GLFW_DLL=thirdparty\GLFW\glfw3.dll

:: Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo === Compiling shaders ===
call compile_shaders.bat

echo.
echo === Compiling Ildzium Engine ===

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cl main.c renderer.c camera.c input.c ^
    /Zi /W3 /MD /nologo ^
    /I"%VULKAN_INCLUDE%" ^
    /I"%THIRDPARTY_INCLUDE%" ^
    /Fe"%BUILD_DIR%\%OUT_EXE%" ^
    /Fo"%BUILD_DIR%\\" ^
    /Fd"%BUILD_DIR%\\" ^
    /link ^
    /DEBUG ^
    /LIBPATH:"%VULKAN_LIB%" ^
    vulkan-1.lib ^
    "%GLFW_LIB%" ^
    user32.lib shell32.lib

if errorlevel 1 (
    echo.
    echo Build FAILED!
    pause
    exit /b
)

:: Check if build succeeded
if exist "%BUILD_DIR%\%OUT_EXE%" (
    echo.
    echo Build succeeded!

    :: Copy GLFW DLL next to the exe (required for runtime)
    if exist "%GLFW_DLL%" (
        copy /Y "%GLFW_DLL%" "%BUILD_DIR%\"
    ) else (
        echo WARNING: glfw3.dll not found at %GLFW_DLL%
        echo You may need to copy it manually or adjust the path.
    )

    echo.
    echo Running %BUILD_DIR%\%OUT_EXE%...
    pushd "%BUILD_DIR%"
    %OUT_EXE%
    popd
) else (
    echo.
    echo Build FAILED - executable not found.
    echo Check the errors above.
    pause
)