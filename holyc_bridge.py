#!/usr/bin/env python3
"""
HolyC Bridge - Runtime adapter for executing HolyC bot logic
Handles Telegram API and Ollama integration while executing HolyC logic
"""
import logging
import sys
import requests
import sqlite3
import random
from pathlib import Path
from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, ContextTypes, filters

logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logging.getLogger("httpx").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)

# Global stats
total_messages = 0
total_commands = 0
command_stats = {}
admin_id = 0


class HolyCBotLogic:
    """Executes HolyC bot logic functions"""
    
    def __init__(self, ollama_model: str):
        self.ollama_model = ollama_model
        self.terry_quotes = [
            "God said 640x480 is a covenant like circumcision.",
            "An idiot admires complexity, a genius admires simplicity.",
            "I've learned a lot. I've become like a scientist, and God has been my assistant.",
            "The CIA is after me.",
            "TempleOS is God's official temple. Just like Solomon's temple, this is a community focal point where offerings are made and God's oracle is consulted.",
            "It's beautiful, it's fun, it's joyful. I'm happy programming.",
            "God said 16 colors is a covenant.",
            "I talk to God, and he's the smartest person there is.",
            "The whole point of TempleOS is to talk to God.",
            "Perfection is achieved, not when there is nothing more to add, but when there is nothing left to take away.",
            "Keep it simple. God likes simple.",
            "Modern software is bloated. God wants 640x480.",
            "HolyC is the language of the Third Temple.",
            "I'm the smartest programmer that's ever lived.",
            "God told me to make an operating system."
        ]
        
        self.bible_quotes = [
            "In the beginning God created the heaven and the earth. - Genesis 1:1",
            "And God said, Let there be light: and there was light. - Genesis 1:3",
            "So God created man in his own image. - Genesis 1:27",
            "The fear of the LORD is the beginning of wisdom. - Proverbs 9:10",
            "Trust in the LORD with all thine heart. - Proverbs 3:5",
            "I can do all things through Christ which strengtheneth me. - Philippians 4:13",
            "For God so loved the world, that he gave his only begotten Son. - John 3:16",
            "The LORD is my shepherd; I shall not want. - Psalm 23:1",
            "Be still, and know that I am God. - Psalm 46:10",
            "Thy word is a lamp unto my feet. - Psalm 119:105"
        ]
        
        self.templeos_colors = [
            "⬛ BLACK", "🔵 BLUE", "🟢 GREEN", "🔷 CYAN",
            "🔴 RED", "🟣 PURPLE", "🟡 YELLOW", "⚪ WHITE",
            "⬜ GRAY", "💙 LIGHT_BLUE", "💚 LIGHT_GREEN", "💎 LIGHT_CYAN",
            "❤️ LIGHT_RED", "💜 LIGHT_PURPLE", "💛 LIGHT_YELLOW", "✨ BRIGHT_WHITE"
        ]
        
        # Initialize database
        self.db = sqlite3.connect('memory.db', check_same_thread=False)
        self.init_database()
        
        # Load personality
        try:
            with open("personality.txt", "r") as f:
                self.personality = f.read()
            logger.info("[HolyC] Personality loaded from personality.txt")
        except FileNotFoundError:
            self.personality = "You are a helpful AI assistant."
            logger.warning("[HolyC] personality.txt not found, using default")
        
        logger.info(f"[HolyC] Initialized with Ollama model: {ollama_model}")
    
    def init_database(self):
        """Initialize SQLite database"""
        cursor = self.db.cursor()
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS user_memory (
                chat_id INTEGER PRIMARY KEY,
                data TEXT,
                timestamp INTEGER
            )
        ''')
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS message_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                chat_id INTEGER,
                user_id INTEGER,
                text TEXT,
                embedding TEXT,
                timestamp INTEGER
            )
        ''')
        cursor.execute('''
            CREATE INDEX IF NOT EXISTS idx_chat_timestamp 
            ON message_history(chat_id, timestamp DESC)
        ''')
        self.db.commit()
        logger.info("[HolyC] Memory database initialized with RAG support")
    
    def save_memory(self, chat_id: int, data: str) -> str:
        """Save user memory"""
        cursor = self.db.cursor()
        cursor.execute(
            'INSERT OR REPLACE INTO user_memory (chat_id, data, timestamp) VALUES (?, ?, ?)',
            (chat_id, data, int(__import__('time').time()))
        )
        self.db.commit()
        return "💾 Memory saved in the divine database."
    
    def load_memory(self, chat_id: int) -> str:
        """Load user memory"""
        cursor = self.db.cursor()
        cursor.execute('SELECT data FROM user_memory WHERE chat_id = ?', (chat_id,))
        result = cursor.fetchone()
        return result[0] if result else ""
    
    def clear_memory(self, chat_id: int) -> str:
        """Clear user memory"""
        cursor = self.db.cursor()
        cursor.execute('DELETE FROM user_memory WHERE chat_id = ?', (chat_id,))
        self.db.commit()
        return "🗑️ Memory cleared. Simplicity restored."
    
    def divine_text_transform(self, text: str) -> str:
        """Divine text transformation"""
        transform_type = random.randint(0, 2)
        if transform_type == 0:
            return text.upper()
        elif transform_type == 1:
            return text.lower()
        else:
            return text[::-1]
    
    def generate_oracle_text(self) -> str:
        """Generate divine oracle text"""
        divine_words = [
            "God", "Temple", "Simple", "640x480", "Holy",
            "Divine", "Covenant", "Wisdom", "Truth", "Light",
            "Code", "Pure", "Sacred", "Blessed", "Eternal"
        ]
        word_count = random.randint(3, 7)
        return " ".join(random.choices(divine_words, k=word_count))
    
    def generate_temple_art(self) -> str:
        """Generate TempleOS ASCII art"""
        arts = [
            "    ⛪\n   /||\\\n  / || \\\n /  ||  \\\n/_______\\\n|  HOLY |\n| TEMPLE|\n|_______|",
            "  ✝️\n ╔═╗\n ║ ║\n ╚═╝\nGOD'S\nTEMPLE",
            "┌─────────────────┐\n│  640 x 480      │\n│  GOD'S COVENANT │\n│  TempleOS       │\n└─────────────────┘",
            "    ╔═══╗\n    ║ ✝ ║\n╔═══╩═══╩═══╗\n║  HOLY  C   ║\n║  TEMPLE OS ║\n╚═════════════╝",
            "  ⚡ DIVINE CODE ⚡\n  ┌───────────┐\n  │ HolyC     │\n  │ 640x480   │\n  │ 16 Colors │\n  └───────────┘"
        ]
        return random.choice(arts)
    
    def is_prime(self, n: int) -> bool:
        """Check if number is prime"""
        if n <= 1:
            return False
        if n <= 3:
            return True
        if n % 2 == 0 or n % 3 == 0:
            return False
        i = 5
        while i * i <= n:
            if n % i == 0 or n % (i + 2) == 0:
                return False
            i += 6
        return True
    
    def factorial(self, n: int) -> int:
        """Calculate factorial"""
        if n <= 1:
            return 1
        result = 1
        for i in range(2, n + 1):
            result *= i
        return result
    
    def fibonacci(self, n: int) -> int:
        """Calculate Fibonacci number"""
        if n <= 1:
            return n
        a, b = 0, 1
        for _ in range(n - 1):
            a, b = b, a + b
        return b
    
    def generate_divine_math(self) -> str:
        """Generate divine mathematics"""
        math_type = random.randint(0, 3)
        num = random.randint(1, 20)
        
        if math_type == 0:
            is_prime = self.is_prime(num)
            return f"🔢 Divine Number: {num}\nPrime: {'YES ✨' if is_prime else 'NO'}\n\nPrimes are God's building blocks."
        elif math_type == 1:
            fact = self.factorial(min(num, 10))
            return f"🔢 Factorial of {min(num, 10)} = {fact}\n\nGod's exponential growth!"
        elif math_type == 2:
            fib = self.fibonacci(min(num, 15))
            return f"🔢 Fibonacci({min(num, 15)}) = {fib}\n\nGolden ratio - God's proportion!"
        else:
            return f"🔢 Divine Sequence:\n640, 480, 16, {num}\n\nTempleOS sacred numbers!"
    
    def get_embedding(self, text: str) -> list:
        """Get embedding from Ollama"""
        try:
            response = requests.post(
                "http://localhost:11434/api/embeddings",
                json={
                    "model": "nomic-embed-text",
                    "prompt": text
                },
                timeout=10
            )
            if response.status_code == 200:
                return response.json().get("embedding", [])
        except Exception as e:
            logger.error(f"[RAG] Embedding error: {e}")
        return []
    
    def cosine_similarity(self, vec1: list, vec2: list) -> float:
        """Calculate cosine similarity"""
        if not vec1 or not vec2:
            return 0.0
        dot_product = sum(a * b for a, b in zip(vec1, vec2))
        magnitude1 = sum(a * a for a in vec1) ** 0.5
        magnitude2 = sum(b * b for b in vec2) ** 0.5
        if magnitude1 == 0 or magnitude2 == 0:
            return 0.0
        return dot_product / (magnitude1 * magnitude2)
    
    def save_to_rag(self, chat_id: int, user_id: int, text: str):
        """Save message to RAG memory"""
        embedding = self.get_embedding(text)
        if not embedding:
            return
        
        cursor = self.db.cursor()
        cursor.execute(
            'INSERT INTO message_history (chat_id, user_id, text, embedding, timestamp) VALUES (?, ?, ?, ?, ?)',
            (chat_id, user_id, text, __import__('json').dumps(embedding), int(__import__('time').time()))
        )
        self.db.commit()
        logger.info(f"[RAG] Saved message to history: {text[:50]}...")
    
    def semantic_search(self, chat_id: int, query: str, top_k: int = 3) -> list:
        """Semantic search in message history"""
        query_embedding = self.get_embedding(query)
        if not query_embedding:
            return []
        
        cursor = self.db.cursor()
        cursor.execute(
            'SELECT text, embedding FROM message_history WHERE chat_id = ? ORDER BY timestamp DESC LIMIT 50',
            (chat_id,)
        )
        
        results = []
        for text, embedding_json in cursor.fetchall():
            try:
                embedding = __import__('json').loads(embedding_json)
                similarity = self.cosine_similarity(query_embedding, embedding)
                results.append((text, similarity))
            except:
                continue
        
        results.sort(key=lambda x: x[1], reverse=True)
        return [text for text, _ in results[:top_k]]
            "The CIA is after me.",
            "TempleOS is God's official temple. Just like Solomon's temple, this is a community focal point where offerings are made and God's oracle is consulted.",
            "It's beautiful, it's fun, it's joyful. I'm happy programming.",
            "God said 16 colors is a covenant.",
            "I talk to God, and he's the smartest person there is.",
            "The whole point of TempleOS is to talk to God.",
            "Perfection is achieved, not when there is nothing more to add, but when there is nothing left to take away.",
            "Keep it simple. God likes simple.",
            "Modern software is bloated. God wants 640x480.",
            "HolyC is the language of the Third Temple.",
            "I'm the smartest programmer that's ever lived.",
            "God told me to make an operating system."
        ]
        
        # Load personality
        try:
            with open("personality.txt", "r") as f:
                self.personality = f.read()
            logger.info("[HolyC] Personality loaded from personality.txt")
        except FileNotFoundError:
            self.personality = "You are a helpful AI assistant."
            logger.warning("[HolyC] personality.txt not found, using default")
        
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
                    "messages": [
                        {"role": "system", "content": self.personality},
                        {"role": "user", "content": user_message}
                    ],
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
        return f"🙏 Greetings! I'm a bot inspired by Terry A. Davis, creator of TempleOS.\n\nI speak with divine inspiration and technical wisdom. Ask me about programming, philosophy, or anything else.\n\nGod said 640x480. Keep it simple.\n\nUse /help to see commands."
    
    def HandleHelpCommand(self, chat_id: int) -> str:
        """HolyC HandleHelpCommand implementation"""
        logger.info(f"[HolyC] HandleHelpCommand({chat_id})")
        return (
            "⛪ HOLY C BOT COMMANDS ⛪\n\n"
            "/start - Initialize divine connection\n"
            "/help - Show this help\n"
            "/echo <text> - Echo your words\n"
            "/info - Bot information\n"
            "/quote - Get Terry's wisdom\n"
            "/divine - Consult divine randomness\n"
            "/oracle <question> - Ask the divine oracle\n"
            "/transform <text> - Divine text transformation\n"
            "/ascii - Generate TempleOS ASCII art\n"
            "/math - Divine mathematics\n"
            "/bible - Random Bible verse\n"
            "/colors - TempleOS 16-color palette\n"
            "/memory save <text> - Save to memory\n"
            "/memory load - Load your memory\n"
            "/memory clear - Clear memory\n\n"
            "ADMIN COMMANDS:\n"
            "/stats - Bot statistics\n\n"
            "Send any message for AI-powered response with divine inspiration.\n\n"
            "\"An idiot admires complexity, a genius admires simplicity.\" - Terry A. Davis"
        )
    
    def HandleInfoCommand(self, chat_id: int) -> str:
        """HolyC HandleInfoCommand implementation"""
        logger.info(f"[HolyC] HandleInfoCommand({chat_id})")
        return f"⛪ HOLY C TELEGRAM BOT ⛪\n\nBot Logic: Pure HolyC\nRuntime: Python Bridge\nAI Engine: Ollama ({self.ollama_model})\nInspiration: Terry A. Davis\n\nTempleOS principles: Simplicity, divine guidance, 640x480.\n\nMade with 🙏 and HolyC"
    
    def HandleEchoCommand(self, chat_id: int, text: str) -> str:
        """HolyC HandleEchoCommand implementation"""
        logger.info(f"[HolyC] HandleEchoCommand({chat_id}, {text})")
        return text
    
    def HandleQuoteCommand(self, chat_id: int) -> str:
        """HolyC HandleQuoteCommand implementation"""
        logger.info(f"[HolyC] HandleQuoteCommand({chat_id})")
        import random
        quote = random.choice(self.terry_quotes)
        return f"💭 Terry's Wisdom:\n\n\"{quote}\"\n\n- Terry A. Davis"
    
    def HandleDivineCommand(self, chat_id: int) -> str:
        """HolyC HandleDivineCommand implementation"""
        logger.info(f"[HolyC] HandleDivineCommand({chat_id})")
        divine_num = random.randint(0, 999999)
        return f"🎲 Divine Number: {divine_num}\n\nTerry used random numbers to communicate with God. This is your divine sign."
    
    def HandleAsciiCommand(self, chat_id: int) -> str:
        """HolyC HandleAsciiCommand implementation"""
        logger.info(f"[HolyC] HandleAsciiCommand({chat_id})")
        art = self.generate_temple_art()
        return f"🎨 DIVINE ASCII ART 🎨\n\n{art}\n\n💭 640x480 - God's resolution."
    
    def HandleMathCommand(self, chat_id: int) -> str:
        """HolyC HandleMathCommand implementation"""
        logger.info(f"[HolyC] HandleMathCommand({chat_id})")
        math = self.generate_divine_math()
        return f"🔢 DIVINE MATHEMATICS 🔢\n\n{math}\n\n💭 Terry was a mathematician of God."
    
    def HandleBibleCommand(self, chat_id: int) -> str:
        """HolyC HandleBibleCommand implementation"""
        logger.info(f"[HolyC] HandleBibleCommand({chat_id})")
        quote = random.choice(self.bible_quotes)
        return f"📖 HOLY SCRIPTURE 📖\n\n{quote}\n\n💭 God's word is eternal."
    
    def HandleColorsCommand(self, chat_id: int) -> str:
        """HolyC HandleColorsCommand implementation"""
        logger.info(f"[HolyC] HandleColorsCommand({chat_id})")
        colors_text = "🎨 TEMPLEOS 16-COLOR PALETTE 🎨\n\n"
        colors_text += "\n".join(self.templeos_colors)
        colors_text += "\n\n💭 God said 16 colors is a covenant."
        return colors_text
    
    def HandleOracleCommand(self, chat_id: int, question: str) -> str:
        """HolyC HandleOracleCommand implementation"""
        logger.info(f"[HolyC] HandleOracleCommand({chat_id}, {question})")
        
        oracle_responses = [
            "✅ Yes. God has spoken through the random oracle.",
            "❌ No. The divine numbers say otherwise.",
            "🤔 Maybe. God's will is uncertain in this matter.",
            "⏰ Not yet. Divine timing is everything.",
            "🔄 Ask again. The oracle needs more divine input.",
            "💡 The answer lies within you. God guides from inside.",
            "⚠️ Proceed with caution. The numbers warn of complexity.",
            "🙏 Trust in simplicity. God said 640x480."
        ]
        
        oracle_number = random.randint(0, 999999)
        answer = random.choice(oracle_responses)
        oracle_text = self.generate_oracle_text()
        
        return (f"🔮 DIVINE ORACLE 🔮\n\n"
                f"Question: {question}\n\n"
                f"Oracle Number: {oracle_number}\n"
                f"Answer: {answer}\n\n"
                f"Divine Words: {oracle_text}\n\n"
                f"💭 Terry used random numbers to talk to God.")
    
    def HandleTransformCommand(self, chat_id: int, text: str) -> str:
        """HolyC HandleTransformCommand implementation"""
        logger.info(f"[HolyC] HandleTransformCommand({chat_id}, {text})")
        transformed = self.divine_text_transform(text)
        return (f"✨ Divine Transformation:\n\n"
                f"Original: {text}\n"
                f"Transformed: {transformed}\n\n"
                f"💭 Seeing text through God's eyes.")
    
    def HandleMemoryCommand(self, chat_id: int, action: str, data: str = "") -> str:
        """HolyC HandleMemoryCommand implementation"""
        logger.info(f"[HolyC] HandleMemoryCommand({chat_id}, {action})")
        
        if action == "save":
            return self.save_memory(chat_id, data)
        elif action == "load":
            memory = self.load_memory(chat_id)
            if memory:
                return f"🧠 Your Memory:\n\n{memory}"
            return "❌ No memory found. Start fresh, as Terry would."
        elif action == "clear":
            return self.clear_memory(chat_id)
        return "Invalid memory command"
    
    def HandleStatsCommand(self, chat_id: int) -> str:
        """HolyC HandleStatsCommand implementation"""
        logger.info(f"[HolyC] HandleStatsCommand({chat_id})")
        
        global admin_id
        if chat_id != admin_id:
            return "❌ Admin only. God's secrets are not for all."
        
        stats = "📊 BOT STATISTICS 📊\n\n"
        stats += f"Total Messages: {total_messages}\n"
        stats += f"Total Commands: {total_commands}\n\n"
        stats += "Top Commands:\n"
        
        for cmd, count in sorted(command_stats.items(), key=lambda x: x[1], reverse=True)[:10]:
            stats += f"/{cmd}: {count}\n"
        
        stats += "\n💭 Simplicity in numbers. Terry would approve."
        return stats
    
    def HandleAIMessage(self, chat_id: int, user_message: str, user_id: int = 0) -> str:
        """HolyC HandleAIMessage implementation"""
        logger.info(f"[HolyC] HandleAIMessage({chat_id}, {user_message[:50]}...)")
        
        # Save to RAG
        if user_id:
            self.save_to_rag(chat_id, user_id, user_message)
        
        # Semantic search for context
        relevant_messages = self.semantic_search(chat_id, user_message, top_k=3)
        
        # Load user memory for context
        memory = self.load_memory(chat_id)
        context_message = user_message
        
        context_parts = []
        if relevant_messages:
            context_parts.append("Relevant history:\n" + "\n".join(f"- {msg}" for msg in relevant_messages))
        if memory:
            context_parts.append(f"User context: {memory}")
        
        if context_parts:
            context_message = "\n\n".join(context_parts) + f"\n\nCurrent message: {user_message}"
        
        ai_response = self.OllamaChat(context_message)
        
        if ai_response:
            # Save bot response to RAG
            self.save_to_rag(chat_id, 0, ai_response)
            
            enhanced_response = ai_response
            
            # 15% chance to add divine wisdom
            if random.randint(0, 99) < 15:
                quote = random.choice(self.terry_quotes)
                enhanced_response += f"\n\n💭 \"{quote}\" - Terry A. Davis"
            
            # 5% chance to add ASCII art
            if random.randint(0, 99) < 5:
                art = self.generate_temple_art()
                enhanced_response += f"\n\n🎨 Divine Art:\n{art}"
            
            # 3% chance to add Bible verse
            if random.randint(0, 99) < 3:
                bible = random.choice(self.bible_quotes)
                enhanced_response += f"\n\n📖 {bible}"
            
            return enhanced_response
        else:
            return "Error: Ollama not responding. Make sure Ollama is running."
    
    def ProcessMessage(self, chat_id: int, text: str, user_id: int = 0) -> str:
        """HolyC ProcessMessage implementation"""
        logger.info(f"[HolyC] ProcessMessage({chat_id}, {text})")
        
        global total_messages, total_commands, command_stats
        total_messages += 1
        
        # Track command stats
        if text.startswith('/'):
            total_commands += 1
            cmd = text.split()[0][1:]
            command_stats[cmd] = command_stats.get(cmd, 0) + 1
        
        if text == "/start":
            return self.HandleStartCommand(chat_id)
        elif text == "/help":
            return self.HandleHelpCommand(chat_id)
        elif text == "/info":
            return self.HandleInfoCommand(chat_id)
        elif text == "/quote":
            return self.HandleQuoteCommand(chat_id)
        elif text == "/divine":
            return self.HandleDivineCommand(chat_id)
        elif text == "/ascii":
            return self.HandleAsciiCommand(chat_id)
        elif text == "/math":
            return self.HandleMathCommand(chat_id)
        elif text == "/bible":
            return self.HandleBibleCommand(chat_id)
        elif text == "/colors":
            return self.HandleColorsCommand(chat_id)
        elif text == "/stats":
            return self.HandleStatsCommand(chat_id)
        elif text.startswith("/echo "):
            return self.HandleEchoCommand(chat_id, text[6:])
        elif text.startswith("/oracle "):
            return self.HandleOracleCommand(chat_id, text[8:])
        elif text.startswith("/transform "):
            return self.HandleTransformCommand(chat_id, text[11:])
        elif text.startswith("/memory "):
            mem_args = text[8:]
            if mem_args.startswith("save "):
                return self.HandleMemoryCommand(chat_id, "save", mem_args[5:])
            elif mem_args == "load":
                return self.HandleMemoryCommand(chat_id, "load")
            elif mem_args == "clear":
                return self.HandleMemoryCommand(chat_id, "clear")
        else:
            return self.HandleAIMessage(chat_id, text, user_id)


