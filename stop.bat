@echo off
echo ========================================
echo   STOPPING HOLY C BOT
echo ========================================
echo.

docker-compose down

echo.
echo [INFO] Bot stopped
echo Data preserved in ./data directory
echo.
pause
