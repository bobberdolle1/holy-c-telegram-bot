HOLY C TELEGRAM BOT - DOCKER DEPLOYMENT
========================================
In Memory of Terry A. Davis

PREREQUISITES:
--------------
1. Docker Desktop installed and running
2. Ollama running on Windows host (localhost:11434)
3. config.txt with bot token and model name

QUICK START:
------------
1. Make sure Ollama is running:
   ollama serve

2. Deploy bot:
   deploy.bat

3. View logs:
   logs.bat

4. Stop bot:
   stop.bat

ARCHITECTURE:
-------------
- Container: Linux (Debian) with GCC 13
- Bridge: C++ compiled with -O3 optimizations
- Network: Host mode (accesses Ollama on Windows host)
- Storage: ./data volume for persistent SQLite database
- Dependencies: libcurl, libsqlite3

OLLAMA CONNECTION:
------------------
Bot connects to Ollama via host network:
- Windows host: localhost:11434
- Container sees: localhost:11434 (via host network mode)
- No port mapping needed
- No Ollama installation in container

MANUAL COMMANDS:
----------------
Build:
  docker-compose build

Start:
  docker-compose up -d

Stop:
  docker-compose down

Logs:
  docker-compose logs -f

Shell access:
  docker exec -it terrystemple_bot /bin/bash

Rebuild:
  docker-compose down
  docker-compose build --no-cache
  docker-compose up -d

TROUBLESHOOTING:
----------------
1. "Ollama not responding":
   - Check Ollama is running: ollama list
   - Check port: curl http://localhost:11434/api/tags

2. "Container exits immediately":
   - Check logs: docker-compose logs
   - Verify config.txt exists and is valid

3. "Permission denied":
   - Run as administrator
   - Check Docker Desktop is running

4. "Build failed":
   - Check internet connection
   - Try: docker-compose build --no-cache

PERFORMANCE:
------------
- C++ bridge: ~10x faster than Python
- Compiled with -O3 -flto optimizations
- Static linking for portability
- Resource limits: 2 CPU cores, 512MB RAM

FILES:
------
Dockerfile          - Container definition
docker-compose.yml  - Deployment configuration
.dockerignore       - Build exclusions
deploy.bat          - Deployment script
stop.bat            - Stop script
logs.bat            - View logs script

God said 640x480. Keep it simple.