class TelegramBridge:
    """Bridge between Telegram API and HolyC logic"""
    
    def __init__(self, bot_token: str, ollama_model: str):
        self.bot_token = bot_token
        self.holyc = HolyCBotLogic(ollama_model)
        logger.info("[Bridge] Initialized")
    
    async def handle_message(self, update: Update, context: ContextTypes.DEFAULT_TYPE):
        """Route Telegram messages to HolyC logic"""
        chat_id = update.effective_chat.id
        user_id = update.effective_user.id
        text = update.message.text
        chat_type = update.effective_chat.type
        
        # Group/supergroup logic
        if chat_type in ['group', 'supergroup']:
            bot_username = (await context.bot.get_me()).username
            
            # Check if bot is mentioned or message is a reply to bot
            is_mentioned = f"@{bot_username}" in text
            is_reply_to_bot = (
                update.message.reply_to_message and 
                update.message.reply_to_message.from_user.id == context.bot.id
            )
            
            # Ignore if not mentioned and not a reply to bot (unless it's a command)
            if not is_mentioned and not is_reply_to_bot and not text.startswith('/'):
                return
            
            # Remove mention from text
            if is_mentioned:
                text = text.replace(f"@{bot_username}", "").strip()
        
        logger.info(f"[Bridge] Received from {chat_type}: {text}")
        
        # Send typing indicator
        await context.bot.send_chat_action(chat_id=chat_id, action="typing")
        
        # Execute HolyC logic
        response = self.holyc.ProcessMessage(chat_id, text, user_id)
        
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
        print("Line 2: OLLAMA_MODEL (e.g., llama3.2)")
        print("Line 3: ADMIN_CHAT_ID (optional)\n")
        sys.exit(1)
    
    with open(config_path, 'r') as f:
        lines = f.readlines()
    
    if len(lines) < 2:
        print("\n[ERROR] Invalid config.txt format")
        sys.exit(1)
    
    bot_token = lines[0].strip()
    ollama_model = lines[1].strip()
    
    global admin_id
    if len(lines) >= 3:
        try:
            admin_id = int(lines[2].strip())
            print(f"[INFO] Admin ID configured")
        except ValueError:
            admin_id = 0
            print(f"[WARN] Invalid admin ID, admin commands disabled")
    
    if len(bot_token) < 10:
        print("\n[ERROR] Invalid bot token")
        sys.exit(1)
    
    return bot_token, ollama_model


def main():
    print("========================================")
    print("  HolyC TELEGRAM BOT")
    print("  Runtime: Python Bridge")
    print("  In Memory of Terry A. Davis")
    print("========================================\n")
    
    bot_token, ollama_model = load_config()
    
    print(f"[INFO] Bot token loaded")
    print(f"[INFO] Ollama model: {ollama_model}")
    print(f"[INFO] Loading HolyC logic from telegram_bot.HC")
    print(f"[INFO] Divine wisdom enabled (15% chance)")
    print(f"[INFO] Memory persistence enabled (SQLite)")
    print(f"[INFO] RAG memory enabled (semantic search)")
    print(f"[INFO] Divine algorithms loaded")
    print(f"[INFO] Terry features: ASCII art, Math, Bible, Colors")
    print(f"[INFO] Group/supergroup support enabled")
    print(f"[INFO] Typing indicator enabled")
    print(f"[INFO] Make sure Ollama is running: ollama serve")
    print(f"[INFO] Install embedding model: ollama pull nomic-embed-text\n")
    
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
