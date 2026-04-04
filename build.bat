@echo off
setlocal enabledelayedexpansion

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
set CURL_LIB=thirdparty\curl\libcurl.lib
set CURL_DLL=thirdparty\curl\libcurl-x64.dll

:: Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo === Optimizing meshes ===
call "bat\optimize_meshes.bat"
if errorlevel 1 (
    echo Mesh optimization FAILED!
    pause
    exit /b
)

echo === Compiling shaders ===
call "bat\compile_shaders.bat"

echo.
echo === Compiling Ildzium Engine ===

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Collect all .c files in the current directory
set "SRC_FILES="
for /R source %%f in (*.c) do set "SRC_FILES=!SRC_FILES! %%f"

rc /fo "%BUILD_DIR%\resource.res" resource.rc
if errorlevel 1 (
    echo Resource compilation FAILED!
    pause
    exit /b
)
cl /std:c11 /D_CRT_SECURE_NO_WARNINGS /wd4047 /wd4267 /wd4244 %SRC_FILES% ^
    /Zi /W3 /MD /nologo ^
    /I"source" ^
    /I"%VULKAN_INCLUDE%" ^
    /I"%THIRDPARTY_INCLUDE%" ^
    "%BUILD_DIR%\resource.res" ^
    /Fe"%BUILD_DIR%\%OUT_EXE%" ^
    /Fo"%BUILD_DIR%\\" ^
    /Fd"%BUILD_DIR%\\" ^
    /link ^
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
    echo Build succeeded!

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