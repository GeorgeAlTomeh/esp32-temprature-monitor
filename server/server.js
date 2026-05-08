// Import Express framework to create a web server
const express = require('express');
// File system module to read/write log files
const fs = require('fs').promises;
// Path module to handle file paths safely
const path = require('path');

const app = express();
const PORT = 3000;

// Log file will be stored in the same folder
const LOG_FILE = path.join(__dirname, 'temperature_log.txt');

// Tell Express to parse form data (x-www-form-urlencoded) and JSON
app.use(express.urlencoded({ extended: true }));
app.use(express.json());

// Endpoint for ESP32 to POST temperature data
app.post('/data', async (req, res) => {
  // Get temperature from the request body (sent as "temperature=25.5")
  const temperature = req.body.temperature;
  
  // Create timestamp in ISO format (e.g., 2025-05-08T14:30:00.000Z)
  const timestamp = new Date().toISOString();
  
  // Print to console so you can see it live
  console.log(`[${timestamp}] Received: ${temperature}°C`);
  
  try {
    // Append data to log file (create file if it doesn't exist)
    await fs.appendFile(LOG_FILE, `${timestamp}, ${temperature}\n`);
    
    // Send success response back to ESP32
    res.status(200).send(`OK: ${temperature}°C logged`);
  } catch (error) {
    console.error('Error writing to file:', error);
    res.status(500).send('Server error');
  }
});

// Simple endpoint to view all logged data in your browser (optional)
app.get('/logs', async (req, res) => {
  try {
    const data = await fs.readFile(LOG_FILE, 'utf8');
    res.send(`<pre>${data}</pre>`);
  } catch (error) {
    res.send('No data yet. Send some from ESP32!');
  }
});

// Start the server
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  console.log(`ESP32 should send data to: http://YOUR_IP:${PORT}/data`);
  console.log(`View logs at: http://localhost:${PORT}/logs`);
});
