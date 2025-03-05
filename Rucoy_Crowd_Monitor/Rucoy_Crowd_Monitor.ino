#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <map>

#define LED_PIN 19
#define BUTTON_PIN 18
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "insert your wifi ssid";
const char* password = "insert your wifi password";
const String url = "https://www.rucoyonline.com/server_list.json";

// Map to store previous characters_online data for change detection
std::map<String, int> previousServerData;

// Toggle status
bool showTopServers = true;

String abbreviateServerName(String name) {
  if (name.startsWith("North America")) return "N" + name.substring(14);
  if (name.startsWith("South America")) return "S" + name.substring(14);
  if (name.startsWith("Europe")) return "E" + name.substring(7);
  if (name.startsWith("Asia")) return "A" + name.substring(5);
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

// Function to display top 4 servers on the OLED screen
void displayServers(String jsonData, bool top) {
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, jsonData);
  JsonArray servers = doc["servers"].as<JsonArray>();
  
  std::vector<JsonObject> sortedServers;
  for (JsonObject server : servers) sortedServers.push_back(server);
  
  std::sort(sortedServers.begin(), sortedServers.end(), [](JsonObject a, JsonObject b) {
    return a["characters_online"].as<int>() > b["characters_online"].as<int>();
  });
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  for (int i = 0; i < 4; i++) {
    JsonObject server = top ? sortedServers[i] : sortedServers[sortedServers.size() - 1 - i];
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
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void loop() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);
  
  //if (buttonState == LOW && lastButtonState == HIGH) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      showTopServers = !showTopServers;
      Serial.println(showTopServers ? "Showing Top 4 Servers" : "Showing Bottom 4 Servers");
      displayServers(fetchServerData(), showTopServers);
    }
  //}
  //lastButtonState = buttonState;

  String jsonData = fetchServerData();
  if (!jsonData.isEmpty()) {
    if (detectAndBlinkLED(jsonData)) {
      displayServers(jsonData, showTopServers);
    }
  } else {
    Serial.println("Failed to fetch server data.");
  }
  delay(1);
}
