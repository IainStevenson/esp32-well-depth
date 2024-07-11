#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include "MyWIFICreds.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Constants and Globals
const int TRIGGER_PIN = 5;
const int ECHO_PIN = 18;
const int MAX_DEPTH_CM = 300;
const int MEASURE_INTERVAL = 10000;          // 10 seconds
const unsigned long RETRY_INTERVAL = 180000; // 3 minutes
const float SPEED_OF_SOUND_CM_US = 0.0343;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
AsyncWebServer server(80);

struct SensorData
{
    unsigned long timestamp; // Store timestamp as unsigned long
    float distance;          // measured distance
    float depth;             // caclulated depth
};

std::vector<SensorData> sensorData;

SemaphoreHandle_t dataMutex;

unsigned long lastMeasureTime = 0;
unsigned long lastNetworkCheckTime = 0;

void addSensorData(unsigned long timestamp, float distance, float depth)
{
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
        // Calculate the current memory usage of the array
        size_t arraySize = sensorData.size() * sizeof(SensorData);
        uint32_t freeMem = esp_get_free_heap_size();
        uint32_t maxMemUsage = freeMem / 4; // 25% of free memory

        // Remove oldest data if exceeding 25% of free memory
        while (arraySize >= maxMemUsage && !sensorData.empty())
        {
            sensorData.erase(sensorData.begin());
            arraySize = sensorData.size() * sizeof(SensorData);
        }

        // Add new data to the array
        sensorData.push_back({timestamp, distance, depth});
        xSemaphoreGive(dataMutex);
    }
}

// Function to format the time as "YYYY-MM-DD HH:MM:SS"
String formatTimestamp(unsigned long epochTime)
{
    time_t rawTime = (time_t)epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    return String(buffer);
}

void initEndpoints()
{
    // Root endpoint to show device information
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                timeClient.update();
        String response = "<head><meta http-equiv='refresh' content='5'></head><table><body>";
        response += "<br>IP Address: " + WiFi.localIP().toString();
        response += "<br>MAC Address: " + WiFi.macAddress();
        response += "<br>UTC Time: " + timeClient.getFormattedTime();
        response += "<br>Data Entries: " + String(sensorData.size());
        response += "<br>Data Size: " + String(sizeof(sensorData)) + " bytes" ;
        response += "<br>Data Volume: " + String(sizeof(sensorData) * sensorData.size()) + " bytes" ;
        response += "<br>Memory Available: " + String(ESP.getFreeHeap()) + " bytes";
        response += "</body></html>";
        request->send(200, "text/html", response); });

    // Endpoint to get the sensor data in JSON format
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String jsonResponse;
        DynamicJsonDocument doc(1024);
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
              {

            for (auto &data : sensorData)
            {
                JsonObject entry = doc.createNestedObject();
                entry["timestamp"] = formatTimestamp(data.timestamp);
                entry["distance"] = data.distance;
                entry["depth"] = data.depth;

            }
            xSemaphoreGive(dataMutex);
              }
        serializeJson(doc, jsonResponse);
        sensorData.clear();
        request->send(200, "application/json", jsonResponse); });

    // Endpoint to get the data and then reset the ESP
    server.on("/data?reset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String jsonResponse;
        DynamicJsonDocument doc(1024);
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            for (auto &data : sensorData)
            {
                JsonObject entry = doc.createNestedObject();
                entry["timestamp"] = formatTimestamp(data.timestamp);
                entry["distance"] = data.depth;
                entry["depth"] = data.depth;
            }
            xSemaphoreGive(dataMutex);
        }
        serializeJson(doc, jsonResponse);
        sensorData.clear();
        request->send(200, "application/json", jsonResponse);
        delay(1000); // Wait for response to be sent before resetting
        ESP.restart(); });

    // Endpoint to monitor the sensor data in a table format
    server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                String htmlResponse = "<head><meta http-equiv='refresh' content='5'></head><table><body><tr><th>Timestamp</th><th>Distance (cm)</th><th>Depth (cm)</th></tr>";
                if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
                {
                    for (auto it = sensorData.rbegin(); it != sensorData.rend(); ++it)
                    {
                        htmlResponse += "<tr>";

                        htmlResponse += "<td> " + formatTimestamp(it->timestamp) + "</td>";
                        htmlResponse += "<td> " + String(it->distance) + "</td>";
                        htmlResponse += "<td> " + String(it->depth) + "</td>";
                        htmlResponse += "</td></tr>";
                    }
                    xSemaphoreGive(dataMutex);
                }
                    htmlResponse += "</table></body>";
                    request->send(200, "text/html", htmlResponse); });
}
// Function to connect to the best available WiFi network
void initNetwork()
{

    // If preferred SSID is defined, try to connect
    if (preferredSSID && strlen(preferredSSID) > 0)
    {
        WiFi.begin(preferredSSID, wifiPassword);
        Serial.print("Connecting to SSID: ");
        Serial.println(preferredSSID);

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
        { // 10 second timeout
            delay(500);
            Serial.print(".");
        }
        Serial.println("");

        // If connected, print the MAC address and return
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("Connected! ");
            Serial.print("IP Address: ");
            Serial.print(WiFi.localIP());
            Serial.print(" MAC Address: ");
            Serial.println(WiFi.macAddress());
            return;
        }
        else
        {
            Serial.print("Failed to connect, ");
            Serial.println("Please reset");
            while (true)
            {
                delay(500);
            }
        }
    }
    timeClient.begin();
}

void initSensor()
{
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

// Function to measure the distance using the ultrasonic sensor
float measureDistance()
{
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distanceCm = duration * SPEED_OF_SOUND_CM_US / 2.0;
    return distanceCm;
}

void setup()
{
    Serial.begin(115200);

    initSensor();

    initNetwork();

    dataMutex = xSemaphoreCreateMutex();

    initEndpoints();

    server.begin();
    Serial.println("Setup complete.");
}

void loop()
{

    timeClient.update();

    // Measure distance and calculate water depth at regular intervals
    float distance = measureDistance();

    float depth = MAX_DEPTH_CM - distance;

    // Manage sensor data in a FIFO manner
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
        if (sizeof(sensorData) >= (ESP.getFreeHeap() / 4))
        {
            sensorData.erase(sensorData.begin());
        }
        sensorData.push_back({timeClient.getEpochTime(), distance, depth});
        xSemaphoreGive(dataMutex);
    }
    delay(MEASURE_INTERVAL);
}