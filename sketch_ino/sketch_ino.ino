#include <WiFi.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(27, 14, 26, 25, 33, 32);
const int lm35Pin = 34;

const char* ssid = "testWIFI";
const char* password = "12345678";

void setup() {
  lcd.begin(16,2);
  WiFi.begin(ssid, password);
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  lcd.clear();
  lcd.print("WiFi OK");
  delay(1000);
  lcd.clear();
}

void loop() {
  int adcValue = analogRead(lm35Pin);
  float voltage_mV = adcValue * (5000.0 / 4095.0);
  float tempC = voltage_mV / 10.0;
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempC, 1);
  lcd.print(" C  ");
  delay(1000);
}