/*
 * Project: ESP32 Temperature Monitor
 * Author: George Tomeh
 * Description: 
 *   This code reads temperature from an LM35 sensor connected to GPIO 35,
 *   displays it on a 16x2 LCD, connects to WiFi, and uploads data to
 *   Google Sheets every 30 seconds using an HTTP GET request.
 * 
 * Hardware Connections:
 *   - LM35 VCC  -> ESP32 5V
 *   - LM35 OUT  -> ESP32 GPIO 34
 *   - LM35 GND  -> ESP32 GND
 *   - LCD RS    -> GPIO 27
 *   - LCD EN    -> GPIO 14
 *   - LCD D4    -> GPIO 26
 *   - LCD D5    -> GPIO 25
 *   - LCD D6    -> GPIO 33
 *   - LCD D7    -> GPIO 32
 *   - LCD VSS   -> GND
 *   - LCD VDD   -> 5V
 *   - LCD V0    -> Potentiometer middle pin
 *   - LCD RW    -> GND
 *   - LCD A     -> 5V (with 220Ω resistor)
 *   - LCD K     -> GND
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>

#include "config.h"

const int lm35Pin = 34;

// LCD pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(27, 14, 26, 25, 33, 32);

// ========== TIMING CONFIGURATION ==========
const long uploadInterval = 30000;  // Upload to Google Sheets every 30 seconds
unsigned long lastUploadTime = 0;

// ========== SETUP FUNCTION ==========
void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=================================");
  Serial.println("ESP32 Temperature Monitor Starting");
  Serial.println("=================================");
  
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Show ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Temp: --.- C");
  
  Serial.println("System ready. Reading temperature...\n");
}

// ========== WiFi CONNECTION FUNCTION ==========
void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected successfully!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\n✗ WiFi connection failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check credentials");
    delay(3000);
  }
  lcd.clear();
}

// ========== READ LM35 TEMPERATURE ==========
// Returns temperature in Celsius
float readTemperature() {
  int adcValue = analogRead(lm35Pin);
  
  // Convert ADC reading to millivolts
  // ESP32 ADC: 0-4095 corresponds to 0-3300mV
  float voltage_mV = adcValue * (3300.0 / 4095.0);
  
  // LM35 outputs 10mV per degree Celsius
  float temperature = voltage_mV / 10.0;
  
  // Print debug info to Serial
  Serial.print("  ADC: ");
  Serial.print(adcValue);
  Serial.print(" | mV: ");
  Serial.print(voltage_mV, 1);
  Serial.print(" | Temp: ");
  Serial.print(temperature, 1);
  Serial.println(" °C");
  
  return temperature;
}

// ========== UPDATE LCD DISPLAY ==========
void updateLCD(float temperature) {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C  ");
  
  // Show WiFi status on second line
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi: Online ");
  } else {
    lcd.print("WiFi: Offline");
  }
}

// ========== UPLOAD TO GOOGLE SHEETS ==========
void uploadToGoogleSheets(float temperature) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("  ✗ Upload skipped: WiFi not connected");
    return;
  }
  
  HTTPClient http;
  
  // Build URL with temperature as query parameter
  String url = serverUrl + "?temp=" + String(temperature);
  
  Serial.print("  Uploading to Google Sheets... ");
  
  http.begin(url);
  http.setTimeout(5000);  // 5 second timeout
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("✓ Success");
      // Optional: read response
      String response = http.getString();
      Serial.print("    Response: ");
      Serial.println(response);
    } else {
      Serial.print("✗ HTTP Error: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("✗ Connection failed: ");
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
}

// ========== MAIN LOOP ==========
void loop() {
  // Read temperature from LM35
  float currentTemp = readTemperature();
  
  // Update LCD display
  updateLCD(currentTemp);
  
  // Check if it's time to upload to Google Sheets
  unsigned long currentTime = millis();
  if (currentTime - lastUploadTime >= uploadInterval) {
    uploadToGoogleSheets(currentTemp);
    lastUploadTime = currentTime;
  }
  
  // Wait 1 second before next reading
  delay(1000);
}