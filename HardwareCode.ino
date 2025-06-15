#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WiFi Credentials
const char* ssid = "realme X7 Max";
const char* password = "FreeWifi4U";

// GPIO Pin Definitions
#define LDRPIN 33
#define SOILPIN 35
#define DHTPIN 5
#define DHTTYPE DHT11
#define LEDPIN 17
#define FANPIN 15
#define PUMPPIN 19

// Initialize DHT Sensor
DHT dht(DHTPIN, DHTTYPE);

// Create Web Server Instance
WebServer server(80);

// Actuator States
bool ledState = false;
bool fanState = false;
bool pumpState = false;

// Function to get stable ADC reading
int getStableReading(int pin, int samples = 10) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(10);
    }
    return sum / samples;
}

// Function to handle sensor data request
void handleSensorData() {
    int ldrValue = analogRead(LDRPIN);
    int rawSoilValue = getStableReading(SOILPIN);
    rawSoilValue = constrain(rawSoilValue, 0, 4095);
    float soilMoisture = 100.0 - (100.0 * (rawSoilValue / 4095.0));
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    String json = "{";
    json += "\"light\":" + String(ldrValue) + ",";
    json += "\"soilMoisture\":" + String(soilMoisture, 1) + ",";
    json += "\"temperature\":" + String(temperature, 2) + ",";
    json += "\"humidity\":" + String(humidity, 2) + ",";
    json += "\"ledState\":" + String(ledState) + ",";
    json += "\"fanState\":" + String(fanState) + ",";
    json += "\"pumpState\":" + String(pumpState);
    json += "}";

    server.send(200, "application/json", json);
}

// Function to handle web page
void handleRoot() {
    int ldrValue = analogRead(LDRPIN);
    int rawSoilValue = getStableReading(SOILPIN);
    rawSoilValue = constrain(rawSoilValue, 0, 4095);
    float soilMoisture = 100.0 - (100.0 * (rawSoilValue / 4095.0));
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    String html = "<html><head>";
    html += "<meta http-equiv='refresh' content='3'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ESP32 Sensor & Actuator Control</title>";
    html += "<style>body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; color: #333; }";
    html += "h2 { color: #007BFF; padding: 2vh; font-size: 24px; } p { font-size: 18px; margin: 10px; }";
    html += "button { padding: 12px 24px; font-size: 16px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; transition: 0.3s; }";
    html += ".btn-on { background-color: #28a745; color: white; } .btn-on:hover { background-color: #218838; }";
    html += ".btn-off { background-color: #dc3545; color: white; } .btn-off:hover { background-color: #c82333; }</style></head><body>";

    html += "<div class='container'>";
    html += "<h2>Environment Parameters</h2>";
    html += "<p><strong>External Light:</strong> " + String(ldrValue < 1000 ? "ON" : "OFF") + "</p>";
    html += "<p><strong>Soil Moisture:</strong> " + String(soilMoisture, 1) + "%</p>";
    html += "<p><strong>Temperature:</strong> " + String(temperature, 2) + "&deg;C</p>";
    html += "<p><strong>Humidity:</strong> " + String(humidity, 2) + "%</p>";

    html += "<h2>Actuator Control</h2>";

    html += "<p><strong>LED:</strong> " + String(ledState ? "ON" : "OFF") + "</p>";
    html += "<button class='btn-on' onclick=\"toggleDevice('led')\">Toggle LED</button>";

    html += "<p><strong>Fan:</strong> " + String(fanState ? "ON" : "OFF") + "</p>";
    html += "<button class='btn-on' onclick=\"toggleDevice('fan')\">Toggle Fan</button>";

    html += "<p><strong>Pump:</strong> " + String(pumpState ? "ON" : "OFF") + "</p>";
    html += "<button class='btn-on' onclick=\"toggleDevice('pump')\">Toggle Pump</button>";

    html += "<script>function toggleDevice(device) { fetch('/toggle?device=' + device); }</script></body></html>";

    server.send(200, "text/html", html);
}

// Function to handle actuator toggles
void handleToggle() {
    if (server.hasArg("device")) {
        String device = server.arg("device");
        if (device == "led") {
            ledState = !ledState;
            digitalWrite(LEDPIN, ledState ? HIGH : LOW);
            Serial.println("LED toggled");
        } else if (device == "fan") {
            fanState = !fanState;
            digitalWrite(FANPIN, fanState ? HIGH : LOW);
            Serial.println("Fan toggled");
        } else if (device == "pump") {
            pumpState = !pumpState;
            digitalWrite(PUMPPIN, pumpState ? HIGH : LOW);
            Serial.println("Pump toggled");
        }
    }
    server.send(200, "text/plain", "OK");
}

void setup() {
    Serial.begin(115200);
    dht.begin();

    pinMode(LEDPIN, OUTPUT);
    pinMode(FANPIN, OUTPUT);
    pinMode(PUMPPIN, OUTPUT);
    pinMode(LDRPIN, INPUT);
    pinMode(SOILPIN, INPUT);

    digitalWrite(LEDPIN, LOW);
    digitalWrite(FANPIN, LOW);
    digitalWrite(PUMPPIN, LOW);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/toggle", handleToggle);
    server.on("/sensors", handleSensorData);
    server.begin();
    Serial.println("HTTP Server Started");
}

void loop() {
    server.handleClient();
}
