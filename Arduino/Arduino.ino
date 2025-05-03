#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <SPI.h>
#include <LoRa.h>

#define DHTPIN 2      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
#define LORA_SS 10    // LoRa module NSS (Chip Select)
#define LORA_RST 9    // LoRa module RESET
#define LORA_DIO0 3   // LoRa module DIO0

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(9600); // For debugging
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1) {}
  }

  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) { // Set frequency to 433 MHz
    Serial.println("LoRa initialization failed!");
    while (1) {}
  }
  Serial.println("Weather Station Initialized!");
  Serial.println("============================");
}

void loop() {
  // Read sensor data
  double h = dht.readHumidity();
  double t = dht.readTemperature();
  double p = bmp.readPressure() / 100.0F; // Convert pressure to hPa
  int rain = analogRead(A0); // Rain sensor reading

  // Check for sensor connection errors
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return; // Skip this iteration of the loop
  }

 if (p < 300.0 || p > 1100.0) {  
  Serial.println("Failed to read from BMP180 sensor!");
  return;
}

 if (rain == 0) {  // Adjust threshold based on sensor behavior
    Serial.println("Failed to read from Rain sensor!");
    return;
 }

  // Send sensor data via LoRa
  LoRa.beginPacket();
  LoRa.print(t);
  LoRa.print(",");
  LoRa.print(h);
  LoRa.print(",");
  LoRa.print(p);
  LoRa.print(",");
  LoRa.println(rain);
  LoRa.endPacket();

  // Print sensor data to Serial Monitor for debugging
  Serial.println("Sensor Readings:");
  Serial.print("Temperature: "); Serial.print(t); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(h); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(p); Serial.println(" hPa");
  Serial.print("Rain Level: "); Serial.print(rain); Serial.println(" (Analog Value)");
  Serial.println("---------------------------");

  delay(2000); // Wait a few seconds between measurements
}
