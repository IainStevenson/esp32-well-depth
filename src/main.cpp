#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "MyWIFICreds.h"

const int triggerPin = 5;
const int echoPin = 18;
const float totalDepth = 250.0; // Total depth in CM

// Time client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Web server
AsyncWebServer server(80);

// Data structure
struct DepthData {
    float depth;
    unsigned long time;
};

// Define array size and mutex
const int arraySize = 100; // Adjust size based on available memory (example value)
DepthData depthArray[arraySize];
int currentIndex = 0;
SemaphoreHandle_t dataMutex;

void setup() {
    Serial.begin(115200);

    // Initialize sensor pins
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize time client
    timeClient.begin();

    // Initialize mutex
    dataMutex = xSemaphoreCreateMutex();

    // Start web server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hello, this is the ESP32!");
    });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        ESP.restart();
        request->send(200, "text/plain", "Device reset initiated");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        xSemaphoreTake(dataMutex, portMAX_DELAY);
        StaticJsonDocument<2000> doc; // Adjust size based on expected JSON payload
        JsonArray data = doc.createNestedArray("data");
        for (int i = 0; i < arraySize; ++i) {
            if (depthArray[i].time != 0) {
                JsonObject entry = data.createNestedObject();
                entry["depth"] = depthArray[i].depth;
                entry["time"] = depthArray[i].time;
            }
        }
        // Clear the array
        memset(depthArray, 0, sizeof(depthArray));
        currentIndex = 0;
        xSemaphoreGive(dataMutex);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
        xSemaphoreTake(dataMutex, portMAX_DELAY);
        String html = "<html><body><table border='1'><tr><th>Time</th><th>Depth (cm)</th></tr>";
        for (int i = arraySize - 1; i >= 0; --i) {
            if (depthArray[i].time != 0) {
                char timeString[25];
                unsigned long epochTime = depthArray[i].time;
                struct tm *ptm = gmtime((time_t *)&epochTime);
                sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d",
                        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
                html += "<tr><td>" + String(timeString) + "</td><td>" + String(depthArray[i].depth) + "</td></tr>";
            }
        }
        html += "</table></body></html>";
        xSemaphoreGive(dataMutex);

        request->send(200, "text/html", html);
    });

    server.begin();
}

float measureDistance() {
    // Clear the trigger pin
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);

    // Send a 10us pulse to the trigger pin
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // Read the echo pin
    long duration = pulseIn(echoPin, HIGH);

    // Calculate the distance in CM
    if (duration == 0) {
        return -1; // Return -1 if no pulse was detected
    }

    float distance = (duration * 0.0343) / 2;
    return distance;
}

float calculateDepth(float distance) {
    if (distance == -1) {
        return -1; // Return -1 if no valid distance was measured
    }
    return totalDepth - distance;
}

void storeData(float depth, unsigned long time) {
    // Protect array access with mutex
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    depthArray[currentIndex] = {depth, time};
    currentIndex = (currentIndex + 1) % arraySize;
    xSemaphoreGive(dataMutex);
}

void loop() {
    timeClient.update();
    // Get current time
    unsigned long epochTime = timeClient.getEpochTime();
    // Convert epoch time to human-readable format
    struct tm *ptm = gmtime((time_t *)&epochTime);
    char timeString[25];
    sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    // Measure distance and calculate depth
    float distance = measureDistance();
    float depth = calculateDepth(distance);

    // Store the data
    if (depth != -1) {
        storeData(depth, epochTime);
    }

    // Print the results
    Serial.print("Time: ");
    Serial.print(timeString);
    Serial.print(" Distance: ");
    Serial.print(distance);
    Serial.print(" cm Depth: ");
    Serial.print(depth);
    Serial.println(" cm");

    delay(10000);  // Delay for 10 seconds
}
