@echo off
echo ========================================
echo   HOLY C BOT - DOCKER DEPLOYMENT
echo   In Memory of Terry A. Davis
echo ========================================
echo.

REM Check if Docker is running
docker info >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Docker is not running!
    echo Please start Docker Desktop
    pause
    exit /b 1
)

REM Check if config.txt exists
if not exist config.txt (
    echo [ERROR] config.txt not found!
    echo Copy config.txt.example to config.txt and fill in your credentials
    pause
    exit /b 1
)

REM Check if Ollama is running on host
echo [INFO] Checking Ollama connection...
curl -s http://localhost:11434/api/tags >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Ollama not responding on localhost:11434
    echo Make sure Ollama is running on your Windows host
    echo.
    choice /C YN /M "Continue anyway?"
    if errorlevel 2 exit /b 1
)

REM Create data directory for persistent storage
if not exist data mkdir data

echo.
echo [INFO] Building Docker image...
docker-compose build

if errorlevel 1 (
    echo [ERROR] Docker build failed
    pause
    exit /b 1
)

echo.
echo [INFO] Starting bot container...
docker-compose up -d

if errorlevel 1 (
    echo [ERROR] Failed to start container
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Bot deployed successfully!
echo.
echo Container: terrystemple_bot
echo Status: docker-compose ps
echo Logs: docker-compose logs -f
echo Stop: docker-compose down
echo.
echo Bot is now running and connected to Ollama on host Windows
echo Open Telegram and send /start to @terrystemple_bot
echo.
pause
