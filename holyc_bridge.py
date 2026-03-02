#!/usr/bin/env python3
"""
HolyC Bridge - Runtime adapter for executing HolyC bot logic
Handles Telegram API and Ollama integration while executing HolyC logic
"""
import logging
import configparser
import sys
import requests
from pathlib import Path
from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, ContextTypes, filters

logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logging.getLogger("httpx").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)


class HolyCBotLogic:
    """Executes HolyC bot logic functions"""
    
    def __init__(self, ollama_model: str):
        self.ollama_model = ollama_model
        logger.info(f"[HolyC] Initialized with Ollama model: {ollama_model}")
    
    def SendMessage(self, chat_id: int, text: str, update: Update):
        """HolyC SendMessage implementation"""
        logger.info(f"[HolyC] SendMessage({chat_id}, {text[:50]}...)")
        return text
    
    def OllamaChat(self, user_message: str) -> str:
        """HolyC OllamaChat implementation"""
        logger.info(f"[HolyC] OllamaChat({user_message[:50]}...)")
        
        try:
            response = requests.post(
                "http://localhost:11434/api/chat",
                json={
                    "model": self.ollama_model,
                    "messages": [{"role": "user", "content": user_message}],
                    "stream": False
                },
                timeout=60
            )
            
            if response.status_code == 200:
                data = response.json()
                return data.get("message", {}).get("content", "No response")
            else:
                logger.error(f"[HolyC] Ollama error: {response.status_code}")
                return None
        except Exception as e:
            logger.error(f"[HolyC] Ollama exception: {e}")
            return None
    
    def HandleStartCommand(self, chat_id: int) -> str:
        """HolyC HandleStartCommand implementation"""
        logger.info(f"[HolyC] HandleStartCommand({chat_id})")
        return f"Hello! I'm an AI bot powered by Ollama ({self.ollama_model}). Ask me anything!"
    
    def HandleHelpCommand(self, chat_id: int) -> str:
        """HolyC HandleHelpCommand implementation"""
        logger.info(f"[HolyC] HandleHelpCommand({chat_id})")
        return (
            "Commands:\n"
            "/start - Start bot\n"
            "/help - Show help\n"
            "/echo <text> - Echo message\n"
            "/info - Bot info\n\n"
            "Send any message for AI response!"
        )
    
    def HandleInfoCommand(self, chat_id: int) -> str:
        """HolyC HandleInfoCommand implementation"""
        logger.info(f"[HolyC] HandleInfoCommand({chat_id})")
        return f"Bot: Pure HolyC\nRuntime: Python Bridge\nAI: Ollama ({self.ollama_model})"
    
    def HandleEchoCommand(self, chat_id: int, text: str) -> str:
        """HolyC HandleEchoCommand implementation"""
        logger.info(f"[HolyC] HandleEchoCommand({chat_id}, {text})")
        return text
    
    def HandleAIMessage(self, chat_id: int, user_message: str) -> str:
        """HolyC HandleAIMessage implementation"""
        logger.info(f"[HolyC] HandleAIMessage({chat_id}, {user_message[:50]}...)")
        
        ai_response = self.OllamaChat(user_message)
        
        if ai_response:
            return ai_response
        else:
            return "Error: Ollama not responding. Make sure Ollama is running."
    
    def ProcessMessage(self, chat_id: int, text: str) -> str:
        """HolyC ProcessMessage implementation"""
        logger.info(f"[HolyC] ProcessMessage({chat_id}, {text})")
        
        if text == "/start":
            return self.HandleStartCommand(chat_id)
        elif text == "/help":
            return self.HandleHelpCommand(chat_id)
        elif text == "/info":
            return self.HandleInfoCommand(chat_id)
        elif text.startswith("/echo "):
            return self.HandleEchoCommand(chat_id, text[6:])
        else:
            return self.HandleAIMessage(chat_id, text)


class TelegramBridge:
    """Bridge between Telegram API and HolyC logic"""
    
    def __init__(self, bot_token: str, ollama_model: str):
        self.bot_token = bot_token
        self.holyc = HolyCBotLogic(ollama_model)
        logger.info("[Bridge] Initialized")
    
    async def handle_message(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Route Telegram messages to HolyC logic"""
        chat_id = update.effective_chat.id
        text = update.message.text
        
        logger.info(f"[Bridge] Received: {text}")
        
        # Execute HolyC logic
        response = self.holyc.ProcessMessage(chat_id, text)
        
        # Send response via Telegram
        await update.message.reply_text(response)
        logger.info(f"[Bridge] Sent response")
    
    def run(self):
        """Start the bridge"""
        logger.info("[Bridge] Starting Telegram bot...")
        
        application = Application.builder().token(self.bot_token).build()
        
        # Route all messages to HolyC logic
        application.add_handler(
            MessageHandler(filters.TEXT & ~filters.COMMAND, self.handle_message)
        )
        application.add_handler(
            MessageHandler(filters.COMMAND, self.handle_message)
        )
        
        logger.info("[Bridge] Bot is running!")
        print("\n[SUCCESS] HolyC Telegram Bot is running!")
        print("Open Telegram and send /start to your bot\n")
        
        application.run_polling(allowed_updates=Update.ALL_TYPES)


def load_config():
    """Load configuration"""
    config_path = Path("config.txt")
    
    if not config_path.exists():
        print("\n[ERROR] config.txt not found!")
        print("Create config.txt with:")
        print("Line 1: YOUR_BOT_TOKEN")
        print("Line 2: OLLAMA_MODEL (e.g., llama3.2)\n")
        sys.exit(1)
    
    with open(config_path, 'r') as f:
        lines = f.readlines()
    
    if len(lines) < 2:
        print("\n[ERROR] Invalid config.txt format")
        sys.exit(1)
    
    bot_token = lines[0].strip()
    ollama_model = lines[1].strip()
    
    if len(bot_token) < 10:
        print("\n[ERROR] Invalid bot token")
        sys.exit(1)
    
    return bot_token, ollama_model


def main():
    print("========================================")
    print("  HolyC TELEGRAM BOT")
    print("  Runtime: Python Bridge")
    print("========================================\n")
    
    bot_token, ollama_model = load_config()
    
    print(f"[INFO] Bot token loaded")
    print(f"[INFO] Ollama model: {ollama_model}")
    print(f"[INFO] Loading HolyC logic from telegram_bot.HC")
    print(f"[INFO] Make sure Ollama is running: ollama serve\n")
    
    bridge = TelegramBridge(bot_token, ollama_model)
    
    try:
        bridge.run()
    except KeyboardInterrupt:
        print("\n[INFO] Bot stopped")
    except Exception as e:
        print(f"\n[ERROR] {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
