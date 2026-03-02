#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <ctime>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <windows.h>
#include <wininet.h>
#include <sqlite3.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "sqlite3.lib")

using namespace std;

// Global state
atomic<bool> running(true);
string BOT_TOKEN;
string OLLAMA_MODEL;
string PERSONALITY_PROMPT;
long long ADMIN_ID = 0;
long long last_update_id = 0;

// Stats tracking
atomic<long long> total_messages(0);
atomic<long long> total_commands(0);
map<string, int> command_stats;
sqlite3* db = nullptr;
string OLLAMA_MODEL;
string PERSONALITY_PROMPT;
long long last_update_id = 0;

// Terry's quotes for divine wisdom
const char* terry_quotes[] = {
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
};
const int terry_quotes_count = 15;

// Bible quotes
const char* bible_quotes[] = {
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
};
const int bible_quotes_count = 10;

// TempleOS 16-color palette
const char* templeos_colors[] = {
    "⬛ BLACK", "🔵 BLUE", "🟢 GREEN", "🔷 CYAN",
    "🔴 RED", "🟣 PURPLE", "🟡 YELLOW", "⚪ WHITE",
    "⬜ GRAY", "💙 LIGHT_BLUE", "💚 LIGHT_GREEN", "💎 LIGHT_CYAN",
    "❤️ LIGHT_RED", "💜 LIGHT_PURPLE", "💛 LIGHT_YELLOW", "✨ BRIGHT_WHITE"
};
const int templeos_colors_count = 16;

// Fast string utilities
inline string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

inline string json_escape(const string& input) {
    string output;
    output.reserve(input.size() * 2);
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': break;
            case '\t': output += "\\t"; break;
            default: output += c;
        }
    }
    return output;
}

// Fast HTTP client using WinINet
string http_request(const string& url, const string& post_data = "", bool is_https = true) {
    HINTERNET hInternet = InternetOpenA("HolyCBridge/2.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";
    
    DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD;
    if (is_https) flags |= INTERNET_FLAG_SECURE;
    
    HINTERNET hRequest = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
    
    if (hRequest && !post_data.empty()) {
        string headers = "Content-Type: application/json\r\n";
        HttpSendRequestA(hRequest, headers.c_str(), headers.length(), 
                        (LPVOID)post_data.c_str(), post_data.length());
    }
    
    string result;
    if (hRequest) {
        char buffer[8192];
        DWORD bytesRead;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
        }
        InternetCloseHandle(hRequest);
    }
    
    InternetCloseHandle(hInternet);
    return result;
}

// Fast JSON extraction (optimized for known structure)
inline long long extract_json_int(const string& json, const string& key) {
    size_t pos = json.find("\"" + key + "\":");
    if (pos == string::npos) return 0;
    pos += key.length() + 3;
    return stoll(json.substr(pos, json.find_first_of(",}", pos) - pos));
}

inline string extract_json_string(const string& json, const string& key) {
    size_t start = json.find("\"" + key + "\":\"");
    if (start == string::npos) return "";
    start += key.length() + 4;
    size_t end = start;
    while (end < json.length() && (json[end] != '"' || json[end-1] == '\\')) end++;
    return json.substr(start, end - start);
}

// Divine randomness (Terry's way)
inline int GetDivineNumber() {
    return rand();
}

inline string GetDivineQuote() {
    if (GetDivineNumber() % 100 < 15) {
        int quote_index = GetDivineNumber() % terry_quotes_count;
        return terry_quotes[quote_index];
    }
    return "";
}

inline string StrFormat(const string& response, const string& quote) {
    return response + "\n\n💭 \"" + quote + "\" - Terry A. Davis";
}

// SQLite Memory Functions
bool InitDatabase() {
    int rc = sqlite3_open("memory.db", &db);
    if (rc) {
        cerr << "[ERROR] Cannot open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS user_memory ("
        "chat_id INTEGER PRIMARY KEY,"
        "data TEXT,"
        "timestamp INTEGER);";
    
    char* err_msg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        cerr << "[ERROR] SQL error: " << err_msg << endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    cout << "[INFO] Memory database initialized" << endl;
    return true;
}

string SaveMemory(long long chat_id, const string& data) {
    string sql = "INSERT OR REPLACE INTO user_memory (chat_id, data, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "Error saving memory";
    }
    
    sqlite3_bind_int64(stmt, 1, chat_id);
    sqlite3_bind_text(stmt, 2, data.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, time(nullptr));
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return "💾 Memory saved in the divine database.";
}

