#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "MyWIFICreds.h"

// Constants and Globals
const int TRIGGER_PIN = 5;
const int ECHO_PIN = 18;
const int MAX_DEPTH_CM = 300;
const int MEASURE_INTERVAL = 10000; // 10 seconds
const unsigned long RETRY_INTERVAL = 180000; // 3 minutes
const float SPEED_OF_SOUND_CM_US = 0.0343;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
AsyncWebServer server(80);

struct SensorData {
    unsigned long timestamp;  // Store timestamp as unsigned long
    float depth;
};

std::vector<SensorData> sensorData;
unsigned long lastMeasureTime = 0;
unsigned long lastNetworkCheckTime = 0;

// Function to connect to the best available WiFi network
void connectToWiFi() {
    int bestRSSI = -9999;
    String bestSSID = "";
    
    // If preferred SSID is defined, try to connect
    if (preferredSSID && strlen(preferredSSID) > 0) {
        WiFi.begin(preferredSSID, wifiPassword);
        Serial.print("Connecting to preferred SSID: ");
        Serial.println(preferredSSID);
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // 10 second timeout
            delay(500);
            Serial.print(".");
        }

        // If connected, print the MAC address and return
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println(" Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.print("MAC Address: ");
            Serial.println(WiFi.macAddress());
            return;
        } else {
            Serial.println(" Failed to connect to preferred SSID.");
        }
    }

    // Scan for available networks
    Serial.println("Scanning for available networks...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("No networks found. Retrying...");
        delay(10000); // Wait 10 seconds before retrying
        return;
    }

    // Find the best SSID with the strongest RSSI
    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        if (rssi > bestRSSI) {
            bestRSSI = rssi;
            bestSSID = ssid;
        }
    }

    // Connect to the best available SSID
    if (bestSSID.length() > 0) {
        Serial.print("Connecting to best available SSID: ");
        Serial.println(bestSSID);
        WiFi.begin(bestSSID.c_str(), wifiPassword);
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // 10 second timeout
            delay(500);
            Serial.print(".");
        }

        // Print connection details
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println(" Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.print("MAC Address: ");
            Serial.println(WiFi.macAddress());
        } else {
            Serial.println(" Failed to connect to any SSID.");
        }
    }
}

// Function to format the time as "YYYY-MM-DD HH:MM:SS"
String formatTimestamp(unsigned long epochTime) {
    time_t rawTime = (time_t)epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    return String(buffer);
}

void setup() {
    Serial.begin(115200);
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    connectToWiFi();

    timeClient.begin();
    timeClient.update();

    // Root endpoint to show device information
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String response = "IP Address: " + WiFi.localIP().toString();
        response += "<br>MAC Address: " + WiFi.macAddress();
        response += "<br>UTC Time: " + timeClient.getFormattedTime();
        response += "<br>Data Entries: " + String(sensorData.size());
        response += "<br>Memory Used: " + String(ESP.getFreeHeap()) + " bytes";
        request->send(200, "text/html", response);
    });

    // Endpoint to get the sensor data in JSON format
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonResponse;
        DynamicJsonDocument doc(1024);
        for (auto &data : sensorData) {
            JsonObject entry = doc.createNestedObject();
            entry["timestamp"] = formatTimestamp(data.timestamp);
            entry["depth"] = data.depth;
        }
        serializeJson(doc, jsonResponse);
        sensorData.clear();
        request->send(200, "application/json", jsonResponse);
    });

    // Endpoint to get the data and then reset the ESP
    server.on("/data?reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonResponse;
        DynamicJsonDocument doc(1024);
        for (auto &data : sensorData) {
            JsonObject entry = doc.createNestedObject();
            entry["timestamp"] = formatTimestamp(data.timestamp);
            entry["depth"] = data.depth;
        }
        serializeJson(doc, jsonResponse);
        sensorData.clear();
        request->send(200, "application/json", jsonResponse);
        delay(1000); // Wait for response to be sent before resetting
        ESP.restart();
    });

    // Endpoint to monitor the sensor data in a table format
    server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
        String htmlResponse = "<table><tr><th>Timestamp</th><th>Depth (cm)</th></tr>";
        for (auto it = sensorData.rbegin(); it != sensorData.rend(); ++it) {
            htmlResponse += "<tr><td>" + formatTimestamp(it->timestamp) + "</td><td>" + String(it->depth) + "</td></tr>";
        }
        htmlResponse += "</table>";
        request->send(200, "text/html", htmlResponse);
    });

    server.begin();
}

// Function to measure the distance using the ultrasonic sensor
float measureDistance() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    float distanceCm = duration * SPEED_OF_SOUND_CM_US / 2.0;
    return distanceCm;
}

void loop() {
    unsigned long currentTime = millis();
    
    // Reconnect to WiFi if disconnected and retry interval has passed
    if (WiFi.status() != WL_CONNECTED && currentTime - lastNetworkCheckTime > RETRY_INTERVAL) {
        connectToWiFi();
        lastNetworkCheckTime = currentTime;
    }

    // Update the NTP client if connected to WiFi
    if (WiFi.status() == WL_CONNECTED) {
        timeClient.update();
    }

    // Measure distance and calculate water depth at regular intervals
    if (currentTime - lastMeasureTime >= MEASURE_INTERVAL) {
        float distance = measureDistance();
        float depth = MAX_DEPTH_CM - distance;
        unsigned long timestamp = WiFi.status() == WL_CONNECTED ? timeClient.getEpochTime() : millis();

        // Manage sensor data in a FIFO manner
        if (sensorData.size() >= (ESP.getFreeHeap() / 4)) {
            sensorData.erase(sensorData.begin());
        }

        sensorData.push_back({timestamp, depth});
        lastMeasureTime = currentTime;
    }
}
