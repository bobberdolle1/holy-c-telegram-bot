@echo off
echo ========================================
echo   COMPILING C++ BRIDGE (MAXIMUM SPEED)
echo ========================================
echo.

g++ --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] G++ not found!
    echo Install MinGW-w64
    pause
    exit /b 1
)

echo [INFO] Compiling with maximum optimizations...
g++ -o holyc_bridge.exe holyc_bridge.cpp ^
    -lwininet ^
    -O3 ^
    -march=native ^
    -mtune=native ^
    -flto ^
    -ffast-math ^
    -funroll-loops ^
    -std=c++17 ^
    -static-libgcc ^
    -static-libstdc++ ^
    -s

if errorlevel 1 (
    echo [ERROR] Compilation failed
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Compiled: holyc_bridge.exe
echo [INFO] Optimizations: -O3 -march=native -flto
echo [INFO] Size optimized and stripped
echo.
echo Run: holyc_bridge.exe
pause
