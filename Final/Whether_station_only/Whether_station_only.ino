#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions
#define SS 5
#define RST 14
#define DIO0 2
#define BUZZER 25 // Buzzer for alerts

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin (not used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);

    // Initialize Buzzer
    pinMode(BUZZER, OUTPUT);
    
    // Initialize OLED Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Change to 0x3D if needed
        Serial.println("SSD1306 OLED not found! Check wiring.");
        while (1);
    }
    
    // Clear Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("LoRa Receiver Ready!");
    display.display();
    delay(2000); // Show message for 2 seconds
    
    // Initialize LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa Init Failed!");
        while (1);
    }
    Serial.println("LoRa Receiver Initialized!");
}

void loop() {
    // Check for incoming LoRa packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String receivedData = "";
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }
        Serial.println("Received Data: " + receivedData);

        // Parse Received Data
        float tempDHT, humDHT, tempBMP, pressure;
        sscanf(receivedData.c_str(), "%f,%f,%f,%f", &tempDHT, &humDHT, &tempBMP, &pressure);

        // Display Received Data on OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LoRa Data Received:");
        display.print("DHT Temp: "); display.print(tempDHT); display.println(" C");
        display.print("Humidity: "); display.print(humDHT); display.println(" %");
        display.print("BMP Temp: "); display.print(tempBMP); display.println(" C");
        display.print("Pressure: "); display.print(pressure); display.println(" hPa");
        display.display();

        // Buzzer Alert (Short Beep)
        tone(BUZZER, 1000, 200);  
    }
}
