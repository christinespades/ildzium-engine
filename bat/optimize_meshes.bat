@echo off
setlocal enabledelayedexpansion

set "GLTFPACK=..\thirdparty\gltfpack.exe"
set "MESH_DIR=..\assets\meshes"

if not exist "%GLTFPACK%" (
    echo ERROR: gltfpack not found at "%GLTFPACK%"
    exit /b 1
)

:: echo.
:: echo Searching for raw meshes in: %MESH_DIR%
:: echo.

for %%F in ("%MESH_DIR%\*.glb") do (
    set "IN=%%~fF"
    set "NAME=%%~nF"
    set "EXT=%%~xF"

    :: Explicitly check if the filename already contains an LOD or optimization marker
    echo !NAME! | findstr /i "_opt _lod1 _lod2 _lod3" >nul
    if errorlevel 1 (
        
        echo Processing asset: !NAME!!EXT!

        :: 1. Generate Main Optimized Base Mesh
        set "OUT_BASE=%%~dpnF_opt.glb"
        if not exist "!OUT_BASE!" (
            echo   -^> Baking Base: !NAME!_opt.glb
            "%GLTFPACK%" -i "%%~fF" -o "!OUT_BASE!" -cf -tc -si 0.01 -sa -noq -mm -mi
            if errorlevel 1 goto :fail
        )

        :: 2. Generate LOD1 (Medium Detail)
        set "OUT_LOD1=%%~dpnF_lod1.glb"
        if not exist "!OUT_LOD1!" (
            echo   -^> Baking LOD1: !NAME!_lod1.glb
            "%GLTFPACK%" -i "%%~fF" -o "!OUT_LOD1!" -cf -tc -si 0.001 -sa -noq -mm -mi
            if errorlevel 1 goto :fail
        )

        :: 3. Generate LOD2 (Low Detail)
        set "OUT_LOD2=%%~dpnF_lod2.glb"
        if not exist "!OUT_LOD2!" (
            echo   -^> Baking LOD2: !NAME!_lod2.glb
            "%GLTFPACK%" -i "%%~fF" -o "!OUT_LOD2!" -cf -tc -si 0.0001 -sa -noq -mm -mi
            if errorlevel 1 goto :fail
        )

        :: 4. Generate LOD3 (Far Proxy)
        set "OUT_LOD3=%%~dpnF_lod3.glb"
        if not exist "!OUT_LOD3!" (
            echo   -^> Baking LOD3: !NAME!_lod3.glb
            "%GLTFPACK%" -i "%%~fF" -o "!OUT_LOD3!" -cf -tc -si 0.000001 -sa -noq -mm -mi
            if errorlevel 1 goto :fail
        )
        
        echo Finished tracking asset: !NAME!!EXT!
    )
)

echo.
:: echo === Mesh optimization and LOD generation complete ===
endlocal
exit /b 0

:fail
echo ERROR: FAILED to execute gltfpack target sequence pipeline step.
endlocal
exit /b 1