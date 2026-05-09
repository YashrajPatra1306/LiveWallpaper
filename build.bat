@echo off
REM Native Windows Sandbox Engine Build Script
REM Requires: MinGW-w64 (gcc) or MSVC

echo Building Native Sandbox Rendering Engine...
echo.

REM Check for MinGW GCC
where gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [GCC] Compiling with MinGW-w64...
    gcc -O2 -o sandbox_engine.exe sandbox_engine.c -lgdi32 -luser32 -lcomctl32 -lm -mwindows
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Build completed: sandbox_engine.exe
        echo.
        echo Run: sandbox_engine.exe
    ) else (
        echo [ERROR] Compilation failed!
        exit /b 1
    )
    goto :end
)

REM Check for MSVC
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [MSVC] Compiling with Microsoft Visual C++...
    cl /O2 /Fe:sandbox_engine.exe sandbox_engine.c gdi32.lib user32.lib comctl32.lib
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Build completed: sandbox_engine.exe
        echo.
        echo Run: sandbox_engine.exe
    ) else (
        echo [ERROR] Compilation failed!
        exit /b 1
    )
    goto :end
)

echo [ERROR] No compiler found!
echo Please install MinGW-w64 or MSVC and try again.
echo.
echo For MinGW-w64: https://www.mingw-w64.org/
echo For MSVC: Install Visual Studio Build Tools
exit /b 1

:end
pause
