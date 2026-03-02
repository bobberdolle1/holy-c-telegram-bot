@echo off
echo ========================================
echo   HOLY C BOT LAUNCHER (C++ FAST MODE)
echo ========================================
echo.

if not exist holyc_bridge.exe (
    echo [ERROR] holyc_bridge.exe not found!
    echo Run compile_fast.bat first
    pause
    exit /b 1
)

echo [INFO] Starting C++ bridge (MAXIMUM SPEED)...
echo.

holyc_bridge.exe

pause
