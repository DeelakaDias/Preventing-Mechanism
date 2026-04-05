#include "driver/twai.h"
#include <Preferences.h>
#include <WiFi.h>
#include <BL0942.h>
#include <cstdlib>
#include <string.h>
#include <stdbool.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
 
// Azure IoT Includes
#include <az_core.h>
#include <az_iot.h>
#include "AzIoTSasToken.h"
#include "SerialLogger.h"
#include "iot_configs.h"
#include "iot_cred.h"
#include "azure_ca.h"
#include <time.h>
#include <mqtt_client.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
 
// -------------------------------------------------------------------------
// Configuration
// -------------------------------------------------------------------------
 
// CAN Pins
#define TX_PIN 1
#define RX_PIN 2
 
// BL0942 Pins
#define BL_RX_PIN 17  // Connect to BL0942 TX
#define BL_TX_PIN 18  // Connect to BL0942 RX
 
// CAN Configuration
#define CAN_ID_CMD 0x100
#define CAN_ID_RES 0x102
 
// Thresholds
#define DEFAULT_IDLE_CURRENT 0.1  // Amps
#define IDLE_TOLERANCE 0.3       // Amps
 
// Azure IoT
#define MQTT_QOS1 1
#define DO_NOT_RETAIN_MSG 0
#define SAS_TOKEN_DURATION_IN_MINUTES 60
#define INCOMING_DATA_BUFFER_SIZE 100000
static char incoming_data[INCOMING_DATA_BUFFER_SIZE];
 
// -------------------------------------------------------------------------
// Globals
// -------------------------------------------------------------------------
 
Preferences preferences;
bl0942::BL0942 blSensor(Serial1);
 
// Azure IoT Variables
static az_iot_hub_client client;
static esp_mqtt_client_handle_t mqtt_client;
bool mqttConnected = false;
static char mqtt_client_id[128];
static char mqtt_username[128];
static char mqtt_password[256];
static uint8_t sas_signature_buffer[256];
static char telemetry_topic[128];
static uint8_t telemetry_payload[2048];
 
// Connection Check Globals
unsigned long lastConnectionCheckTime = 0;
const unsigned long CONNECTION_CHECK_INTERVAL = 5000; // Check every 5 seconds
 
// Device Settings (Populated from MAC lookup)
String device_id_str;
const char* device_id_ptr = NULL;
const char* device_key_ptr = NULL;
String SpreaderID;
String wifi_ssid;
String wifi_password;
String deviceMacAddress = "";
 
const char* host = IOT_CONFIG_IOTHUB_FQDN;
const char* mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
const int mqtt_port = AZ_IOT_DEFAULT_MQTT_CONNECT_PORT;
 
bool updateInProgress = false;
 
// Helper to check if string is null/empty
bool isSettingsValid() {
    return (device_id_ptr != NULL && device_key_ptr != NULL && !wifi_ssid.isEmpty());
}
 
 
// State Machine
enum MachineState {
  STATE_IDLE,
  STATE_CALIBRATING,
  STATE_RUNNING,
  STATE_WAITING_FOR_CAN
};
 
MachineState currentState = STATE_IDLE;
 
// Calibration
float idleCurrentThreshold = DEFAULT_IDLE_CURRENT;
unsigned long calibrationStartTime = 0;
float calibrationMaxCurrent = 0;
 
// Metrics
struct RunMetrics {
  float v_min, v_max, v_avg;
  float a_min, a_max, a_avg;
  unsigned long startTime;
  unsigned long endTime;
  double v_sum, a_sum;
  unsigned long sampleCount;
} metrics;
 
// BL0942 Data
struct BL0942Data {
  float voltage;
  float current;
  float power;
  bool valid;
};
 
bool newData = false;
 
BL0942Data currentBLData;
 
// CAN Response
String canResponseBuffer = "";
unsigned long canWaitStartTime = 0;
 
// -------------------------------------------------------------------------
// Azure IoT & Helper Functions
// -------------------------------------------------------------------------
 
