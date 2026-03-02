@echo off
echo ========================================
echo   HolyC TELEGRAM BOT LAUNCHER
echo ========================================
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found!
    pause
    exit /b 1
)

echo [INFO] Installing dependencies...
python -m pip install -r requirements.txt >nul 2>&1

echo [INFO] Starting HolyC bot with Python bridge...
echo.

python holyc_bridge.py

pause
