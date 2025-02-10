#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SS_PIN 5
#define RST_PIN 14
#define DIO0_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float lastTxLat = 0, lastTxLon = 0;
unsigned long lastReceivedTime = 0;
const unsigned long timeout = 3000;  // 3 sec timeout for real-time updates
bool isDataUpdating = false;  // Flag to track if data is being received

void setup() {
    Serial.begin(115200);

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 Initialization Failed!");
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LoRa GPS Receiver");
    display.display();
    delay(1000);

    SPI.begin();
    LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
    
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa failed!");
        while (1);
    }
    LoRa.setSpreadingFactor(7);  // Faster updates
    Serial.println("LoRa Initialized!");
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        lastReceivedTime = millis();
        isDataUpdating = true;  // Data is updating, so don't show "Disconnected"

        String receivedData = "";
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }

        Serial.println("Received: " + receivedData);

        float txLat, txLon;
        sscanf(receivedData.c_str(), "%f,%f", &txLat, &txLon);

        float distance = haversine(lastTxLat, lastTxLon, txLat, txLon);
        int rssi = LoRa.packetRssi();

        Serial.print("Tx Lat: "); Serial.print(txLat, 6);
        Serial.print(" | Tx Lon: "); Serial.print(txLon, 6);
        Serial.print(" | Distance: "); Serial.print(distance);
        Serial.print(" m | RSSI: "); Serial.println(rssi);

        lastTxLat = txLat;
        lastTxLon = txLon;

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Real-Time GPS:");
        display.print("Lat: "); display.println(txLat, 6);
        display.print("Lon: "); display.println(txLon, 6);
        display.print("Dist: "); display.print(distance); display.println("m");
        display.print("RSSI: "); display.println(rssi);
        display.display();
    }

    // If no data for 3 seconds and NOT updating, show "Disconnected"
    if ((millis() - lastReceivedTime > timeout) && !isDataUpdating) {
        Serial.println("LoRa Disconnected!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LoRa Disconnected!");
        display.display();
    }

    delay(100);  // Faster updates
}

// **Haversine Formula for Distance Calculation**
float haversine(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371000;
    float dLat = (lat2 - lat1) * PI / 180.0;
    float dLon = (lon2 - lon1) * PI / 180.0;
    
    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0) *
              sin(dLon / 2) * sin(dLon / 2);
    
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}