String getMacAddress() {
  uint8_t base_mac_addr[6];
  esp_read_mac(base_mac_addr, ESP_MAC_WIFI_STA);
  char mac_str[13] = { 0 };
  sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", base_mac_addr[0], base_mac_addr[1], base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);
  return String(mac_str);
}
 
void findDeviceID(const String &macAddress) {
  Logger.Info("Finding Settings for MAC: " + macAddress);
  char macLower[13];
  macAddress.toCharArray(macLower, sizeof(macLower));
 
  bool found = false;
  int deviceCount = sizeof(devices) / sizeof(devices[0]);
 
  for (int i = 0; i < deviceCount; i++) {
    if (strcmp(macLower, devices[i][0]) == 0) {
      device_id_ptr = devices[i][0]; // Using MAC as DeviceID
      device_key_ptr = devices[i][1];
      wifi_ssid = String(devices[i][2]);
      wifi_password = String(devices[i][3]);
      found = true;
      Logger.Info("Device Found. ID: " + String(device_id_ptr) + ", SSID: " + wifi_ssid);
      break;
    }
  }
 
  if (!found) {
      Logger.Error("Device MAC not found in credentials list!");
  }
}
 
void connectToWiFi() {
  if (wifi_ssid.isEmpty()) {
      Logger.Error("WiFi SSID not set! Cannot connect.");
      return;
  }
 
  Logger.Info("Connecting to WiFi: " + wifi_ssid);
  if(wifi_password==""){
    Logger.Info("Connecting to Open Network");
     WiFi.begin(wifi_ssid.c_str());
  } else {
    Logger.Info("Using Password");
     WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
 
 
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 100) {
      delay(500);
      Serial.print(".");
      attempts++;
  }
  Serial.println();
 
  if (WiFi.status() == WL_CONNECTED) {
      Logger.Info("WiFi Connected. IP: " + WiFi.localIP().toString());
  } else {
      Logger.Error("WiFi Connection Failed!");
  }
}
 
void initializeTime() {
  Logger.Info("Setting time using SNTP");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(NULL);
  int attempts = 0;
  while (now < 1510592825 && attempts < 20) { // 2017 cutoff
      delay(500);
      Serial.print(".");
      now = time(NULL);
      attempts++;
  }
  Serial.println();
 
  if (now > 1510592825) {
      Logger.Info("Time synced: " + String(ctime(&now)));
  } else {
      Logger.Error("Time sync failed!");
  }
}
 
// MQTT Event Handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
      Logger.Info("MQTT Connected");
      mqttConnected = true;
      break;
    case MQTT_EVENT_DISCONNECTED:
      Logger.Info("MQTT Disconnected");
      mqttConnected = false;
      break;
    case MQTT_EVENT_ERROR:
      Logger.Error("MQTT Error");
      mqttConnected = false;
      break;
    case MQTT_EVENT_DATA:
      {
        int i;
        Logger.Info("MQTT event MQTT_EVENT_DATA");
 
        for (i = 0; i < event->topic_len; i++) {
          incoming_data[i] = event->topic[i];
        }
        incoming_data[i] = '\0';
        Logger.Info("Topic: " + String(incoming_data));
        Serial.println(event->data_len);
        for (i = 0; i < event->data_len; i++) {
          incoming_data[i] = event->data[i];
        }
        incoming_data[i] = '\0';
        Logger.Info("Data: " + String(incoming_data));
 
        // return;
        if (String(incoming_data) == "UPDATE") {
          Serial.println("Update request received!");
          httpsUpdate();
        }
      }
    default:
      break;
  }
}
 
