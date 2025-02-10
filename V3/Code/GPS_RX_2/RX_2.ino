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
#define TOUCH_PIN T0  // Inbuilt touch sensor on GPIO4 (ESP32)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float lastTxLat = 0, lastTxLon = 0;
unsigned long lastReceivedTime = 0;
const unsigned long timeout = 3000; // 3 sec timeout for real-time updates
bool isDataUpdating = false;
bool petMode = false;
bool touchReleased = true;
bool modeSelected = false;
unsigned long touchStartTime = 0;

float homeLat = 22.977640; // Safe zone latitude
float homeLon = 72.594093; // Safe zone longitude
float geofenceRadius = 50.0; // Safe zone radius in meters

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    pinMode(BUZZER_PIN, OUTPUT);
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

    handleReturnToMenu(); // Check if user wants to return to menu

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
    //display.println("Tap: Switch Mode");
    //display.println("Hold 2s: Confirm");
    display.println(petMode ? "[Pet Mode]" : "[Tracker Mode]");
    display.display();
}

void handleModeSelection() {
    int touchValue = touchRead(TOUCH_PIN);
    if (touchValue < 30) {
        if (touchReleased) {
            petMode = !petMode; // Toggle mode
            displayStartupMenu();
            touchStartTime = millis();
            touchReleased = false;
        }

        if (millis() - touchStartTime > 5000) { // 2s hold
            modeSelected = true;
            Serial.println(petMode ? "Pet Mode Selected" : "Tracker Mode Selected");
            petMode ? showPetAnimation() : showTrackerAnimation();
        }
    } else {
        touchReleased = true;
    }
}

void handleReturnToMenu() {
    int touchValue = touchRead(TOUCH_PIN);
    if (touchValue < 30) {
        if (touchReleased) {
            touchStartTime = millis();
            touchReleased = false;
        }
        if (millis() - touchStartTime > 3000) { // 3s hold
            modeSelected = false;
            displayStartupMenu();
        }
    } else {
        touchReleased = true;
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
