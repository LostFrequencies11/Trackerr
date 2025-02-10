#include <SPI.h>
#include <LoRa.h>
#include <TinyGPSPlus.h>

// LoRa SX1278 Pins for Nano 33 BLE
#define SS_PIN 10
#define RST_PIN 9
#define DIO0_PIN 2

// GPS Module (Neo-6M) - Use Serial1 (default on Nano 33 BLE)
#define GPS_RX 1  // Connect to TX pin of GPS module
#define GPS_TX 0  // Connect to RX pin of GPS module
TinyGPSPlus gps;

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600); // Use Serial1 for GPS communication

    // LoRa Setup
    SPI.begin();
    LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
    
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa failed!");
        while (1);
    }
    Serial.println("LoRa Started!");
}

void loop() {
    while (Serial1.available()) { // Read GPS data from Serial1
        gps.encode(Serial1.read());
    }

    if (gps.location.isValid()) {
        float txLat = gps.location.lat();
        float txLon = gps.location.lng();
        unsigned long txTime = millis();

        String gpsData = String(txLat, 6) + "," + String(txLon, 6) + "," + String(txTime);
        
        Serial.println("Sending: " + gpsData);
        LoRa.beginPacket();
        LoRa.print(gpsData);
        LoRa.endPacket();
    } else {
        Serial.println("No GPS Signal! Move to Open Space.");
    }

    delay(1000);
}