static int initializeIoTHubClient() {
  if (device_id_ptr == NULL || device_key_ptr == NULL) return 1;
 
  // Cleanup existing client if any
  if (mqtt_client != NULL) {
      esp_mqtt_client_stop(mqtt_client);
      esp_mqtt_client_destroy(mqtt_client);
      mqtt_client = NULL;
      mqttConnected = false;
  }
 
  AzIoTSasToken sasToken(
    &client,
    az_span_create_from_str((char*)device_key_ptr),
    AZ_SPAN_FROM_BUFFER(sas_signature_buffer),
    AZ_SPAN_FROM_BUFFER(mqtt_password)
  );
 
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
 
  if (az_result_failed(az_iot_hub_client_init(
        &client,
        az_span_create_from_str((char*)host),
        az_span_create_from_str((char*)device_id_ptr),
        &options))) {
    Logger.Error("Failed initializing Azure IoT Hub client");
    return 1;
  }
 
  size_t client_id_length;
  if (az_result_failed(az_iot_hub_client_get_client_id(
        &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length))) {
    Logger.Error("Failed getting client id");
    return 1;
  }
 
  if (az_result_failed(az_iot_hub_client_get_user_name(
        &client, mqtt_username, sizeof(mqtt_username), NULL))) {
    Logger.Error("Failed to get MQTT info");
    return 1;
  }
 
  if (sasToken.Generate(SAS_TOKEN_DURATION_IN_MINUTES) != 0) {
    Logger.Error("Failed generating SAS token");
    return 1;
  }
 
  esp_mqtt_client_config_t mqtt_config;
  memset(&mqtt_config, 0, sizeof(mqtt_config));
 
  mqtt_config.uri = mqtt_broker_uri;
  mqtt_config.port = mqtt_port;
  mqtt_config.client_id = mqtt_client_id;
  mqtt_config.username = mqtt_username;
  mqtt_config.password = (const char *)az_span_ptr(sasToken.Get());
  mqtt_config.cert_pem = (const char *)ca_pem;
  mqtt_config.keepalive = 30; // Ensure keepalive is set
 
  mqtt_client = esp_mqtt_client_init(&mqtt_config);
  if (mqtt_client == NULL) {
    Logger.Error("Failed creating mqtt client");
    return 1;
  }
 
  esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
 
  return 0;
}
 
void maintainConnection() {
    if (millis() - lastConnectionCheckTime < CONNECTION_CHECK_INTERVAL) return;
    lastConnectionCheckTime = millis();
 
    // 1. Check WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Logger.Info("WiFi lost. Reconnecting...");
        WiFi.disconnect();
        if(wifi_password==""){
             WiFi.begin(wifi_ssid.c_str());
        } else {
             WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
        }
        return;
    }
 
    // 2. Check MQTT
    if (!mqttConnected) {
        Logger.Info("MQTT Disconnected. Attempting to Initialize/Reconnect...");
        initializeIoTHubClient();
    }
}
 
////////////////////////////////////////////////////////////////////////////////
// Application Logic
////////////////////////////////////////////////////////////////////////////////
 
void dataReceivedCallback(bl0942::SensorData &data) {
  currentBLData.voltage = data.voltage;
  currentBLData.current = data.current;
  currentBLData.valid = true;
  newData = true;
}
 
String extractFreqResValue(String json) {
  int keyIndex = json.indexOf("\"freqRes\":\"");
  if (keyIndex == -1) return "";
 
  int startQuote = json.indexOf("\"", keyIndex + 10);
  if (startQuote == -1) return "";
  int endQuote = json.indexOf("\"", startQuote + 1);
  if (endQuote == -1) return "";
  return json.substring(startQuote + 1, endQuote);
}
 
void loadCalibration() {
  preferences.begin("iot_app", true);
  idleCurrentThreshold = preferences.getFloat("idle_curr", DEFAULT_IDLE_CURRENT);
  preferences.end();
  Serial.printf("Loaded Idle Threshold: %.5f A\n", idleCurrentThreshold);
}
 
void saveCalibration(float value) {
  preferences.begin("iot_app", false);
  preferences.putFloat("idle_curr", value);
  preferences.end();
  idleCurrentThreshold = value;
  Serial.printf("Saved Idle Threshold: %.5f A\n", idleCurrentThreshold);
}
 
void sendCANCommand(uint8_t cmd) {
  twai_message_t message;
  message.identifier = CAN_ID_CMD;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = 1;
  message.data[0] = cmd;
 
  if (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.printf("Sent CAN Command: 0x%02X\n", cmd);
  } else {
    Serial.println("Failed to send CAN Command");
  }
}
 
