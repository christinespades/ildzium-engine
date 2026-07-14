@echo off
setlocal enabledelayedexpansion

:: Expect caller to have set these globals; re-export local copies if needed
set "BUILD_DIR=%~dp0..\builds\debug"
set "OUT_EXE=ildzium.exe"
set "THIRDPARTY_INCLUDE=%~dp0..\thirdparty"

:: Vulkan Specifics
set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
set "VULKAN_LIB=%VULKAN_SDK%\Lib"
set "GLSLC=%VULKAN_SDK%\Bin\glslc.exe"

:: Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

call "%~dp0optimize_meshes.bat"
if errorlevel 1 (
echo Mesh optimization FAILED!
exit /b 1
)

call "%~dp0compile_shaders.bat"
if errorlevel 1 (
echo Shader compilation FAILED!
exit /b 1
)

echo.

set VSCMD_SKIP_LOGO=1
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

rc /fo "%BUILD_DIR%\resource.res" ..\resource.rc
if errorlevel 1 (
echo Resource compilation FAILED!
exit /b 1
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
exit /b 1
)

:: Collect all .c files in the current directory, excluding other platform headers
set "SRC_FILES="
for /R ..\source %%f in (*.c) do (
echo %%~nxf | findstr /I "webgpu" >nul
if errorlevel 1 (
if /I not "%%~nxf"=="pch.c" (
set "SRC_FILES=!SRC_FILES! %%f"
)
)
)

:: Compile main files (Filtered, errorlevel preserved)
cl %LOG_FLAGS% /std:c11 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4047 /wd4100 /wd4267 /wd4244 ^
/Yu"pch.h" /Fp"%BUILD_DIR%\pch.pch" ^
%SRC_FILES% ^
/Zi /Od /RTC1 /GS /W4 /WX /MD /nologo ^
/I"..\source" ^
/I"%VULKAN_INCLUDE%" ^
/I"%THIRDPARTY_INCLUDE%" ^
"%BUILD_DIR%\resource.res" ^
"%BUILD_DIR%\pch.obj" ^
/Fe"%BUILD_DIR%%OUT_EXE%" ^
/Fo"%BUILD_DIR%\" ^
/Fd"%BUILD_DIR%\" ^
/link /MACHINE:X64 ^
/DEBUG ^
/LIBPATH:"%VULKAN_LIB%" ^
vulkan-1.lib ^
"%GLFW_LIB%" "%CURL_LIB%" ^
user32.lib shell32.lib > "%BUILD_DIR%\main_build.log" 2>&1
set MAIN_ERR=%errorlevel%

type "%BUILD_DIR%\main_build.log" | findstr /V /R /C:"^[a-zA-Z0-9_]*.c$" /C:"^Generating Code...$" /C:"^Compiling...$"
del "%BUILD_DIR%\main_build.log" >nul 2>&1

if %MAIN_ERR% neq 0 (
echo.
echo Build FAILED!
exit /b 1
)

:: Check if build succeeded
if exist "%BUILD_DIR%%OUT_EXE%" (
echo.

if exist "%GLFW_DLL%" (
copy /Y "%GLFW_DLL%" "%BUILD_DIR%" >nul
) else (
echo WARNING: glfw3.dll not found at %GLFW_DLL%
echo You may need to copy it manually or adjust the path.
)

if exist "%CURL_DLL%" (
copy /Y "%CURL_DLL%" "%BUILD_DIR%" >nul
) else (
echo WARNING: libcurl-x64.dll not found at %CURL_DLL%
)

echo.
echo Running %BUILD_DIR%%OUT_EXE%...
pushd "%BUILD_DIR%"
"%OUT_EXE%"
popd
exit /b 0
) else (
echo.
echo Build FAILED - executable not found.
echo Check the errors above.
exit /b 1
)