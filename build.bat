@echo off
echo Building Sandbox Rendering Engine...
echo.

REM Check if MinGW is available
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: GCC/MinGW not found in PATH!
    echo.
    echo Please install MinGW-w64 from: https://www.mingw-w64.org/
    echo Or use MSYS2: https://www.msys2.org/
    echo.
    echo After installation, add the bin directory to your PATH.
    echo Example: C:\msys64\mingw64\bin
    echo.
    pause
    exit /b 1
)

echo Compiling sandbox_engine.c...
echo.

REM Compile with optimizations for performance
gcc -O2 -o sandbox_engine.exe sandbox_engine.c -lgdi32 -luser32 -lcomctl32 -lm -mwindows

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build successful!
    echo ========================================
    echo.
    echo Output: sandbox_engine.exe
    echo.
    echo The application is a native Windows executable.
    echo It does NOT use Electron, Chromium, or any web technologies.
    echo.
    echo Features:
    echo - Pure Win32 API + GDI rendering
    echo - Optimized for low-tier hardware
    echo - Full physics simulation
    echo - Particle system
    echo - Save/Load scenes
    echo.
    echo Run sandbox_engine.exe to start the application.
    echo.
    start sandbox_engine.exe
) else (
    echo.
    echo ========================================
    echo Build failed!
    echo ========================================
    echo.
    echo Make sure you have MinGW-w64 installed.
    echo Download from: https://www.mingw-w64.org/
    echo.
)

pause