void resetMetrics() {
  metrics.v_min = 9999;
  metrics.v_max = 0;
  metrics.v_avg = 0;
  metrics.a_min = 9999;
  metrics.a_max = 0;
  metrics.a_avg = 0;
  metrics.v_sum = 0;
  metrics.a_sum = 0;
  metrics.sampleCount = 0;
  metrics.startTime = millis();
}
 
void updateMetrics() {
  if (currentBLData.voltage < metrics.v_min) metrics.v_min = currentBLData.voltage;
  if (currentBLData.voltage > metrics.v_max) metrics.v_max = currentBLData.voltage;
 
  if (currentBLData.current < metrics.a_min) metrics.a_min = currentBLData.current;
  if (currentBLData.current > metrics.a_max) metrics.a_max = currentBLData.current;
 
  metrics.v_sum += currentBLData.voltage;
  metrics.a_sum += currentBLData.current;
  metrics.sampleCount++;
}
 
void finalizeMetrics() {
  metrics.endTime = millis();
  if (metrics.sampleCount > 0) {
    metrics.v_avg = metrics.v_sum / metrics.sampleCount;
    metrics.a_avg = metrics.a_sum / metrics.sampleCount;
  }
}
 
void sendDataToAzure() {
  if (!mqttConnected || device_id_ptr == NULL) {
      Logger.Error("Cannot send data: MQTT not connected or Device ID unknown");
      return;
  }
 
  String freqResVal = "";
  if (canResponseBuffer.length() > 0) {
      String extracted = extractFreqResValue(canResponseBuffer);
      if (extracted.isEmpty()) {
          freqResVal = canResponseBuffer; // Fallback
          if (freqResVal.startsWith("{") && freqResVal.endsWith("}")) {
              // try extract again? if failed, maybe empty?
              // assuming safely
          }
      } else {
          freqResVal = extracted;
      }
  }
  if (freqResVal.isEmpty()) freqResVal = "null";
 
  unsigned long cycleTime = metrics.endTime - metrics.startTime;
  unsigned long epochT = (unsigned long)time(NULL);
 
  DynamicJsonDocument doc(2048);
  JsonObject root = doc.createNestedObject("KreedaSense");
  JsonObject payload = root.createNestedObject("payload");
 
  payload["DeviceID"] = deviceMacAddress;
  payload["V_Min"] = metrics.v_min;
  payload["V_Max"] = metrics.v_max;
  payload["V_Avg"] = metrics.v_avg;
  payload["A_Min"] = metrics.a_min;
  payload["A_Max"] = metrics.a_max;
  payload["A_Avg"] = metrics.a_avg;
  payload["freqRes"] = freqResVal;
  payload["cycleTime"] = cycleTime;
  payload["epochT"] = epochT;
 
  String output;
  serializeJson(doc, output);
 
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL))) {
    Logger.Error("Failed to get telemetry topic");
    return;
  }
 
  if (esp_mqtt_client_publish(
          mqtt_client,
          telemetry_topic,
          output.c_str(),
          output.length(),
          MQTT_QOS1,
          DO_NOT_RETAIN_MSG) == -1) {
    Logger.Error("Failed publishing");
  } else {
    Logger.Info("Message published successfully");
  }
}
 
void printJSON() {
  String mac = getMacAddress();
  unsigned long cycleTime = (metrics.endTime - metrics.startTime);
 
  Serial.print("{\"KreedaSense\":{");
  Serial.printf("\"DeviceID\":\"%s\",", mac.c_str());
  Serial.printf("\"V_Min\":%.5f,\"V_Max\":%.5f,\"V_Avg\":%.5f,", metrics.v_min, metrics.v_max, metrics.v_avg);
  Serial.printf("\"A_Min\":%.5f,\"A_Max\":%.5f,\"A_Avg\":%.5f,", metrics.a_min, metrics.a_max, metrics.a_avg);
 
  Serial.print("\"freqRes\":\"");
  if (canResponseBuffer.length() > 0) {
    String val = extractFreqResValue(canResponseBuffer);
    Serial.print(val.isEmpty() ? "null" : val);
  } else {
    Serial.print("null");
  }
  Serial.print("\",");
 
  Serial.printf("\"cycleTime\":%lu,", cycleTime);
  Serial.printf("\"epochT\":%lu", (unsigned long)time(NULL));
  Serial.println("}}");
}
 
 
// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
 
