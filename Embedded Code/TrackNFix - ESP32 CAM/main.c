#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <base64.h>

// Wi-Fi credentials
const char* ssid = "ADHAM";
const char* password = "121269@ma#";

// MQTT broker details private
const char* mqtt_broker = "0785cb996e8440269dfc410343b0d3ef.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "shehab101";
const char* mqtt_password = "@inV6GzVjGtES2x";

// MQTT topic
const char* topic_publish = "esp32/tracknfix";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Variables for timing
long previous_time = 0;

void setupMQTT() {
  mqttClient.setServer(mqtt_broker, mqtt_port);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleimage() {
  // get the image from esp32 cam every 2 sec
  // encodes the image into binary format
  // testing images encoded
}

String encodeImageToBase64(const char* filepath) {
  if (!SPIFFS.exists(filepath)) {
    Serial.println("File does not exist: " + String(filepath));
    return "";
  }
  File file = SPIFFS.open(filepath, "r");
  if (!file) {
    Serial.println("Failed to open file: " + String(filepath));
    return "";
  }
  size_t fileSize = file.size();
  Serial.println("File size: " + String(fileSize) + " bytes");
  if (fileSize == 0) {
    Serial.println("File is empty!");
    file.close();
    return "";
  }
  // Check available heap memory
  Serial.println("Free heap before allocation: " + String(ESP.getFreeHeap()) + " bytes");

  if (fileSize > ESP.getFreeHeap() / 2) {
    Serial.println("File too large for available memory!");
    file.close();
    return "";
  }
  // Allocate buffer for the image data
  uint8_t* buffer = new uint8_t[fileSize];
  if (!buffer) {
    Serial.println("Failed to allocate memory for image buffer");
    file.close();
    return "";
  }
  
  // Read the file into buffer
  size_t bytesRead = file.read(buffer, fileSize);
  file.close();
  
  Serial.println("Bytes read: " + String(bytesRead) + "/" + String(fileSize));

  if (bytesRead != fileSize) {
    Serial.println("Error reading file - expected " + String(fileSize) + " bytes, got " + String(bytesRead));
    delete[] buffer;
    return "";
  }

  // Show first few bytes for debugging
  Serial.print("First 10 bytes: ");
  for (int i = 0; i < min(10, (int)fileSize); i++) {
    Serial.print("0x");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Encode to base64
  String encoded = base64::encode(buffer, fileSize);
  delete[] buffer;
  
  Serial.println("Base64 encoded length: " + String(encoded.length()));
  Serial.println("Free heap after encoding: " + String(ESP.getFreeHeap()) + " bytes");
  return encoded;
}

void handleGPS() {
  // get the long/lat from GPS NEO6 module (and time)
}

void listSPIFFSFiles() {
  Serial.println("=== SPIFFS File System Contents ===");
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Root is not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR : ");
      Serial.println(file.name());
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
  Serial.println("=== End of SPIFFS Contents ===");
}

void setup()
{
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize

   if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }
  Serial.println("\nSPIFFS initialized successfully");
   // Check SPIFFS info
  Serial.println("SPIFFS Total: " + String(SPIFFS.totalBytes()) + " bytes");
  Serial.println("SPIFFS Used: " + String(SPIFFS.usedBytes()) + " bytes");
  listSPIFFSFiles();

  String testPaths[3] = {"/image1.jpg", "/image2.jpg", "/image3.jpg"};
  for (int i = 0; i < 3; i++) {
    Serial.println("\n--- Testing " + testPaths[i] + " ---");
    if (SPIFFS.exists(testPaths[i])) {
      Serial.println("File exists!");
      File testFile = SPIFFS.open(testPaths[i], "r");
      if (testFile) {
        Serial.println("File size: " + String(testFile.size()) + " bytes");
        testFile.close();
      }
    } else {
      Serial.println("File does not exist!");
    }
  }

  // connecting to wifi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Connected
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize secure WiFiClient
  wifiClient.setInsecure();
  setupMQTT();
}

const int vechile_id = 1;
int id = 1;

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - previous_time > 10000) {
    previous_time = now;

    // Temporary images for testing
    String testPaths[3] = {"/image1.jpg", "/image2.jpg", "/image3.jpg"};
    static int imageIndex = 0;
    String image_encoded = encodeImageToBase64(testPaths[imageIndex].c_str());
    imageIndex = (imageIndex + 1) % 3; // Cycle through images
    
    if (image_encoded.length() == 0) {
      Serial.println("Failed to encode image, skipping this transmission");
      return;
    }

    id++;
    double Latitude = 30.34005;
    double longitude = -32.9440;
    String time = "23:6:2025:0z";
    
    String msg = "{";
    msg += "\"id\":" + String(id) + ",";
    msg += "\"vehicle_id\":" + String(vechile_id) + ",";
    msg += "\"latitude\":" + String(Latitude, 6) + ",";
    msg += "\"longitude\":" + String(longitude, 6) + ",";
    msg += "\"time\":\"" + time + "\",";
    msg += "\"image\":\"" + image_encoded + "\"";
    msg += "}";

    Serial.println("Publishing message size: " + String(msg.length()) + " bytes");
    Serial.println(msg.c_str());
    mqttClient.publish(topic_publish, msg.c_str());
  }
}