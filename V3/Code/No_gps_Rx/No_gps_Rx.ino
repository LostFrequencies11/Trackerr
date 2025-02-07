#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SS 5
#define RST 14
#define DIO0 2

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("LoRa Receiver");

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Waiting for data...");
    display.display();

    LoRa.setPins(SS, RST, DIO0);

    if (!LoRa.begin(433E6)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Received packet: ");
        String received = "";

        while (LoRa.available()) {
            received += (char)LoRa.read();
        }

        int rssi = LoRa.packetRssi();

        Serial.print(received);
        Serial.print(" with RSSI ");
        Serial.println(rssi);

        display.clearDisplay();
        display.setCursor(0, 10);
        display.println("Received: ");
        display.println(received);
        display.print("RSSI: ");
        display.println(rssi);
        display.display();
    }
}
