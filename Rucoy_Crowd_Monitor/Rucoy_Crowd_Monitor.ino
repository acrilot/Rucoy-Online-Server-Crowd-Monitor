#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <map>

#define LED_PIN 19
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "insert your wifi ssid";
const char* password = "insert your wifi password";

// Server URL
const String url = "https://www.rucoyonline.com/server_list.json";

// Map to store previous characters_online data for change detection
std::map<String, int> previousServerData;

String abbreviateServerName(String name) {
  if (name == "North America 1") return "N1";
  if (name == "North America 2") return "N2";
  if (name == "North America 3") return "N3";
  if (name == "North America 4") return "N4";
  if (name == "North America 5") return "N5";
  if (name == "North America 6") return "N6";
  if (name == "South America 1") return "S1";
  if (name == "South America 2") return "S2";
  if (name == "South America 3") return "S3";
  if (name == "South America 4") return "S4";
  if (name == "South America 5") return "S5";
  if (name == "South America 6") return "S6";
  if (name == "South America 7") return "S7";
  if (name == "South America 8") return "S8";
  if (name == "Europe 1") return "E1";
  if (name == "Europe 2") return "E2";
  if (name == "Europe 3") return "E3";
  if (name == "Europe 4") return "E4";
  if (name == "Europe 5") return "E5";
  if (name == "Europe 6") return "E6";
  if (name == "Asia 1") return "A1";
  if (name == "Asia 2") return "A2";
  if (name == "Asia 3") return "A3";
  if (name == "Asia 4") return "A4";
  return name;
}

// Helper function to fetch server data
String fetchServerData() {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    http.end();
    return payload;
  } else {
    http.end();
    return "";
  }
}

// Function to display top 3 servers on the OLED screen
void displayTopServers(String jsonData) {
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, jsonData);
  JsonArray servers = doc["servers"].as<JsonArray>();

  // Sort servers based on 'characters_online' in descending order
  std::vector<JsonObject> sortedServers;
  for (JsonObject server : servers) {
    sortedServers.push_back(server);
  }
  std::sort(sortedServers.begin(), sortedServers.end(), [](JsonObject a, JsonObject b) {
    return a["characters_online"].as<int>() > b["characters_online"].as<int>();
  });

  // Display top 3 servers
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  for (int i = 0; i < 4 && i < sortedServers.size(); i++) {
    JsonObject server = sortedServers[i];
    String serverName = abbreviateServerName(server["name"].as<String>());
    int charactersOnline = server["characters_online"].as<int>();
    display.printf("%d) %s-%d\n", i + 1, serverName.c_str(), charactersOnline);
  }
  display.display();
}

bool detectAndBlinkLED(String jsonData) {
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, jsonData);
  JsonArray servers = doc["servers"].as<JsonArray>();

  bool dataChanged = false;
  for (JsonObject server : servers) {
    String serverName = server["name"].as<String>();
    int charactersOnline = server["characters_online"].as<int>();

    // Check for changes in server data
    if (previousServerData[serverName] != charactersOnline) {
      previousServerData[serverName] = charactersOnline;
      dataChanged = true;
    }
  }

  // Blink LED if data has changed
  if (dataChanged) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  return dataChanged;
}

void setup() {
  // Initialize Serial, WiFi, OLED display, and LED
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void loop() {
  String jsonData = fetchServerData();
  if (!jsonData.isEmpty()) {
    if (detectAndBlinkLED(jsonData)) {
      displayTopServers(jsonData);
    }
  } else {
    Serial.println("Failed to fetch server data.");
  }

  delay(1); // Update every second
}
