// Weather Plugin - Example HolyC Plugin
// Command: /weather <city>

U0 HandleWeatherCommand(I64 chat_id, U8 *city) {
  // Divine weather prediction using random oracle
  I64 divine_temp = (GetDivineNumber() % 40) - 10; // -10 to 30°C
  I64 divine_condition = GetDivineNumber() % 5;
  
  U8 *conditions[] = {
    "☀️ Sunny (God's blessing)",
    "⛅ Partly Cloudy (Divine balance)",
    "☁️ Cloudy (God's contemplation)",
    "🌧️ Rainy (Heaven's tears)",
    "⛈️ Stormy (Divine wrath)"
  };
  
  U8 *condition = conditions[divine_condition];
  
  U8 *message = StrFormat(
    "🌤️ Divine Weather for %s:\n\nTemperature: %d°C\nCondition: %s\n\n💭 Weather is God's will. Terry knew this.",
    city, divine_temp, condition
  );
  
  SendMessage(chat_id, message);
}
