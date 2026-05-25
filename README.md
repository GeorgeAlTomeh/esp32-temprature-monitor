# ESP32 Temperature Monitor with Node.js Server

[![Platform](https://img.shields.io/badge/platform-ESP32-blue)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

## Short Story

I made this project as a **safety monitor** to know the real-time temperature in the places we go camp in 

I made it using esp32 that makes a post request to a node.js server - which I also made and is in this repo too - and by the time the server receives the request it writes the data into a local database (txt file based)

with a simple page to view the data in realtime

leaders can open this page from wherever place on the planet earth with internet connection once deployed

## Features

- Reads temperature from LM35 sensor (10mV per degree Celsius accuracy)
- Displays live temperature on 16x2 LCD
- Connects to WiFi network
- Sends temperature data to a Node.js server every 30 seconds
- Server logs each reading with a timestamp to a text file
- Simple web interface to view all logged data (`/logs`)
- Secure credential management using `config.h` (not committed to GitHub)

## Hardware Required

- ESP32 Development Board (1) – Any variant with WiFi
- LM35 Temperature Sensor (1) – Analog output, 10mV per degree Celsius
- 16x2 LCD HD44780 (1) – 16-pin parallel version
- 10k Ohm Potentiometer (1) – For LCD contrast control
- 220 Ohm Resistor (1) – For LCD backlight (optional)
- Breadboard and jumper wires – As needed for connections
- USB Cable (1) – For power and programming

## Wiring Diagram

### LM35 Temperature Sensor

- Left pin (VCC) connects to ESP32 **5V**
- Middle pin (OUT) connects to ESP32 **GPIO 35**
- Right pin (GND) connects to ESP32 **GND**

> **Note:** Power the LM35 with 5V for accurate readings. The output pin is safe for ESP32's 3.3V ADC.

### 16x2 LCD (Parallel Mode)

- LCD Pin 1 (VSS) → GND
- LCD Pin 2 (VDD) → 5V
- LCD Pin 3 (V0) → Potentiometer middle pin
- LCD Pin 4 (RS) → GPIO 27
- LCD Pin 5 (RW) → GND
- LCD Pin 6 (E) → GPIO 14
- LCD Pin 11 (D4) → GPIO 26
- LCD Pin 12 (D5) → GPIO 25
- LCD Pin 13 (D6) → GPIO 33
- LCD Pin 14 (D7) → GPIO 32
- LCD Pin 15 (A / LED+) → 5V (with 220 Ohm resistor, optional)
- LCD Pin 16 (K / LED-) → GND

### Potentiometer (Contrast Control)

- Left pin → 5V
- Middle pin → LCD pin 3 (V0)
- Right pin → GND

## Software Setup

### 1. Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/)

### 2. Install ESP32 Board Package
- Go to File → Preferences → Additional Boards Manager URLs
- Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Go to Tools → Board → Boards Manager → Search "ESP32" → Install

### 3. Install Required Libraries
- `LiquidCrystal` (comes with Arduino IDE)
- `WiFi` (comes with ESP32 package)
- `HTTPClient` (comes with ESP32 package)

### 4. Set Up the Node.js Server

#### 4.1 Install Node.js
Download and install Node.js from [nodejs.org](https://nodejs.org)

#### 4.2 Create a Project Folder
```bash
mkdir temperature-server
cd temperature-server
npm init -y
npm install express
```

4.3 Create server.js

Save the following code as server.js in the project folder:

```javascript
const express = require('express');
const fs = require('fs').promises;
const path = require('path');

const app = express();
const PORT = 3000;
const LOG_FILE = path.join(__dirname, 'temperature_log.txt');

app.use(express.urlencoded({ extended: true }));
app.use(express.json());

app.post('/data', async (req, res) => {
  const temperature = req.body.temperature;
  const timestamp = new Date().toISOString();
  console.log(`[${timestamp}] Received: ${temperature}°C`);
  try {
    await fs.appendFile(LOG_FILE, `${timestamp}, ${temperature}\n`);
    res.status(200).send(`OK: ${temperature}°C logged`);
  } catch (error) {
    console.error('Error writing to file:', error);
    res.status(500).send('Server error');
  }
});

app.get('/logs', async (req, res) => {
  try {
    const data = await fs.readFile(LOG_FILE, 'utf8');
    res.send(`<pre>${data}</pre>`);
  } catch (error) {
    res.send('No data yet. Send some from ESP32!');
  }
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  console.log(`ESP32 should send data to: http://YOUR_IP:${PORT}/data`);
  console.log(`View logs at: http://localhost:${PORT}/logs`);
});
```

4.4 Run the Server

```bash
node server.js
```

4.5 Find Your Computer's IP Address

· Windows: Open Command Prompt, type ipconfig, look for "IPv4 Address"
· Mac/Linux: Open Terminal, type ifconfig or ip addr, look for inet

Update the serverUrl in the ESP32 code with http://YOUR_IP:3000/data.

ESP32 Code

Create a new sketch in Arduino IDE and copy the code below. Replace YOUR_COMPUTER_IP with your actual IP address.

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://YOUR_COMPUTER_IP:3000/data";

const int lm35Pin = 35;
LiquidCrystal lcd(27, 14, 26, 25, 33, 32);

const long uploadInterval = 30000;
unsigned long lastUploadTime = 0;

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

float readTemperature() {
  int adcValue = analogRead(lm35Pin);
  float voltage_mV = adcValue * (3300.0 / 4095.0);
  float temperature = voltage_mV / 10.0;
  return temperature;
}

void updateLCD(float temperature) {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print(" C  ");
  
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi: On ");
  } else {
    lcd.print("WiFi: Off");
  }
}

void sendToNodeServer(float temperature) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String postData = "temperature=" + String(temperature);
  
  Serial.print("Sending... ");
  int httpCode = http.POST(postData);
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("OK");
    String response = http.getString();
    Serial.print("Server said: ");
    Serial.println(response);
  } else {
    Serial.print("Failed, error code: ");
    Serial.println(httpCode);
  }
  http.end();
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
  
  unsigned long now = millis();
  if (now - lastUploadTime >= uploadInterval) {
    sendToNodeServer(currentTemp);
    lastUploadTime = now;
  }
  
  delay(1000);
}
```

Expected Output

Serial Monitor (115200 baud)

```
ESP32 Temperature Monitor Starting
Connecting........
WiFi connected
System ready

Sending... OK
Server said: OK: 25.3°C logged
```

LCD Display

· Line 1: Temp: 25.3 C
· Line 2: WiFi: On

Node.js Server Console

```
Server running on http://localhost:3000
ESP32 should send data to: http://192.168.1.100:3000/data
View logs at: http://localhost:3000/logs
[2025-05-08T14:30:00.000Z] Received: 25.3°C
```

View Logged Data

Open a browser and go to http://localhost:3000/logs. You will see all temperature readings with timestamps:

```
2025-05-08T14:30:00.000Z, 25.3
2025-05-08T14:30:30.000Z, 25.4
2025-05-08T14:31:00.000Z, 25.2
```

License

This project is open source under the MIT License.

Author

George Tomeh