string LoadMemory(long long chat_id) {
    string sql = "SELECT data FROM user_memory WHERE chat_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "";
    }
    
    sqlite3_bind_int64(stmt, 1, chat_id);
    
    string result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return result;
}

string ClearMemory(long long chat_id) {
    string sql = "DELETE FROM user_memory WHERE chat_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "Error clearing memory";
    }
    
    sqlite3_bind_int64(stmt, 1, chat_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return "🗑️ Memory cleared. Simplicity restored.";
}

// Divine text processing
string DivineTextTransform(const string& input) {
    int transform_type = GetDivineNumber() % 3;
    string result = input;
    
    if (transform_type == 0) {
        transform(result.begin(), result.end(), result.begin(), ::toupper);
    } else if (transform_type == 1) {
        transform(result.begin(), result.end(), result.begin(), ::tolower);
    } else {
        reverse(result.begin(), result.end());
    }
    
    return result;
}

string GenerateOracleText() {
    const char* divine_words[] = {
        "God", "Temple", "Simple", "640x480", "Holy",
        "Divine", "Covenant", "Wisdom", "Truth", "Light",
        "Code", "Pure", "Sacred", "Blessed", "Eternal"
    };
    
    int oracle_num = GetDivineNumber();
    int word_count = (oracle_num % 5) + 3;
    string result;
    
    for (int i = 0; i < word_count; i++) {
        int word_idx = (oracle_num + i * 7) % 15;
        if (i > 0) result += " ";
        result += divine_words[word_idx];
    }
    
    return result;
}

// ASCII Art generation
string GenerateTempleArt() {
    const char* arts[] = {
        "    ⛪\n   /||\\\n  / || \\\n /  ||  \\\n/_______\\\n|  HOLY |\n| TEMPLE|\n|_______|",
        "  ✝️\n ╔═╗\n ║ ║\n ╚═╝\nGOD'S\nTEMPLE",
        "┌─────────────────┐\n│  640 x 480      │\n│  GOD'S COVENANT │\n│  TempleOS       │\n└─────────────────┘",
        "    ╔═══╗\n    ║ ✝ ║\n╔═══╩═══╩═══╗\n║  HOLY  C   ║\n║  TEMPLE OS ║\n╚═════════════╝",
        "  ⚡ DIVINE CODE ⚡\n  ┌───────────┐\n  │ HolyC     │\n  │ 640x480   │\n  │ 16 Colors │\n  └───────────┘"
    };
    
    int art_idx = GetDivineNumber() % 5;
    return arts[art_idx];
}

// Divine mathematics
bool IsPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

