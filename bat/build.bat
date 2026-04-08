@echo off
setlocal enabledelayedexpansion

:: === Configuration ===
set BUILD_DIR=..\builds\debug
set OUT_EXE=ildzium.exe

set LOG_FLAGS=
for %%a in (%*) do (
    if /I "%%a"=="-LOG_SCOPE" set LOG_FLAGS=!LOG_FLAGS! /DLOG_SCOPE_ENABLED
    if /I "%%a"=="-LOG_MALLOC" set LOG_FLAGS=!LOG_FLAGS! /DLOG_MALLOC_ENABLED
)

:: Vulkan
set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
set "VULKAN_LIB=%VULKAN_SDK%\Lib"
set "GLSLC=%VULKAN_SDK%\Bin\glslc.exe"

:: Dependency paths
set THIRDPARTY_INCLUDE=..\thirdparty
set GLFW_LIB=..\thirdparty\GLFW\glfw3dll.lib
set GLFW_DLL=..\thirdparty\GLFW\glfw3.dll
set CURL_LIB=..\thirdparty\curl\libcurl.lib
set CURL_DLL=..\thirdparty\curl\libcurl-x64.dll

:: Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

call "optimize_meshes.bat"
if errorlevel 1 (
    echo Mesh optimization FAILED!
    pause
    exit /b
)

call "compile_shaders.bat"

echo.

set VSCMD_SKIP_LOGO=1
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

rc /fo "%BUILD_DIR%\resource.res" ..\resource.rc
if errorlevel 1 (
    echo Resource compilation FAILED!
    pause
    exit /b
)

:: Compile precompiled header
cl %LOG_FLAGS% /std:c11 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4047 /wd4100 /wd4267 /wd4244 ^
    /Zi /Od /RTC1 /GS /W4 /WX /MD /nologo ^
    /I"..\source" ^
    /I"%VULKAN_INCLUDE%" ^
    /I"%THIRDPARTY_INCLUDE%" ^
    /Fo"%BUILD_DIR%\pch.obj" ^
    /Fp"%BUILD_DIR%\pch.pch" ^
    /Fd"%BUILD_DIR%\vc140.pdb" ^
    /Yc"pch.h" ^
    /c ..\source\pch.c
if errorlevel 1 (
    echo PCH compilation FAILED!
    pause
    exit /b
)

:: Collect all .c files in the current directory
set "SRC_FILES="
for /R ..\source %%f in (*.c) do (
    if /I not "%%~nxf"=="pch.c" (
        set "SRC_FILES=!SRC_FILES! %%f"
    )
)

cl %LOG_FLAGS% /std:c11 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4047 /wd4100 /wd4267 /wd4244 ^
    /Yu"pch.h" /Fp"%BUILD_DIR%\pch.pch" ^
    %SRC_FILES% ^
    /Zi /Od /RTC1 /GS /W4 /WX /MD /nologo ^
    /I"..\source" ^
    /I"%VULKAN_INCLUDE%" ^
    /I"%THIRDPARTY_INCLUDE%" ^
    "%BUILD_DIR%\resource.res" ^
    "%BUILD_DIR%\pch.obj" ^
    /Fe"%BUILD_DIR%\%OUT_EXE%" ^
    /Fo"%BUILD_DIR%\\" ^
    /Fd"%BUILD_DIR%\\" ^
    /link /MACHINE:X64 ^
    /DEBUG ^
    /LIBPATH:"%VULKAN_LIB%" ^
    vulkan-1.lib ^
    "%GLFW_LIB%" "%CURL_LIB%" ^
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

    :: Copy GLFW DLL next to the exe (required for runtime)
    if exist "%GLFW_DLL%" (
        copy /Y "%GLFW_DLL%" "%BUILD_DIR%\"
    ) else (
        echo WARNING: glfw3.dll not found at %GLFW_DLL%
        echo You may need to copy it manually or adjust the path.
    )

    if exist "%CURL_DLL%" (
        copy /Y "%CURL_DLL%" "%BUILD_DIR%\"
    ) else (
        echo WARNING: libcurl-x64.dll not found at %CURL_DLL%
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