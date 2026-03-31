@echo off
setlocal enabledelayedexpansion

set "GLTFPACK=thirdparty\gltfpack.exe"
set "MESH_DIR=meshes"

if not exist "%GLTFPACK%" (
    echo ERROR: gltfpack not found at "%GLTFPACK%"
    exit /b 1
)

echo === Optimizing GLB meshes ===
echo Will generate *_opt.glb files in %MESH_DIR%
echo.

for %%F in ("%MESH_DIR%\*.glb") do (
    set "IN=%%~fF"
    set "NAME=%%~nF"
    set "EXT=%%~xF"

    echo !NAME!!EXT! | findstr /i "_opt.glb" >nul
    if errorlevel 1 (
        set "OUT=%%~dpnF_opt.glb"
        if exist "!OUT!" (
            echo Skipping already optimized: %%~nxF
        ) else (
            echo Optimizing: %%~nxF
            "%GLTFPACK%" -i "%%~fF" -o "!OUT!" -cf -tc -si 0.2 -noq -mm -mi
            if errorlevel 1 (
                echo FAILED to optimize %%~nxF
                exit /b 1
            )
            echo   Created: !OUT!
        )
    ) else (
        echo Skipping optimized file: %%~nxF
    )
)

echo.
echo === Mesh optimization complete ===
endlocal