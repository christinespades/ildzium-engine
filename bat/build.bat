@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=..\builds\debug
set WEB_BUILD_DIR=..\builds\web
set OUT_EXE=ildzium.exe
set PROD_OUT=C:\Users\Public\Downloads\000github\christineee.com\static\js
set THIRDPARTY_INCLUDE=..\thirdparty
set GLFW_LIB=..\thirdparty\GLFW\glfw3dll.lib
set GLFW_DLL=..\thirdparty\GLFW\glfw3.dll
set CURL_LIB=..\thirdparty\curl\libcurl.lib
set CURL_DLL=..\thirdparty\curl\libcurl-x64.dll

:: === Argument Parsing ===
set BUILD_WEB=0
set LOG_FLAGS=
for %%a in (%*) do (
    if /I "%%a"=="-WEB" set BUILD_WEB=1
    if /I "%%a"=="-LOG_SCOPE" set LOG_FLAGS=!LOG_FLAGS! /DLOG_SCOPE_ENABLED
    if /I "%%a"=="-LOG_MALLOC" set LOG_FLAGS=!LOG_FLAGS! /DLOG_MALLOC_ENABLED
)

if "%BUILD_WEB%"=="1" (
    goto BUILD_WEB
) else (
    goto BUILD_NATIVE
)

:BUILD_NATIVE
:: Vulkan Specifics
set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
set "VULKAN_LIB=%VULKAN_SDK%\Lib"
set "GLSLC=%VULKAN_SDK%\Bin\glslc.exe"

:: Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

call "optimize_meshes.bat"
if errorlevel 1 (
    echo Mesh optimization FAILED!
    pause
    exit /b
)

call "compile_shaders.bat"
if errorlevel 1 (
    echo Shader compilation FAILED!
    pause
    exit /b 1
)
echo.

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set VSCMD_SKIP_LOGO=1
rc /fo "%BUILD_DIR%\resource.res" ..\resource.rc
if errorlevel 1 (
    echo Resource compilation FAILED!
    pause
    exit /b
)

:: Shared compiler settings, optimization, and target flags
set "BASE_FLAGS=/std:c11 /D_CRT_SECURE_NO_WARNINGS /Zi /Od /RTC1 /GS /MD /nologo"
set "DISABLED_WARNINGS=/wd4005 /wd4047 /wd4100 /wd4267 /wd4244"
set "INCLUDES=/I"..\source" /I"%VULKAN_INCLUDE%" /I"%THIRDPARTY_INCLUDE%""
set "COMMON_FLAGS=%LOG_FLAGS% %BASE_FLAGS% %DISABLED_WARNINGS% %INCLUDES%"

:: Compile Precompiled Header (PCH)
cl %COMMON_FLAGS% ^
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

:: Collect Source Files
set "SRC_FILES="
for /R ..\source %%f in (*.c) do (
    echo %%~nxf | findstr /I "webgpu" >nul
    if errorlevel 1 (
        if /I not "%%~nxf"=="pch.c" (
            set "SRC_FILES=!SRC_FILES! %%f"
        )
    )
)

:: Compile Main Files & Link
(
    cl %COMMON_FLAGS% ^
        /Yu"pch.h" /Fp"%BUILD_DIR%\pch.pch" ^
        %SRC_FILES% ^
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
        user32.lib shell32.lib 2>&1
) | powershell -Command "$c=0; $had_err=0; $input | %%{ if($_ -notmatch '^[a-zA-Z0-9_]*\.c$' -and $_ -notmatch '^Generating Code...$' -and $_ -notmatch '^Compiling...$'){ if($_ -match 'error' -or $_ -match 'fatal error'){ $c++; $had_err=1; Write-Host $_ -ForegroundColor Red } else { Write-Host $_ }; if($c -ge 13){ Write-Host '>>> ABORTING: TOO MANY ERRORS. <<<' -ForegroundColor Red; Stop-Process -Name cl -Force -ErrorAction SilentlyContinue; exit 1 } } }; if($had_err -eq 1){ exit 1 }"
set MAIN_ERR=%errorlevel%

if %MAIN_ERR% neq 0 (
    echo.
    echo Build FAILED or Aborted!
    pause
    exit /b
)

:: Check if build succeeded
if exist "%BUILD_DIR%\%OUT_EXE%" (
    echo.

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
goto END

:BUILD_WEB
if not exist %WEB_BUILD_DIR% mkdir %WEB_BUILD_DIR%

set "SRC_FILES="
for /R ..\source %%f in (*.c) do (
    if /I not "%%~nxf"=="pch.c" (
        set "SRC_FILES=!SRC_FILES! %%f"
    )
)

set COMMON_FLAGS=^
 -I"..\source" ^
 -I"%THIRDPARTY_INCLUDE%" ^
 -I"..\thirdparty" ^
 -O2 ^
 --use-port=emdawnwebgpu ^
 --preload-file ../assets@assets ^
 -s ALLOW_MEMORY_GROWTH=1 ^
 -s ALLOW_TABLE_GROWTH=1 ^
 -s RESERVED_FUNCTION_POINTERS=64 ^
 -s WASM=1 ^
 -s FETCH=1 ^
 -s ASSERTIONS=1 ^
 -s ENVIRONMENT=web ^
 -s MODULARIZE=1 ^
 -s EXPORT_ES6=0 ^
 -s ASYNCIFY=1 ^
 -s EXPORTED_FUNCTIONS="['_main','_on_http_result']" ^
 -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
 --js-library "..\source\platform\web\library_http.js"

:: Build development
call emcc %SRC_FILES% ^
 %COMMON_FLAGS% ^
 --shell-file shell.html ^
 -o "%WEB_BUILD_DIR%\index.html"

:: Build production
call emcc %SRC_FILES% ^
 %COMMON_FLAGS% ^
 --shell-file shell.html ^
 -o "%PROD_OUT%\ildzium.js"

xcopy /E /I /Y ..\assets %WEB_BUILD_DIR%\assets
xcopy /Y ..\web\* %WEB_BUILD_DIR%\

echo.
echo WEB BUILD COMPLETE
:END