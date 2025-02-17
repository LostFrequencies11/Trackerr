#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>

// Pin Definitions
#define DHTPIN 25  
#define DHTTYPE DHT11
#define SS 5
#define RST 14
#define DIO0 2

// Sensor Objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

void setup() {
    Serial.begin(115200);
    dht.begin();
    
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 Not Found!");
        while (1);
    }

    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa Init Failed!");
        while (1);
    }
    Serial.println("LoRa Transmitter Ready!");
}

void loop() {
    float tempDHT = dht.readTemperature();
    float humDHT = dht.readHumidity();
    float tempBMP = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;  // Convert to hPa

    // Send Data via LoRa
    LoRa.beginPacket();
    LoRa.print(tempDHT); LoRa.print(",");
    LoRa.print(humDHT); LoRa.print(",");
    LoRa.print(tempBMP); LoRa.print(",");
    LoRa.print(pressure);
    LoRa.endPacket();

    Serial.print("Sent Data: ");
    Serial.print(tempDHT); Serial.print(",");
    Serial.print(humDHT); Serial.print(",");
    Serial.print(tempBMP); Serial.print(",");
    Serial.print(pressure);
    Serial.println();

    delay(7000);  // Update every 5 seconds
}
