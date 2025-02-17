#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

#define SS 5
#define RST 14
#define DIO0 2
#define BUTTON_MODE 32
#define BUTTON_SELECT 33

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

TinyGPSPlus gps;

int currentMode = 0;
bool modeSelected = false;

String lastReceivedData = "";
unsigned long lastPacketTime = 0;
const unsigned long displayTimeout = 10000; // 10 sec timeout for "Last Received Data"

// Default GPS coordinates
float receiverLat = 22.977531701731635;
float receiverLon = 72.59396662698606;
float lastTempDHT = 0, lastHumDHT = 0, lastTempBMP = 0, lastPressure = 0;
double lastLat = 0, lastLon = 0;
int lastRSSI = 0;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_MODE, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED Init Failed!");
        while (1);
    }

    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa Init Failed!");
        while (1);
    }
    Serial.println("LoRa Receiver Ready!");
    modeSelected = false;
    updateMenu();
}

void loop() {
    handleButtons();
    if (!modeSelected) {
        updateMenu();
    } else {
        receiveData();
        if (currentMode == 0) {
            displayWeatherData();
        } else {
            displayGPSData();
        }
    }
}

void handleButtons() {
    if (digitalRead(BUTTON_MODE) == LOW) {
        delay(200);
        currentMode = (currentMode + 1) % 2;
        updateMenu();
    }
    if (digitalRead(BUTTON_SELECT) == LOW) {
        delay(200);
        modeSelected = true;
    }
}

void updateMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 10);
    display.println("SELECT MODE");
    display.setCursor(20, 30);
    display.print(currentMode == 0 ? "> Weather Station" : "> GPS Tracker");
    display.display();
}

void receiveData() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        lastReceivedData = "";
        while (LoRa.available()) {
            lastReceivedData += (char)LoRa.read();
        }
        lastPacketTime = millis();
        Serial.println("Received Data: " + lastReceivedData);

        if (currentMode == 0) {
            sscanf(lastReceivedData.c_str(), "%f,%f,%f,%f", &lastTempDHT, &lastHumDHT, &lastTempBMP, &lastPressure);
        } else {
            char identifier;
            sscanf(lastReceivedData.c_str(), "%c,%lf,%lf,%d", &identifier, &lastLat, &lastLon, &lastRSSI);

            // Corrected: Get RSSI **after** packet reception
            lastRSSI = LoRa.packetRssi();
        }
    }
}

void displayWeatherData() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Weather Station:");

    display.setCursor(0, 10);
    display.print("Temp: "); display.print(lastTempDHT); display.print("C");
    display.setCursor(0, 20);
    display.print("Humidity: "); display.print(lastHumDHT); display.print("%");
    display.setCursor(0, 30);
    display.print("Pressure: "); display.print(lastPressure); display.print("hPa");

    if (millis() - lastPacketTime > displayTimeout) {
        display.setCursor(0, 50);
        display.println("Last Received Data");
    }

    display.display();
}

void displayGPSData() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("GPS Tracker:");

    display.setCursor(0, 10);
    display.print("Lat: "); display.print("00.000");
    display.setCursor(0, 20);
    display.print("Lon: "); display.print("00.000");
    display.setCursor(0, 30);
    display.print("RSSI: "); display.print(lastRSSI);
    display.setCursor(0, 40);
    display.print("satallites: 0");

    if (millis() - lastPacketTime > displayTimeout) {
        display.setCursor(0, 50);
        display.println("Last Received Data");
    }

    display.display();
}

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    float R = 6371000;
    float dLat = (lat2 - lat1) * PI / 180;
    float dLon = (lon2 - lon1) * PI / 180;
    float a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * PI / 180) * cos(lat2 * PI / 180) * sin(dLon/2) * sin(dLon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}
