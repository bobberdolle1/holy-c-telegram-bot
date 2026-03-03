# HolyC Telegram Bot - C++ Bridge
# In memory of Terry A. Davis
FROM gcc:13-bookworm

LABEL maintainer="terrystemple_bot"
LABEL description="HolyC Telegram Bot with C++ Bridge"

# Install dependencies
RUN apt-get update && apt-get install -y \
    sqlite3 \
    libsqlite3-dev \
    libcurl4-openssl-dev \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY holyc_bridge.cpp .
COPY telegram_bot.HC .
COPY personality.txt .
COPY config.txt .
COPY lib/ ./lib/
COPY plugins/ ./plugins/

# Compile C++ bridge with maximum optimizations
RUN g++ -o holyc_bridge holyc_bridge.cpp \
    -lsqlite3 \
    -lcurl \
    -O3 \
    -march=x86-64 \
    -mtune=generic \
    -flto \
    -ffast-math \
    -funroll-loops \
    -std=c++17 \
    -static-libgcc \
    -static-libstdc++ \
    -s \
    && chmod +x holyc_bridge

# Create volume for persistent data
VOLUME ["/app/data"]

# Expose no ports (bot uses Telegram API)
# Ollama connection via host network

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f memory.db || exit 1

# Run the bot
CMD ["./holyc_bridge"]
