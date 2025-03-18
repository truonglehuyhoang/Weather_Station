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

void setup() {
  Serial.begin(9600); 
  Blynk.begin(auth, ssid, pass); 

  if (Blynk.connected()) {
    Serial.println("Connected to Blynk!");
  } else {
    Serial.println("Failed to connect to Blynk.");
  }

  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) { 
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa Receiver Initialized!");
}

void loop() {
  Blynk.run(); 

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    Serial.println("Received LoRa data: " + receivedData);

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
