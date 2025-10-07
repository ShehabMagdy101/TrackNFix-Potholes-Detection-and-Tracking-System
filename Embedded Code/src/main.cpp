#include "esp_camera.h"
#include <WiFi.h>
#include "base64.h"
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// =============================
// indicator: Buzzer
#define BUZZER_PIN 0
// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ---- GPS -----
#define RXD2 13
#define TXD2 12
#define GPS_BAUD 9600
bool gpsFixNotified = false;

HardwareSerial SerialGPS(1);
TinyGPSPlus gps;

// ---- Variables ----
const int vechile_id = 1;
int id = 1;

// MQTT topic
const char* topic_publish = "esp32/tracknfix";

// MQTT broker details private
const char* mqtt_broker = "0785cb996e8440269dfc410343b0d3ef.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "shehab101";
const char* mqtt_password = "@inV6GzVjGtES2x";

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "ADHAM";
const char *password = "121269@ma#";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Timing variables
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 10000; // 10 seconds
unsigned long lastGPSCheck = 0;

void SetupIndicator(){
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void buzzerBeep(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delayMs);
  }
}

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
      buzzerBeep(2, 100);
      Serial.println("Connected to MQTT Broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String captureAndEncodeImage();

void setup() {
  Serial.begin(115200);

  SetupIndicator();
  Serial.setDebugOutput(true);
  Serial.println();
  mqttClient.setBufferSize(16000);  // Increased buffer size

  Serial.println("\nGPS Searching Started");
  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  
  // connecting to wifi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  buzzerBeep(1, 100);
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize secure WiFiClient
  wifiClient.setInsecure();
  setupMQTT();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;  // Smaller image for better transmission
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 20;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 15;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  Serial.println("Camera initialized successfully!");
  delay(2000);
}


void loop() {
  // CRITICAL: Read GPS data continuously
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    gps.encode(c);
  }

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // GPS status check every 5 seconds
  if (millis() - lastGPSCheck > 5000) {
    lastGPSCheck = millis();
    
    if (gps.location.isValid()) {
      buzzerBeep(2, 50); // GPS has fix
      Serial.printf("GPS: Lat=%.6f, Lon=%.6f, Sats=%d\n", 
                    gps.location.lat(), gps.location.lng(), gps.satellites.value());
    } else {
      buzzerBeep(1, 50); // Still searching
      Serial.printf("GPS searching... Chars=%lu, Sentences=%lu\n", 
                    gps.charsProcessed(), gps.sentencesWithFix());
    }
  }

  // Publish data every 10 seconds (SEPARATE from GPS check!)
  if (millis() - lastPublishTime >= publishInterval) {
    lastPublishTime = millis();

    double Latitude, Longitude;
    String timestamp;

    if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
      Latitude = gps.location.lat();
      Longitude = gps.location.lng();

      if (!gpsFixNotified) {
        Serial.println("*** GPS FIX ACQUIRED! ***");
        buzzerBeep(3, 200); // 3 long beeps = GPS fix acquired!
        gpsFixNotified = true;
      }

      // Time adjustment for Egypt (UTC+2)
      int hour = gps.time.hour() + 3;  // زودت واحد عشان التوقيت الصيفي
      if (hour >= 24) hour -= 24;

      char buffer[25];
      sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d",
              hour,
              gps.time.minute(),
              gps.time.second(),
              gps.date.day(),
              gps.date.month(),
              gps.date.year());
      timestamp = String(buffer);
      
      Serial.printf("Using REAL GPS data: %.6f, %.6f, %s\n", Latitude, Longitude, timestamp.c_str());
    } else {
      if (gpsFixNotified) {
        Serial.println("GPS Fix Lost!");
        buzzerBeep(4, 100);
        gpsFixNotified = false;
      }
      Latitude = 29;
      Longitude = 30;
      timestamp = "TEST_TIME";
      Serial.println("Using TEST data (no GPS fix)");
    }

    Serial.println("Capturing image...");
    String Image_encoded = captureAndEncodeImage();

    if (Image_encoded.length() == 0) {
      Serial.println("ERROR: Camera capture failed!");
      buzzerBeep(5, 100); // 5 beeps = camera error
      return;
    }

    // build JSON message
    id++;
    String msg = "{";
    msg += "\"id\":" + String(id) + ",";
    msg += "\"vehicle_id\":" + String(vechile_id) + ",";
    msg += "\"latitude\":" + String(Latitude, 6) + ",";
    msg += "\"longitude\":" + String(Longitude, 6) + ",";
    msg += "\"time\":\"" + timestamp + "\",";
    msg += "\"image\":\"" + Image_encoded + "\"";
    msg += "}";

    Serial.print("Publishing message... Size: ");
    Serial.print(msg.length());
    Serial.println(" bytes");
    
    bool published = mqttClient.publish(topic_publish, msg.c_str());
    
    if (published) {
      Serial.println("✓ Message published successfully!");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
        digitalWrite(BUZZER_PIN, LOW);
    } else {
      Serial.println("✗ FAILED to publish message!");
      buzzerBeep(6, 100); // 6 quick beeps = publish failed
      
      // Try to reconnect
      if (!mqttClient.connected()) {
        Serial.println("MQTT disconnected, reconnecting...");
        reconnect();
      }
    }
  }
  // Small delay to prevent watchdog reset
  delay(10);
}

String captureAndEncodeImage() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    return "";
  }
  Serial.printf("Image captured: %d bytes\n", fb->len);
  String base64Image = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb); 
  return base64Image;
}
