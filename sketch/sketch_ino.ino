#include <WiFi.h>        // For WiFi connection
#include <HTTPClient.h>  // For sending HTTP requests
#include <LiquidCrystal.h> // For LCD display

// WiFi settings - change to your network
const char* ssid = "testWIFI";
const char* password = "12345678";

// Node server address - change to your computer's IP
// Run "ipconfig" (Windows) or "ifconfig" (Mac/Linux) to find your IP
// Example: "http://192.168.1.100:3000/data"
const char* serverUrl = "http://10.129.10.3:3000/data";

// LM35 temperature sensor connected to GPIO 34
const int lm35Pin = 34;

// LCD pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(27, 14, 26, 25, 33, 32);

// Upload every 30 seconds
const long uploadInterval = 30000;
unsigned long lastUploadTime = 0;

// Connect to WiFi and show status on LCD
void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  Serial.print("Connecting");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    lcd.clear();
    lcd.print("WiFi OK");
    delay(1000);
  } else {
    Serial.println("\nWiFi failed");
    lcd.clear();
    lcd.print("WiFi Failed");
    delay(3000);
  }
  lcd.clear();
}

// Read temperature from LM35 sensor
float readTemperature() {
  int adcValue = analogRead(lm35Pin);                     // Read raw ADC (0-4095)
  float voltage_mV = adcValue * (3300.0 / 4095.0);        // Convert to millivolts
  float temperature = voltage_mV / 10.0;                  // LM35 gives 10mV per °C
  return temperature;
}

// Update LCD with temperature and WiFi status
void updateLCD(float temperature) {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C  ");
  
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi: On     ");
  } else {
    lcd.print("WiFi: Off    ");
  }
}

// Send temperature data to Node.js server
void sendToNodeServer(float temperature) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi, skipping upload");
    return;
  }
  
  HTTPClient http;
  http.begin(serverUrl);                                 // Start connection to server
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Tell server we send form data
  
  String postData = "temperature=" + String(temperature); // Create data string
  
  Serial.print("Sending... ");
  int httpCode = http.POST(postData);                    // Send POST request
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("OK");
    String response = http.getString();                  // Read server reply
    Serial.print("Server said: ");
    Serial.println(response);
  } else {
    Serial.print("Failed, error code: ");
    Serial.println(httpCode);
  }
  http.end();                                            // Close connection
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  
  lcd.print("Temp Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);
  
  connectToWiFi();
  
  lcd.clear();
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Temp: --.- C");
  Serial.println("System ready");
}

void loop() {
  float currentTemp = readTemperature();
  updateLCD(currentTemp);
  
  // Send data every 30 seconds
  unsigned long now = millis();
  if (now - lastUploadTime >= uploadInterval) {
    sendToNodeServer(currentTemp);
    lastUploadTime = now;
  }
  
  delay(1000);  // Update LCD once per second
}