// Telegram Bot in Pure HolyC
// TempleOS-compatible syntax

U0 HandleStartCommand(I64 chat_id) {
  U8 *message = "Hello! I'm an AI bot powered by Ollama. Ask me anything!";
  Print("Sending to chat %d: %s\n", chat_id, message);
  SendMessage(chat_id, message);
}

U0 HandleHelpCommand(I64 chat_id) {
  U8 *help_text = "Commands:\n/start - Start bot\n/help - Show help\n/echo <text> - Echo message\n/info - Bot info\n\nSend any message for AI response!";
  SendMessage(chat_id, help_text);
}

U0 HandleInfoCommand(I64 chat_id) {
  U8 *info = "Bot: Pure HolyC\nRuntime: Bridge Adapter\nAI: Ollama Integration";
  SendMessage(chat_id, info);
}

U0 HandleEchoCommand(I64 chat_id, U8 *text) {
  SendMessage(chat_id, text);
}

U0 HandleAIMessage(I64 chat_id, U8 *user_message) {
  U8 *ai_response = OllamaChat(user_message);
  
  if (ai_response) {
    SendMessage(chat_id, ai_response);
  } else {
    SendMessage(chat_id, "Error: AI not responding");
  }
}

U0 ProcessMessage(I64 chat_id, U8 *text) {
  Print("Processing message from %d: %s\n", chat_id, text);
  
  if (StrCmp(text, "/start") == 0) {
    HandleStartCommand(chat_id);
  }
  else if (StrCmp(text, "/help") == 0) {
    HandleHelpCommand(chat_id);
  }
  else if (StrCmp(text, "/info") == 0) {
    HandleInfoCommand(chat_id);
  }
  else if (StrNCmp(text, "/echo ", 6) == 0) {
    HandleEchoCommand(chat_id, text + 6);
  }
  else {
    HandleAIMessage(chat_id, text);
  }
}

U0 TelegramBotMain() {
  Print("========================================\n");
  Print("  TELEGRAM BOT (Pure HolyC)\n");
  Print("========================================\n\n");
  Print("Bot is running...\n");
  Print("Waiting for messages...\n\n");
  
  // Main loop handled by bridge
  while (TRUE) {
    Sleep(100);
  }
}
