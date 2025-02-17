#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// Pin Definitions for LoRa
#define SS 10
#define RST 9
#define DIO0 2

// GPS Module Setup
TinyGPSPlus gps;
#define GPS_SERIAL Serial1  // GPS is connected to Serial1 on Arduino Nano 33 BLE

void setup() {
    Serial.begin(115200);
    GPS_SERIAL.begin(9600);  // GPS Baud Rate

    // Initialize LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa Init Failed!");
        while (1);
    }
    Serial.println("LoRa Transmitter Ready!");
}

void loop() {
    while (GPS_SERIAL.available()) {
        gps.encode(GPS_SERIAL.read());
    }

    if (gps.location.isUpdated()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        // Check if GPS data is valid
        if (gps.location.isValid()) {
            String gpsData = String(latitude, 6) + "," + String(longitude, 6);
            Serial.println("Sending GPS Data: " + gpsData);
            
            LoRa.beginPacket();
            LoRa.print(gpsData);
            LoRa.endPacket();
        } else {
            Serial.println("Waiting for GPS Fix...");
        }
    } else {
        Serial.println("No GPS Data Yet...");
    }

    delay(2000);  // Send every 2 seconds
}
