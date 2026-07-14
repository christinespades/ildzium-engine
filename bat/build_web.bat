@echo off
setlocal enabledelayedexpansion

set "WEB_BUILD_DIR=..\builds\web"
set "PROD_OUT=C:\Users\Public\Downloads\000github\christineee.com\static\js"
set "THIRDPARTY_INCLUDE=..\thirdparty"

if not exist "%WEB_BUILD_DIR%" mkdir "%WEB_BUILD_DIR%"

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
if errorlevel 1 (
echo Emscripten development build FAILED!
exit /b 1
)

:: Build production
call emcc %SRC_FILES% ^
%COMMON_FLAGS% ^
--shell-file shell.html ^
-o "%PROD_OUT%\ildzium.js"
if errorlevel 1 (
echo Emscripten production build FAILED!
exit /b 1
)

xcopy /E /I /Y ..\assets "%WEB_BUILD_DIR%\assets" >nul
xcopy /Y ..\web* "%WEB_BUILD_DIR%" >nul

echo.
echo WEB BUILD COMPLETE
exit /b 0