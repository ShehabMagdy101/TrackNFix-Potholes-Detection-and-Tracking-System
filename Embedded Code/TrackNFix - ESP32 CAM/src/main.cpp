#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <SPIFFS.h>
#include "esp_camera.h"   // Added for ESP32-CAM

// ---- PIN DEFINITIONS FOR ESP32-CAM (AI Thinker) ----
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ---- GPS ----
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

// ---- Variables ----
const int vechile_id = 1;
int id = 1;

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
const long capture_interval = 2000;  // Capture every 2 seconds

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

// Capture and overwrite image
String captureAndEncodeImage() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }

  // Encode directly from framebuffer to base64 (no SPIFFS used, overwriting in RAM)
  String encoded = base64::encode(fb->buf, fb->len);

  esp_camera_fb_return(fb);  // free buffer
  Serial.println("Captured & encoded image, size: " + String(encoded.length()));
  return encoded;
}

void handleGPS(double &lat, double &lng, String &timestamp) {
  if (gps.location.isUpdated()) {
    lat = gps.location.lat();
    lng = gps.location.lng();

    // Format time string (UTC)
    char timeBuf[25];
    if (gps.date.isValid() && gps.time.isValid()) {
      sprintf(timeBuf, "%04d-%02d-%02d %02d:%02d:%02d",
              gps.date.year(),
              gps.date.month(),
              gps.date.day(),
              gps.time.hour(),
              gps.time.minute(),
              gps.time.second());
      timestamp = String(timeBuf);
    } else {
      timestamp = "unknown";
    }
  } else {
    lat = 0.0;
    lng = 0.0;
    timestamp = "no_fix";  // No GPS fix yet
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  // Initialize Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;  //  Lower resolution to save space
    config.jpeg_quality = 12;            //  Quality adjusted
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  }

  if(esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }

  // connecting to wifi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize secure WiFiClient
  wifiClient.setInsecure();
  setupMQTT();
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - previous_time > capture_interval) {  //  Capture every 2 sec
    previous_time = now;

    // Capture fresh image (overwrites previous automatically)
    String image_encoded = captureAndEncodeImage();
    
    if (image_encoded.length() == 0) {
      Serial.println("Failed to encode image, skipping this transmission");
      return;
    }

    double Latitude, Longitude;
    String timestamp;
    handleGPS(Latitude, Longitude, timestamp);

    // build JSON message
    id++;
    String msg = "{";
    msg += "\"id\":" + String(id) + ",";
    msg += "\"vehicle_id\":" + String(vechile_id) + ",";
    msg += "\"latitude\":" + String(Latitude, 6) + ",";
    msg += "\"longitude\":" + String(Longitude, 6) + ",";
    msg += "\"time\":\"" + timestamp + "\",";
    msg += "\"image\":\"" + image_encoded + "\"";
    msg += "}";

    Serial.println("Publishing message size: " + String(msg.length()) + " bytes");
    mqttClient.publish(topic_publish, msg.c_str());
  }
}