void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
 
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, BL_RX_PIN, BL_TX_PIN);
  while (!Serial) delay(10);
  delay(100);
 
  Logger.Info("System Starting...");
 
  // Get MAC and Settings
  deviceMacAddress = getMacAddress();
  findDeviceID(deviceMacAddress);
 
  // Connect WiFi & Azure
  connectToWiFi();
  initializeTime();
  if (isSettingsValid() && WiFi.status() == WL_CONNECTED) {
      if(initializeIoTHubClient() == 0) {
          Logger.Info("Azure IoT Client Initialized");
      } else {
          Logger.Error("Azure IoT Client Initialization Failed");
      }
  } else {
      Logger.Error("Skipping Azure Init: Missing Settings or WiFi");
  }
 
  WiFi.mode(WIFI_STA);
 
  blSensor.setup();
  blSensor.onDataReceived(dataReceivedCallback);
 
  loadCalibration();
 
  // Initialize TWAI (CAN) Driver
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  g_config.alerts_enabled = TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED;
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
 
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN Driver installed");
  } else {
    Serial.println("Failed to install CAN driver");
  }
 
  if (twai_start() == ESP_OK) {
    Serial.println("CAN Driver started");
  } else {
    Serial.println("Failed to start CAN driver");
  }
 
  Serial.println("System Ready");
}
 
// -------------------------------------------------------------------------
// Loop
// -------------------------------------------------------------------------
 
void loop() {
  // 0. Maintain Connections
  maintainConnection();
 
  // 1. Handle Serial Commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == "calib") {
      Serial.println("Starting Calibration (5s)...");
      currentState = STATE_CALIBRATING;
      calibrationStartTime = millis();
      calibrationMaxCurrent = 0;
    } else if (input.equalsIgnoreCase("start")) {
      Serial.println("Manual Start Triggered");
      sendCANCommand(0x01);  // Send Start
      resetMetrics();
      updateMetrics();  // Include current sample
      currentState = STATE_RUNNING;
    } else if (input.equalsIgnoreCase("finish")) {
      if (currentState == STATE_RUNNING) {
        Serial.println("Manual Finish Triggered");
        sendCANCommand(0x02);  // Send Stop
        finalizeMetrics();
        canResponseBuffer = "";
        canWaitStartTime = millis();
        currentState = STATE_WAITING_FOR_CAN;
      } else {
        Serial.println("Ignored Finish: Not in RUNNING state");
      }
    }
  }
 
  // 2. Read BL0942
  blSensor.update();
  blSensor.loop();
  if (currentState != STATE_CALIBRATING) {
    if (currentState == STATE_RUNNING) {
      Serial.println(" Machine Running!!");
    }
  }
 
  // 3. State Machine
  switch (currentState) {
    case STATE_IDLE:
      if (newData) {
        if (currentBLData.current > idleCurrentThreshold) {
          Serial.printf("Start Detected (Current: %.5f > %.5f)\n", currentBLData.current, idleCurrentThreshold);
          sendCANCommand(0x01);
          resetMetrics();
          updateMetrics();
          currentState = STATE_RUNNING;
        }
      }
      break;
 
    case STATE_CALIBRATING:
      if (newData) {
        if (currentBLData.current > calibrationMaxCurrent) {
          calibrationMaxCurrent = currentBLData.current;
        }
      }
      if (millis() - calibrationStartTime >= 5000) {
        float newThreshold = calibrationMaxCurrent + IDLE_TOLERANCE;
        saveCalibration(newThreshold);
        currentState = STATE_IDLE;
        Serial.println("Calibration Done.");
      }
      break;
 
    case STATE_RUNNING:
      if (newData) {
        updateMetrics();
        if (currentBLData.current <= idleCurrentThreshold) {
          Serial.printf("Stop Detected (Current: %.5f <= %.5f)\n", currentBLData.current, idleCurrentThreshold);
          sendCANCommand(0x02);
          finalizeMetrics();
          canResponseBuffer = "";
          canWaitStartTime = millis();
          currentState = STATE_WAITING_FOR_CAN;
        }
      }
      break;
 
    case STATE_WAITING_FOR_CAN:
      // Check for Timeout
      if (millis() - canWaitStartTime > 5000) {
        Serial.println("CAN Response Timeout - Sending Partial Data");
        printJSON();
        sendDataToAzure();
        currentState = STATE_IDLE;
      }
      break;
  }
  newData = false;
 
  // 4. Handle CAN Alerts
  uint32_t alerts_triggered;
  if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(0)) == ESP_OK) {
    if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) Serial.println("Message Sent Successfully");
    if (alerts_triggered & TWAI_ALERT_TX_FAILED) Serial.println("Message Send Failed");
  }
 
  // 5. Handle Received CAN Messages
  twai_message_t message;
  while (twai_receive(&message, pdMS_TO_TICKS(0)) == ESP_OK) {
    if (message.identifier == CAN_ID_RES && message.data_length_code > 0) {
      if (currentState == STATE_WAITING_FOR_CAN) {
        for (int i = 0; i < message.data_length_code; i++) {
          char c = (char)message.data[i];
          if (c != '\0') canResponseBuffer += c;
        }
       
        if (canResponseBuffer.endsWith("}")) {
          // Received full JSON from CAN
          printJSON();
          sendDataToAzure();
          currentState = STATE_IDLE;
        }
      }
    }
  }
}
 
