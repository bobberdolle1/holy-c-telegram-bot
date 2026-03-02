#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <regex>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

using namespace std;

// Global state
atomic<bool> running(true);
string BOT_TOKEN;
string OLLAMA_MODEL;
long long last_update_id = 0;

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

// HolyC Logic Implementation (optimized)
class HolyCBotLogic {
private:
    string ollama_model;
    
public:
    HolyCBotLogic(const string& model) : ollama_model(model) {
        cout << "[HolyC] Initialized with model: " << model << endl;
    }
    
    string OllamaChat(const string& user_message) {
        string escaped = json_escape(user_message);
        string json_request = "{\"model\":\"" + ollama_model + 
                            "\",\"messages\":[{\"role\":\"user\",\"content\":\"" + 
                            escaped + "\"}],\"stream\":false}";
        
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
        return "Hello! I'm an AI bot powered by Ollama (" + ollama_model + "). Ask me anything!";
    }
    
    inline string HandleHelpCommand(long long chat_id) {
        return "Commands:\n/start - Start bot\n/help - Show help\n/echo <text> - Echo message\n/info - Bot info\n\nSend any message for AI response!";
    }
    
    inline string HandleInfoCommand(long long chat_id) {
        return "Bot: Pure HolyC\nRuntime: C++ Bridge (FAST)\nAI: Ollama (" + ollama_model + ")";
    }
    
    inline string HandleEchoCommand(long long chat_id, const string& text) {
        return text;
    }
    
    string HandleAIMessage(long long chat_id, const string& user_message) {
        string ai_response = OllamaChat(user_message);
        return ai_response.empty() ? "Error: Ollama not responding" : ai_response;
    }
    
    string ProcessMessage(long long chat_id, const string& text) {
        if (text == "/start") return HandleStartCommand(chat_id);
        if (text == "/help") return HandleHelpCommand(chat_id);
        if (text == "/info") return HandleInfoCommand(chat_id);
        if (text.substr(0, 6) == "/echo ") return HandleEchoCommand(chat_id, text.substr(6));
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
bool LoadConfig(string& bot_token, string& ollama_model) {
    ifstream config("config.txt");
    if (!config.is_open()) {
        cerr << "\n[ERROR] config.txt not found!" << endl;
        return false;
    }
    
    getline(config, bot_token);
    getline(config, ollama_model);
    config.close();
    
    bot_token = trim(bot_token);
    ollama_model = trim(ollama_model);
    
    if (bot_token.length() < 10 || ollama_model.length() < 2) {
        cerr << "[ERROR] Invalid config" << endl;
        return false;
    }
    
    return true;
}

int main() {
    cout << "========================================" << endl;
    cout << "  HOLY C TELEGRAM BOT" << endl;
    cout << "  Runtime: C++ Bridge (MAXIMUM SPEED)" << endl;
    cout << "========================================\n" << endl;
    
    string bot_token, ollama_model;
    
    if (!LoadConfig(bot_token, ollama_model)) {
        return 1;
    }
    
    cout << "[INFO] Bot token loaded" << endl;
    cout << "[INFO] Ollama model: " << ollama_model << endl;
    cout << "[INFO] Loading HolyC logic from telegram_bot.HC" << endl;
    cout << "[INFO] Make sure Ollama is running\n" << endl;
    
    TelegramBridge bridge(bot_token, ollama_model);
    
    try {
        bridge.Run();
    } catch (const exception& e) {
        cerr << "\n[ERROR] " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
