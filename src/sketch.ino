#include <WiFi.h>
#include <HTTPClient.h>

// Firebase Realtime Database URL and API Key
const String firebaseUrl = "https://smartbintracker-d98c4-default-rtdb.asia-southeast1.firebasedatabase.app";
const String firebaseKey = "AIzaSyDLGT3iUSiaVW-JvHY_pgHLpjr9brMr7m4"; // Replace with your Firebase key

// Ultrasonic Sensor Pins
#define TRIG_PIN 27
#define ECHO_PIN 26
#define LED_PIN 2

void setup() {
    Serial.begin(115200);
    Serial.println("Hello, ESP32!");

    // Connect to Wi-Fi
    connectToWiFi();

    // Initialize Sensor and LED
    setupSensor();
}

void connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin("Wokwi-GUEST");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("Connected to WiFi!");
}

void setupSensor() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
}

float getBinLevel() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = (duration * 0.034) / 2; // Distance in cm
    return distance;
}

void sendPercentageToFirebase(float levelPercentage) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Build the Firebase URL for your specific node
        String url = String(firebaseUrl) + "/binStatus.json?auth=" + firebaseKey;

        // Create JSON payload
        String jsonPayload = "{\"levelPercentage\": " + String(levelPercentage) + "}";

        // Send HTTP POST request
        http.begin(url.c_str());
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.PUT(jsonPayload); // Use PUT for updating data

        // Log the result
        if (httpResponseCode > 0) {
            Serial.println("Data sent to Firebase successfully!");
        } else {
            Serial.println("Error sending data to Firebase: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("WiFi not connected!");
    }
}

void loop() {
    float distance = getBinLevel();
    float binHeight = 400.0; // Height of the bin in cm
    float levelPercentage = ((binHeight - distance) / binHeight) * 100;

    if (levelPercentage > 80.0) {
        Serial.println("percentageHigh: " + String(levelPercentage));
        digitalWrite(LED_PIN, HIGH); // Turn on LED if the bin is full
    } else {
        Serial.println("percentageLow: " + String(levelPercentage));
        digitalWrite(LED_PIN, LOW);
    }

    // Send the percentage to Firebase
    sendPercentageToFirebase(levelPercentage);

    delay(2000); // Update every 5 seconds
}
