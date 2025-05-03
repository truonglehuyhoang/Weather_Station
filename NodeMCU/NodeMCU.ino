// Blynk template and authentication details
#define BLYNK_TEMPLATE_ID "TEMPLATE ID"
#define BLYNK_TEMPLATE_NAME "TEMPLATE NAME"
#define BLYNK_AUTH_TOKEN "AUTHENTICATION TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
#include <LoRa.h>

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR WIFI'S NAME"; // Replace with your WiFi SSID
char pass[] = "YOUR WIFI'S PASSWORD";  // Replace with your WiFi password

// Define LoRa module pins for NodeMCU
#define LORA_SS D8   // LoRa module NSS (Chip Select)
#define LORA_RST D4  // LoRa module RESET
#define LORA_DIO0 D2 // LoRa module DIO0

// Thresholds for notifications
#define TEMP_THRESHOLD 32.0  
#define HUMIDITY_THRESHOLD 80.0 
#define RAIN_THRESHOLD 500  
#define PRESSURE_THRESHOLD 1013.00  

// Timeout settings for detecting Arduino disconnection
const unsigned long ARDUINO_TIMEOUT = 10000;  // 10 seconds

// Global variable to track the last time a LoRa packet was received
unsigned long lastLoRaReceived = 0;

// Flag to avoid duplicate Arduino disconnection alerts
bool arduinoAlertSent = false;

// Interval for checking connection status (ms)
const unsigned long CHECK_INTERVAL = 5000;

// Blynk timer for periodic checks
BlynkTimer timer;

void setup() {
  Serial.begin(9600); 
  Blynk.begin(auth, ssid, pass); 

  // Record current time as the last LoRa reception time
  lastLoRaReceived = millis();
  
  // Set up a periodic check for Arduino disconnection every 5 seconds
  timer.setInterval(CHECK_INTERVAL, checkConnections);

  if (Blynk.connected()) {
    Serial.println("Connected to Blynk!");
  } else {
    Serial.println("Failed to connect to Blynk.");
  }

  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  delay(500);
  if (!LoRa.begin(433E6)) { 
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa Receiver Initialized!");
}

void loop() {
  Blynk.run(); 
  timer.run();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    Serial.println("Received LoRa data: " + receivedData);

    // Update the last received time and reset alert flag
    lastLoRaReceived = millis();  // Critical fix: Update timestamp here
    arduinoAlertSent = false;     // Reset alert flag on new data

    int commaIndex1 = receivedData.indexOf(',');
    int commaIndex2 = receivedData.indexOf(',', commaIndex1 + 1);
    int commaIndex3 = receivedData.indexOf(',', commaIndex2 + 1);

    double temperature = receivedData.substring(0, commaIndex1).toDouble();
    double humidity = receivedData.substring(commaIndex1 + 1, commaIndex2).toDouble();
    double pressure = receivedData.substring(commaIndex2 + 1, commaIndex3).toDouble();
    int rain = receivedData.substring(commaIndex3 + 1).toInt();

    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, pressure);
    Blynk.virtualWrite(V3, rain);

    Serial.println("Data sent to Blynk!");

    // Check conditions for Blynk notifications
    if (temperature > TEMP_THRESHOLD) {
      Serial.println("Warning: High Temperature Detected! " + String(temperature) + "°C");
      Blynk.logEvent("high_temperature", "Warning: High Temperature Detected! " + String(temperature) + "°C");
    }

    if (humidity > HUMIDITY_THRESHOLD) {
      Serial.println("Warning: High Humidity Detected! " + String(humidity) + "%");
      Blynk.logEvent("high_humidity", "Warning: High Humidity Detected! " + String(humidity) + "%");
    }

    if (rain < RAIN_THRESHOLD) {
      Serial.println("Warning: Heavy Rainfall Detected! Sensor Value: " + String(rain));
      Blynk.logEvent("heavy_rainfall", "Warning: Heavy Rainfall Detected! Sensor Value: " + String(rain));
    }

    if (pressure > PRESSURE_THRESHOLD) {
      Serial.println("Warning: High Pressure Detected! " + String(pressure) + " hPa");
      Blynk.logEvent("high_pressure", "Warning: High Pressure Detected! " + String(pressure) + " hPa");
    }
  }
}

// Periodic function to check if the Arduino transmitter is disconnected
void checkConnections() {
  unsigned long now = millis();

  if (now - lastLoRaReceived > ARDUINO_TIMEOUT) {
    if (!arduinoAlertSent) {
      Serial.println("Alert: No LoRa packet received from Arduino in the last 10 seconds!");
      Blynk.logEvent("arduino_disconnect", "No LoRa packet received for over 10 seconds.");
      
      // Attempt to reinitialize the LoRa module
      LoRa.end();
      delay(100);  // Short pause to allow cleanup
      if (LoRa.begin(433E6)) {
        Serial.println("LoRa reinitialized successfully.");
        lastLoRaReceived = millis();  // Reset the timer after reinitialization
      } else {
        Serial.println("Failed to reinitialize LoRa.");
      }
      
      arduinoAlertSent = true;  // Prevent duplicate alerts
    }
  }
}
