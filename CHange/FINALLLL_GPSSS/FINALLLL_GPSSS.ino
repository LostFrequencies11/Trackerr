#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// LoRa Module Pins
#define SS 10
#define RST 9
#define DIO0 2
#define GPS_Serial Serial1  

TinyGPSPlus gps;

void setup() {
    Serial.begin(115200);
    GPS_Serial.begin(9600);

    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa Init Failed!");
        while (1);
    }
    Serial.println("LoRa Ready!");
}

void loop() {
    double latitude = 0.0, longitude = 0.0;
    unsigned long startTime = millis();

    // Read GPS Data for a Maximum of 2 Seconds
    while (millis() - startTime < 2000) {
        while (GPS_Serial.available()) {
            gps.encode(GPS_Serial.read());
        }
    }

    // Check if GPS data is valid, else use a default location
    if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
    } else {
        latitude = 22.977630477581773;
        longitude = 72.59394516931476;
    }

    // Simulated RSSI Value for Transmission
    int rssi = random(-90, -40);  // Simulated range from -90 dBm to -40 dBm

    // Send Data via LoRa with Identifier "P,"
    LoRa.beginPacket();
    LoRa.print("P,");
    LoRa.print(latitude, 8); LoRa.print(",");
    LoRa.print(longitude, 8); LoRa.print(",");
    LoRa.print(rssi);
    LoRa.endPacket();

    Serial.print("Sent GPS Data: ");
    Serial.print(latitude, 8); Serial.print(",");
    Serial.print(longitude, 8); Serial.print(",");
    Serial.print(rssi);
    Serial.println();

    delay(2000);
}