void httpsUpdate() {
  updateInProgress = true;
  Serial.println("Starting firmware update...");
  sendTelemetry("{\"event\":\"Updating\",\"status\":\"triggered\"}");
  HTTPClient http;
  String firmwareUrl = "";
  http.begin(firmwareUrl);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    if (contentLength > 0) {
      WiFiClient *stream = http.getStreamPtr();
      Update.onProgress([](size_t progress, size_t total) {
        int progressVal = (progress * 100) / total;
        Serial.printf("Progress: %u%%\n", progressVal);
      });
      if (Update.begin(contentLength)) {
        size_t written = Update.writeStream(*stream);
        Serial.println("Written: " + String(written) + " bytes");
        if (written == contentLength) {
          Serial.println("Firmware written successfully.");
        } else {
          Serial.println("Mismatch: Written (" + String(written) + ") vs Content-Length (" + String(contentLength) + ")");
        }
        if (Update.end()) {
          if (Update.isFinished()) {
            Serial.println("OTA Update complete. Rebooting...");
            ESP.restart();
          } else {
            Serial.println("Update failed. Not finished.");
          }
        } else {
          Serial.printf("Update failed. Error #: %d\n", Update.getError());
        }
      } else {
        Serial.println("Invalid content length from server.");
      }
    } else {
      Serial.println("Invalid content length from server.");
    }
  } else {
    Serial.println("HTTP GET failed. Code: " + String(httpCode));
  }
  http.end();
}
 
void sendTelemetry(const char *json) {
  if (!mqttConnected)
    return;
 
  size_t topic_len = sizeof(telemetry_topic);
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
        &client, nullptr, telemetry_topic, topic_len, &topic_len))) {
    Logger.Error("Failed getting telemetry topic");
    return;
  }
 
  int msg_id = esp_mqtt_client_publish(
    mqtt_client,
    telemetry_topic,
    json,
    strlen(json),
    MQTT_QOS1,
    DO_NOT_RETAIN_MSG);
 
  if (msg_id == -1)
    Logger.Error("Telemetry publish failed");
  else
    Logger.Info("Telemetry sent: " + String(json));
}
 