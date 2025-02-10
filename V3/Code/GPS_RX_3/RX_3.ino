#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SS_PIN 5
#define RST_PIN 14
#define DIO0_PIN 2
#define BUZZER_PIN 15

#define MODE_BUTTON 32      // Button to switch modes
#define SELECT_BUTTON 33    // Button to confirm selection or return to menu
#define RESET_BUTTON 0      // ESP32 Reset Button (EN pin)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float lastTxLat = 0, lastTxLon = 0;
unsigned long lastReceivedTime = 0;
const unsigned long timeout = 3000;
bool isDataUpdating = false;
bool petMode = false;
bool modeSelected = false;

float homeLat = 22.977640;
float homeLon = 72.594093;
float geofenceRadius = 50.0;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(MODE_BUTTON, INPUT_PULLUP);
    pinMode(SELECT_BUTTON, INPUT_PULLUP);
    pinMode(RESET_BUTTON, INPUT_PULLUP);
    digitalWrite(BUZZER_PIN, LOW);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 Initialization Failed!");
        while (1);
    }

    displayStartupMenu();
    SPI.begin();
    LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa failed!");
        while (1);
    }
    LoRa.setSpreadingFactor(7);
    Serial.println("LoRa Initialized!");
}

void loop() {
    if (!modeSelected) {
        handleModeSelection();
        return;
    }

    handleReturnToMenu();

    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        lastReceivedTime = millis();
        isDataUpdating = true;
        String receivedData = "";
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }
        Serial.println("Received: " + receivedData);

        float txLat, txLon;
        sscanf(receivedData.c_str(), "%f,%f", &txLat, &txLon);
        float distance = haversine(lastTxLat, lastTxLon, txLat, txLon);
        float petDistance = haversine(homeLat, homeLon, txLat, txLon);
        int rssi = LoRa.packetRssi();

        lastTxLat = txLat;
        lastTxLon = txLon;

        display.clearDisplay();
        display.setCursor(0, 0);
        if (petMode) {
            display.println("PET MODE ON");
            display.print("From Geofence: "); display.print(petDistance); display.println("m");
            if (petDistance > geofenceRadius) {
                display.println("OUTSIDE!");
                digitalWrite(BUZZER_PIN, HIGH);
            } else {
                display.println("Safe Zone");
                digitalWrite(BUZZER_PIN, LOW);
            }
        } else {
            display.println("Tracker Mode");
            display.print("Lat: "); display.println(txLat, 6);
            display.print("Lon: "); display.println(txLon, 6);
            display.print("Dist: "); display.print(distance); display.println("m");
            display.print("RSSI: "); display.println(rssi);
        }
        display.display();
    }

    if ((millis() - lastReceivedTime > timeout) && !isDataUpdating) {
        Serial.println("LoRa Disconnected!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LoRa Disconnected!");
        display.display();
        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(100);
}

void displayStartupMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Select Mode:");
    display.println(petMode ? "[Pet Mode]" : "[Tracker Mode]");
    display.display();
}

void handleModeSelection() {
    static unsigned long lastPressTime = 0;
    if (digitalRead(MODE_BUTTON) == LOW) {
        if (millis() - lastPressTime > 200) { // Debounce
            petMode = !petMode;
            displayStartupMenu();
        }
        lastPressTime = millis();
    }

    if (digitalRead(SELECT_BUTTON) == LOW) {
        if (millis() - lastPressTime > 200) { // Debounce
            modeSelected = true;
            Serial.println(petMode ? "Pet Mode Selected" : "Tracker Mode Selected");
            petMode ? showPetAnimation() : showTrackerAnimation();
        }
        lastPressTime = millis();
    }
}

void handleReturnToMenu() {
    static unsigned long pressStartTime = 0;
    static bool modePressed = false;

    if (digitalRead(MODE_BUTTON) == LOW) {
        if (!modePressed) {
            pressStartTime = millis();
            modePressed = true;
        }
        if (millis() - pressStartTime > 3000) { // Hold MODE button for 3 sec
            modeSelected = false;
            displayStartupMenu();
            modePressed = false;
        }
    } else {
        modePressed = false;
    }

}

void showPetAnimation() {
    for (int i = 0; i < 3; i++) {
        display.clearDisplay();
        display.setCursor(30, 10);
        display.println("Pet Mode!");
        display.display();
        delay(500);
        display.clearDisplay();
        display.display();
        delay(500);
    }
}

void showTrackerAnimation() {
    for (int i = 0; i < 3; i++) {
        display.clearDisplay();
        display.setCursor(30, 10);
        display.println("Tracker Mode!");
        display.display();
        delay(500);
        display.clearDisplay();
        display.display();
        delay(500);
    }
}

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
