#include "esp_camera.h"
#include <WiFi.h>
#include "base64.h"
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>

// =============================
// indicator: Buzzer
#define BUZZER_PIN 0
// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ---- GPS -----
#define RXD2 14
#define TXD2 15
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
  mqttClient.setBufferSize(8000);  // New edit here

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
  Serial.println("WiFi Beeb");
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
  config.frame_size = FRAMESIZE_QVGA;  // edit image size
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 15;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
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
  
  // Give camera time to adjust
  delay(2000);
}


void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  double Latitude, Longitude;
  String timestamp;


  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
    Latitude = gps.location.lat();
    Longitude = gps.location.lng();

    if (!gpsFixNotified) {
      Serial.println("GPS Fix Acquired!");
      buzzerBeep(1, 400);
      Serial.println("GPS Beeb");
      gpsFixNotified = true;
    } 

    // Time adjustment for Egypt (UTC+2)
    int hour = gps.time.hour() + 2;
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
  }else{
    if (gpsFixNotified) {
      Serial.println("GPS Fix Lost!");
      gpsFixNotified = false;
    }
    Latitude = 29;
    Longitude = 30;
    timestamp = "TEST_TIME";
  }
    // Serial.println("Capturing and encoding image...");
    
    String Image_encoded = captureAndEncodeImage();

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

    Serial.println("Publishing message size: " + String(msg.length()) + " bytes");
    mqttClient.publish(topic_publish, msg.c_str());
    
    // Wait 10 seconds before next capture
    delay(10000);
}

String captureAndEncodeImage() {
  // Capture image
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("Camera capture failed!");
    return "";
  }
  
  // Serial.println("Image captured successfully!");
  // Serial.printf("Image size: %d bytes\n", fb->len);
  // Serial.printf("Image width: %d\n", fb->width);
  // Serial.printf("Image height: %d\n", fb->height);
  // Serial.printf("Image format: %s\n", fb->format == PIXFORMAT_JPEG ? "JPEG" : "Other");
  
  // Encode to Base64
  String base64Image = base64::encode(fb->buf, fb->len);
  
  // Serial.println("\n--- Base64 Encoding Results ---");
  // Serial.printf("Original image size: %d bytes\n", fb->len);
  // Serial.printf("Base64 encoded size: %d characters\n", base64Image.length());
  // Serial.printf("Size increase: %.2f%%\n", ((float)(base64Image.length() - fb->len) / fb->len) * 100);
  
  // // Print first 100 characters of Base64 string as sample
  // Serial.println("\nFirst 100 characters of Base64:");
  // Serial.println(base64Image.substring(0, 100));
  // Serial.println("...");
  
  
  // Return the frame buffer back to the driver
  esp_camera_fb_return(fb);
  
  return base64Image;
}
