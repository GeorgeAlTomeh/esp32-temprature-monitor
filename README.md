# ESP32 Temperature Monitor with Google Sheets Logging

[![Platform](https://img.shields.io/badge/platform-ESP32-blue)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

## Overview

This project reads temperature from an **LM35 analog sensor** using an **ESP32**, displays the temperature on a **16x2 LCD**, connects to **WiFi**, and uploads the data to **Google Sheets** every 30 seconds using an HTTP GET request.

The system is designed as a **safety monitor for scout hikes** - leaders can view real-time temperature data remotely via the Google Sheet.

## Features

- Reads temperature from LM35 sensor (10mV per degree Celsius accuracy)
- Displays live temperature on 16x2 LCD
- Connects to WiFi network
- Uploads temperature and timestamp to Google Sheets every 30 seconds
- Secure credential management using `config.h` (not committed to GitHub)
- Automatic WiFi reconnection handling

## Hardware Required

- ESP32 Development Board (1) - Any variant with WiFi
- LM35 Temperature Sensor (1) - Analog output, 10mV per degree Celsius
- 16x2 LCD HD44780 (1) - 16-pin parallel version
- 10k Ohm Potentiometer (1) - For LCD contrast control
- 220 Ohm Resistor (1) - For LCD backlight (optional)
- Breadboard and jumper wires - As needed for connections
- USB Cable (1) - For power and programming

## Wiring Diagram

### LM35 Temperature Sensor

- Left pin (VCC) connects to ESP32 5V
- Middle pin (OUT) connects to ESP32 GPIO 35
- Right pin (GND) connects to ESP32 GND

Note: Power the LM35 with 5V for accurate readings. The output pin is safe for ESP32's 3.3V ADC.

### 16x2 LCD (Parallel Mode)

- LCD Pin 1 (VSS) connects to GND
- LCD Pin 2 (VDD) connects to 5V
- LCD Pin 3 (V0) connects to Potentiometer middle pin
- LCD Pin 4 (RS) connects to GPIO 27
- LCD Pin 5 (RW) connects to GND
- LCD Pin 6 (E) connects to GPIO 14
- LCD Pin 11 (D4) connects to GPIO 26
- LCD Pin 12 (D5) connects to GPIO 25
- LCD Pin 13 (D6) connects to GPIO 33
- LCD Pin 14 (D7) connects to GPIO 32
- LCD Pin 15 (A / LED+) connects to 5V (with 220 Ohm resistor (optional) )
- LCD Pin 16 (K / LED-) connects to GND

### Potentiometer (Contrast Control)

- Left pin connects to 5V
- Middle pin connects to LCD pin 3 (V0)
- Right pin connects to GND

## Software Setup

### 1. Install Arduino IDE
Download from arduino.cc

### 2. Install ESP32 Board Package
- Go to File -> Preferences -> Additional Boards Manager URLs
- Add this URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Go to Tools -> Board -> Boards Manager
- Search for "ESP32" and install the package

### 3. Install Required Libraries
- `LiquidCrystal` (comes with Arduino IDE)
- `WiFi` (comes with ESP32 package)
- `HTTPClient` (comes with ESP32 package)

### 4. Create Google Apps Script (Required for Data Logging)

Step-by-step to create your Google Sheets endpoint:

1. Create a new Google Sheet at sheets.new
2. Rename the first sheet to `TemperatureData`
3. Add headers in row 1: `Timestamp` , `Temperature (C)`
4. Go to Extensions -> Apps Script
5. Delete the default code and paste the script below
6. Click Deploy -> New Deployment
7. Choose Web App as deployment type
8. Set Execute as to `Me`
9. Set Who has access to `Anyone`
10. Click Deploy and copy the URL
11. Add this URL to your `config.h` file

**Google Apps Script Code:**

```javascript
function doGet(e) {
  // Get temperature from URL parameter
  var temp = e.parameter.temp;
  
  // Get current timestamp
  var timestamp = new Date();
  
  // Get the active spreadsheet
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  // Add headers if sheet is empty
  if (sheet.getLastRow() === 0) {
    sheet.appendRow(["Timestamp", "Temperature (C)", "Status"]);
  }
  
  // Append data as new row
  sheet.appendRow([timestamp, temp, "OK"]);
  
  // Return success response
  return ContentService.createTextOutput("Success: " + temp + "C recorded");
}