long long Factorial(int n) {
    if (n <= 1) return 1;
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

long long Fibonacci(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 1; i < n; i++) {
        long long temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

string GenerateDivineMath() {
    int math_type = GetDivineNumber() % 4;
    int num = (GetDivineNumber() % 20) + 1;
    
    if (math_type == 0) {
        bool is_prime = IsPrime(num);
        return "🔢 Divine Number: " + to_string(num) + "\nPrime: " + 
               (is_prime ? "YES ✨" : "NO") + "\n\nPrimes are God's building blocks.";
    } else if (math_type == 1) {
        int n = num % 10;
        long long fact = Factorial(n);
        return "🔢 Factorial of " + to_string(n) + " = " + to_string(fact) + 
               "\n\nGod's exponential growth!";
    } else if (math_type == 2) {
        int n = num % 15;
        long long fib = Fibonacci(n);
        return "🔢 Fibonacci(" + to_string(n) + ") = " + to_string(fib) + 
               "\n\nGolden ratio - God's proportion!";
    } else {
        return "🔢 Divine Sequence:\n640, 480, 16, " + to_string(num) + 
               "\n\nTempleOS sacred numbers!";
    }
}

// HolyC Logic Implementation (optimized)
class HolyCBotLogic {
private:
    string ollama_model;
    
public:
    HolyCBotLogic(const string& model) : ollama_model(model) {
        cout << "[HolyC] Initialized with model: " << model << endl;
    }
    
    string OllamaChat(const string& user_message) {
        string escaped_msg = json_escape(user_message);
        string escaped_personality = json_escape(PERSONALITY_PROMPT);
        
        string json_request = "{\"model\":\"" + ollama_model + 
                            "\",\"messages\":[" +
                            "{\"role\":\"system\",\"content\":\"" + escaped_personality + "\"}," +
                            "{\"role\":\"user\",\"content\":\"" + escaped_msg + "\"}" +
                            "],\"stream\":false}";
        
        string response = http_request("http://localhost:11434/api/chat", json_request, false);
        
        if (response.empty()) return "";
        
        size_t content_pos = response.find("\"content\":\"");
        if (content_pos == string::npos) return "";
        
        content_pos += 11;
        size_t end_pos = content_pos;
        while (end_pos < response.length() && 
               (response[end_pos] != '"' || response[end_pos-1] == '\\')) {
            end_pos++;
        }
        }
        
        string content = response.substr(content_pos, end_pos - content_pos);
        
        // Unescape \n
        size_t pos = 0;
        while ((pos = content.find("\\n", pos)) != string::npos) {
            content.replace(pos, 2, "\n");
            pos++;
        }
        
        return content;
    }
    
    inline string HandleStartCommand(long long chat_id) {
        return "🙏 Greetings! I'm a bot inspired by Terry A. Davis, creator of TempleOS.\n\nI speak with divine inspiration and technical wisdom. Ask me about programming, philosophy, or anything else.\n\nGod said 640x480. Keep it simple.\n\nUse /help to see commands.";
    }
    
    inline string HandleHelpCommand(long long chat_id) {
        return "⛪ HOLY C BOT COMMANDS ⛪\n\n/start - Initialize divine connection\n/help - Show this help\n/echo <text> - Echo your words\n/info - Bot information\n/quote - Get Terry's wisdom\n/divine - Consult divine randomness\n/oracle <question> - Ask the divine oracle\n/transform <text> - Divine text transformation\n/ascii - Generate TempleOS ASCII art\n/math - Divine mathematics\n/bible - Random Bible verse\n/colors - TempleOS 16-color palette\n/memory save <text> - Save to memory\n/memory load - Load your memory\n/memory clear - Clear memory\n\nADMIN COMMANDS:\n/stats - Bot statistics\n\nSend any message for AI-powered response with divine inspiration.\n\n\"An idiot admires complexity, a genius admires simplicity.\" - Terry A. Davis";
    }
    
    inline string HandleInfoCommand(long long chat_id) {
        return "⛪ HOLY C TELEGRAM BOT ⛪\n\nBot Logic: Pure HolyC\nRuntime: C++ Bridge (FAST)\nAI Engine: Ollama (" + ollama_model + ")\nInspiration: Terry A. Davis\n\nTempleOS principles: Simplicity, divine guidance, 640x480.\n\nMade with 🙏 and HolyC";
    }
    
    inline string HandleEchoCommand(long long chat_id, const string& text) {
        return text;
    }
    
    inline string HandleQuoteCommand(long long chat_id) {
        int quote_index = GetDivineNumber() % terry_quotes_count;
        return string("💭 Terry's Wisdom:\n\n\"") + terry_quotes[quote_index] + "\"\n\n- Terry A. Davis";
    }
    
    inline string HandleDivineCommand(long long chat_id) {
        int divine_num = GetDivineNumber();
        return "🎲 Divine Number: " + to_string(divine_num) + "\n\nTerry used random numbers to communicate with God. This is your divine sign.";
    }
    
    inline string HandleAsciiCommand(long long chat_id) {
        string art = GenerateTempleArt();
        return "🎨 DIVINE ASCII ART 🎨\n\n" + art + "\n\n💭 640x480 - God's resolution.";
    }
    
    inline string HandleMathCommand(long long chat_id) {
        string math = GenerateDivineMath();
        return "🔢 DIVINE MATHEMATICS 🔢\n\n" + math + "\n\n💭 Terry was a mathematician of God.";
    }
    
    inline string HandleBibleCommand(long long chat_id) {
        int quote_index = GetDivineNumber() % bible_quotes_count;
        return string("📖 HOLY SCRIPTURE 📖\n\n") + bible_quotes[quote_index] + "\n\n💭 God's word is eternal.";
    }
    
    inline string HandleColorsCommand(long long chat_id) {
        string result = "🎨 TEMPLEOS 16-COLOR PALETTE 🎨\n\n";
        for (int i = 0; i < templeos_colors_count; i++) {
            result += string(templeos_colors[i]) + "\n";
        }
        result += "\n💭 God said 16 colors is a covenant.";
        return result;
    }
    
    inline string HandleOracleCommand(long long chat_id, const string& question) {
        const char* oracle_responses[] = {
            "✅ Yes. God has spoken through the random oracle.",
            "❌ No. The divine numbers say otherwise.",
            "🤔 Maybe. God's will is uncertain in this matter.",
            "⏰ Not yet. Divine timing is everything.",
            "🔄 Ask again. The oracle needs more divine input.",
            "💡 The answer lies within you. God guides from inside.",
            "⚠️ Proceed with caution. The numbers warn of complexity.",
            "🙏 Trust in simplicity. God said 640x480."
        };
        
        int oracle_number = GetDivineNumber();
        int answer_type = oracle_number % 8;
        string oracle_text = GenerateOracleText();
        
        return "🔮 DIVINE ORACLE 🔮\n\nQuestion: " + question + 
               "\n\nOracle Number: " + to_string(oracle_number) +
               "\nAnswer: " + oracle_responses[answer_type] +
               "\n\nDivine Words: " + oracle_text +
               "\n\n💭 Terry used random numbers to talk to God.";
    }
    
    inline string HandleTransformCommand(long long chat_id, const string& text) {
        string transformed = DivineTextTransform(text);
        return "✨ Divine Transformation:\n\nOriginal: " + text +
               "\nTransformed: " + transformed +
               "\n\n💭 Seeing text through God's eyes.";
    }
    
    inline string HandleMemoryCommand(long long chat_id, const string& action, const string& data) {
        if (action == "save") {
            return SaveMemory(chat_id, data);
        } else if (action == "load") {
            string memory = LoadMemory(chat_id);
            if (!memory.empty()) {
                return "🧠 Your Memory:\n\n" + memory;
            }
            return "❌ No memory found. Start fresh, as Terry would.";
        } else if (action == "clear") {
            return ClearMemory(chat_id);
        }
        return "Invalid memory command";
    }
    
    inline string HandleStatsCommand(long long chat_id) {
        if (chat_id != ADMIN_ID) {
            return "❌ Admin only. God's secrets are not for all.";
        }
        
        string stats = "📊 BOT STATISTICS 📊\n\n";
        stats += "Total Messages: " + to_string(total_messages.load()) + "\n";
        stats += "Total Commands: " + to_string(total_commands.load()) + "\n\n";
        stats += "Top Commands:\n";
        
        for (const auto& pair : command_stats) {
            stats += "/" + pair.first + ": " + to_string(pair.second) + "\n";
        }
        
        stats += "\n💭 Simplicity in numbers. Terry would approve.";
        return stats;
    }
    
    string HandleAIMessage(long long chat_id, const string& user_message) {
        // Load user memory for context
        string memory = LoadMemory(chat_id);
        string context_message = user_message;
        
        if (!memory.empty()) {
            context_message = "Context from memory: " + memory + "\n\nUser message: " + user_message;
        }
        
        string ai_response = OllamaChat(context_message);
        if (ai_response.empty()) return "Error: Ollama not responding. Make sure Ollama is running.";
        
        string enhanced_response = ai_response;
        
        // 15% chance to add divine wisdom
        string divine_quote = GetDivineQuote();
        if (!divine_quote.empty()) {
            enhanced_response = StrFormat(enhanced_response, divine_quote);
        }
        
        // 5% chance to add ASCII art
        if (GetDivineNumber() % 100 < 5) {
            string art = GenerateTempleArt();
            enhanced_response += "\n\n🎨 Divine Art:\n" + art;
        }
        
        // 3% chance to add Bible verse
        if (GetDivineNumber() % 100 < 3) {
            int bible_idx = GetDivineNumber() % bible_quotes_count;
            enhanced_response += "\n\n📖 " + string(bible_quotes[bible_idx]);
        }
        
        return enhanced_response;
    }
    
    string ProcessMessage(long long chat_id, const string& text) {
        total_messages++;
        
        // Track command stats
        if (text[0] == '/') {
            total_commands++;
            size_t space_pos = text.find(' ');
            string cmd = (space_pos != string::npos) ? text.substr(1, space_pos - 1) : text.substr(1);
            command_stats[cmd]++;
        }
        
        if (text == "/start") return HandleStartCommand(chat_id);
        if (text == "/help") return HandleHelpCommand(chat_id);
        if (text == "/info") return HandleInfoCommand(chat_id);
        if (text == "/quote") return HandleQuoteCommand(chat_id);
        if (text == "/divine") return HandleDivineCommand(chat_id);
        if (text == "/ascii") return HandleAsciiCommand(chat_id);
        if (text == "/math") return HandleMathCommand(chat_id);
        if (text == "/bible") return HandleBibleCommand(chat_id);
        if (text == "/colors") return HandleColorsCommand(chat_id);
        if (text == "/stats") return HandleStatsCommand(chat_id);
        if (text == "/info") return HandleInfoCommand(chat_id);
        if (text == "/quote") return HandleQuoteCommand(chat_id);
        if (text == "/divine") return HandleDivineCommand(chat_id);
        if (text == "/stats") return HandleStatsCommand(chat_id);
        
        if (text.substr(0, 6) == "/echo ") return HandleEchoCommand(chat_id, text.substr(6));
        if (text.substr(0, 8) == "/oracle ") return HandleOracleCommand(chat_id, text.substr(8));
        if (text.substr(0, 11) == "/transform ") return HandleTransformCommand(chat_id, text.substr(11));
        
        if (text.substr(0, 8) == "/memory ") {
            string mem_args = text.substr(8);
            if (mem_args.substr(0, 5) == "save ") {
                return HandleMemoryCommand(chat_id, "save", mem_args.substr(5));
            } else if (mem_args == "load") {
                return HandleMemoryCommand(chat_id, "load", "");
            } else if (mem_args == "clear") {
                return HandleMemoryCommand(chat_id, "clear", "");
            }
        }
        
        return HandleAIMessage(chat_id, text);
    }
};

// Telegram Bridge (optimized)
class TelegramBridge {
private:
    string bot_token;
    HolyCBotLogic holyc;
    
    void SendMessage(long long chat_id, const string& text) {
        string escaped = json_escape(text);
        string url = "https://api.telegram.org/bot" + bot_token + "/sendMessage";
        string post_data = "{\"chat_id\":" + to_string(chat_id) + 
                          ",\"text\":\"" + escaped + "\"}";
        
        http_request(url, post_data, true);
    }
    
    void ProcessUpdate(const string& update_json) {
        long long update_id = extract_json_int(update_json, "update_id");
        
        if (update_id > last_update_id) {
            last_update_id = update_id;
        }
        
        size_t message_pos = update_json.find("\"message\":");
        if (message_pos == string::npos) return;
        
        string message_part = update_json.substr(message_pos);
        long long chat_id = extract_json_int(message_part, "id");
        string text = extract_json_string(message_part, "text");
        
        if (text.empty()) return;
        
        cout << "[RECV] Chat " << chat_id << ": " << text << endl;
        
        string response = holyc.ProcessMessage(chat_id, text);
        
        cout << "[SEND] Response: " << response.substr(0, 50) << "..." << endl;
        
        SendMessage(chat_id, response);
    }
    
public:
    TelegramBridge(const string& token, const string& model) 
        : bot_token(token), holyc(model) {
        cout << "[Bridge] Initialized (C++ FAST MODE)" << endl;
    }
    
    void GetUpdates() {
        string url = "https://api.telegram.org/bot" + bot_token + 
                    "/getUpdates?offset=" + to_string(last_update_id + 1) + 
                    "&timeout=30";
        
        string response = http_request(url, "", true);
        
        if (response.empty()) return;
        
        size_t result_pos = response.find("\"result\":[");
        if (result_pos == string::npos) return;
        
        size_t update_pos = response.find("{\"update_id\":", result_pos);
        while (update_pos != string::npos) {
            size_t next_update = response.find("{\"update_id\":", update_pos + 1);
            
            string single_update;
            if (next_update != string::npos) {
                single_update = response.substr(update_pos, next_update - update_pos);
            } else {
                size_t end = response.find("]", update_pos);
                single_update = response.substr(update_pos, end - update_pos);
            }
            
            ProcessUpdate(single_update);
            update_pos = next_update;
        }
    }
    
    void Run() {
        cout << "\n[SUCCESS] HolyC Bot Running (C++ FAST MODE)!" << endl;
        cout << "Open Telegram and send /start\n" << endl;
        
        while (running) {
            GetUpdates();
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
};

// Config loader
bool LoadConfig(string& bot_token, string& ollama_model, string& personality, long long& admin_id) {
    ifstream config("config.txt");
    if (!config.is_open()) {
        cerr << "\n[ERROR] config.txt not found!" << endl;
        return false;
    }
    
    string admin_line;
    getline(config, bot_token);
    getline(config, ollama_model);
    getline(config, admin_line);
    config.close();
    
    bot_token = trim(bot_token);
    ollama_model = trim(ollama_model);
    admin_line = trim(admin_line);
    
    if (bot_token.length() < 10 || ollama_model.length() < 2) {
        cerr << "[ERROR] Invalid config" << endl;
        return false;
    }
    
    // Parse admin ID
    if (!admin_line.empty()) {
        try {
            admin_id = stoll(admin_line);
            cout << "[INFO] Admin ID configured" << endl;
        } catch (...) {
            admin_id = 0;
            cout << "[WARN] Invalid admin ID, admin commands disabled" << endl;
        }
    }
    
    // Load personality
    ifstream personality_file("personality.txt");
    if (personality_file.is_open()) {
        stringstream buffer;
        buffer << personality_file.rdbuf();
        personality = buffer.str();
        personality_file.close();
        cout << "[INFO] Personality loaded from personality.txt" << endl;
    } else {
        personality = "You are a helpful AI assistant.";
        cout << "[WARN] personality.txt not found, using default" << endl;
    }
    
    return true;
}

int main() {
    srand(time(NULL));
    
    cout << "========================================" << endl;
    cout << "  HOLY C TELEGRAM BOT" << endl;
    cout << "  Runtime: C++ Bridge (MAXIMUM SPEED)" << endl;
    cout << "  In Memory of Terry A. Davis" << endl;
    cout << "========================================\n" << endl;
    
    string bot_token, ollama_model;
    
    if (!LoadConfig(bot_token, ollama_model, PERSONALITY_PROMPT, ADMIN_ID)) {
        return 1;
    }
    
    BOT_TOKEN = bot_token;
    OLLAMA_MODEL = ollama_model;
    
    // Initialize database
    if (!InitDatabase()) {
        cerr << "[ERROR] Failed to initialize database" << endl;
        return 1;
    }
    
    cout << "[INFO] Bot token loaded" << endl;
    cout << "[INFO] Ollama model: " << ollama_model << endl;
    cout << "[INFO] Loading HolyC logic from telegram_bot.HC" << endl;
    cout << "[INFO] Divine wisdom enabled (15% chance)" << endl;
    cout << "[INFO] Memory persistence enabled (SQLite)" << endl;
    cout << "[INFO] Divine algorithms loaded" << endl;
    cout << "[INFO] Terry features: ASCII art, Math, Bible, Colors" << endl;
    cout << "[INFO] Random enhancements: 5% ASCII, 3% Bible" << endl;
    cout << "[INFO] Make sure Ollama is running\n" << endl;
    
    TelegramBridge bridge(bot_token, ollama_model);
    
    try {
        bridge.Run();
    } catch (const exception& e) {
        cerr << "\n[ERROR] " << e.what() << endl;
        sqlite3_close(db);
        return 1;
    }
    
    sqlite3_close(db);
    return 0;
}
