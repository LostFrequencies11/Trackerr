#include <SPI.h>
#include <LoRa.h>

#define SS 10
#define RST 9
#define DIO0 2

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("LoRa Transmitter");

    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(433E6)) {  // Set frequency (433MHz)
        Serial.println("Starting LoRa failed!");
        while (1);
    }
}

void loop() {
    Serial.println("Sending packet...");
    
    LoRa.beginPacket();
    LoRa.print("Hello ESP32!");
    LoRa.endPacket();
    
    delay(2000); // Send every 2 seconds
}
