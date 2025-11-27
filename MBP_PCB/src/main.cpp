#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "arduinoFFT.h"
#include "driver/twai.h"
 
// ------------------- SENSOR & SYSTEM CONFIG -------------------
#define LIS3DHTR_ADDR 0x19
#define LIS3DHTR_CTRL_REG1 0x20
#define LIS3DHTR_CTRL_REG4 0x23
#define LIS3DHTR_OUT_X_L 0x28
 
#define SAMPLE_SIZE 512           // Must be power of 2o
#define SAMPLING_FREQUENCY 400    // Hz (max detectable freq = Fs/2 = 200 Hz)

// Pin Definitions
#define TX_PIN GPIO_NUM_1
#define RX_PIN GPIO_NUM_2

// INTERFACE MODE:
// Use TWAI_MODE_NORMAL if you have a CAN Transceiver (SN65HVD230, etc.)
// Use TWAI_MODE_NO_TRANSCEIVER if you are wiring GPIOs directly (Testing only)
#define CAN_MODE TWAI_MODE_NORMAL 
 
// ------------------- GLOBAL VARIABLES -------------------
WiFiClient client;
ArduinoFFT<double> FFT;
 
double vReal[SAMPLE_SIZE];
double vImag[SAMPLE_SIZE];
float SCALE = 0.000061f;  // For ±2g range
 
// ------------------- FUNCTION DECLARATIONS -------------------
void initLIS3DHTR();
void setRange(uint8_t range_g);
float readVibrationMagnitude();
 
// ------------------- SETUP -------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();
  initLIS3DHTR();
  setRange(2);

  // 1. Configure TWAI
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, CAN_MODE);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // 2. Install Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Sender Driver installed.");
  } else {
    Serial.println("Sender Driver install failed!");
    return;
  }

  // 3. Start Driver
  if (twai_start() == ESP_OK) {
    Serial.println("Sender Driver started.");
  } else {
    Serial.println("Sender Driver start failed!");
    return;
  }


}
 
// ------------------- MAIN LOOP -------------------
void loop() {
  unsigned long startTime;
  unsigned long elapsed;
  double mean = 0;
  double actualFs = 0;
 
  // ---- 1. Sampling ----
  startTime = micros();
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    unsigned long tStart = micros();
    vReal[i] = readVibrationMagnitude();
    mean += vReal[i];
    vImag[i] = 0;
    while (micros() - tStart < (1000000 / SAMPLING_FREQUENCY));  // precise sampling
  }
  elapsed = micros() - startTime;
  actualFs = SAMPLE_SIZE / (elapsed / 1000000.0);
  mean /= SAMPLE_SIZE;
 
  // Remove DC offset (bias)
  for (int i = 0; i < SAMPLE_SIZE; i++) vReal[i] -= mean;
 
  // ---- 2. FFT PROCESS ----
  FFT.windowing(vReal, SAMPLE_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLE_SIZE, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLE_SIZE);
 
  // ---- 3. FIND DOMINANT FREQUENCY ----
  double peak = 0;
  int peakIndex = 0;
  for (int i = 1; i < SAMPLE_SIZE / 2; i++) {
    if (vReal[i] > peak) {
      peak = vReal[i];
      peakIndex = i;
    }
  }
  double dominantFreq = (peakIndex * actualFs) / SAMPLE_SIZE;
  // ---- 5. SEND OR PRINT RESULTS ----
 
  // String payload = String("{\"freq\":") + String(dominantFreq, 2) +
  //                 ", \"Fs\":" + String(actualFs, 2) + "}";
 
  // String payload = String("{\"freq\":") + String(dominantFreq, 2) + "}";
  String payload = String(dominantFreq, 2);
  Serial.println(payload);
 
  delay(100);
  // Define the message to send
  // IMPORTANT: Standard CAN max data length is 8 BYTES.
  String msg = payload; // 8 Characters max
  
  twai_message_t tx_msg;
  tx_msg.identifier = 0x123;  // Identifier
  tx_msg.extd = 0;            // Standard ID (11-bit)
  tx_msg.rtr = 0;             // Data Frame
  
  // Cap length at 8 bytes to prevent errors
  int len = msg.length();
  if (len > 8) len = 8; 
  tx_msg.data_length_code = len;

  // Load the string into the data buffer
  for (int i = 0; i < len; i++) {
    tx_msg.data[i] = msg[i];
  }

  // Transmit
  Serial.print("Sending: " + msg.substring(0, len) + " ... ");
  
  // Send message
  esp_err_t result = twai_transmit(&tx_msg, pdMS_TO_TICKS(1000));

  if (result == ESP_OK) {
    Serial.println("Success!");
  } else {
    Serial.printf("Failed! Error: 0x%x ", result);
    
    // Diagnose specific errors
    if (result == ESP_ERR_TIMEOUT) Serial.println("(Timeout - Bus busy or no ACK)");
    else if (result == ESP_ERR_INVALID_ARG) Serial.println("(Invalid Argument - Check DLC)");
    else if (result == ESP_ERR_INVALID_STATE) Serial.println("(Driver not started)");
    else Serial.println();

    // Optional: Check bus status if failing repeatedly
    twai_status_info_t status_info;
    twai_get_status_info(&status_info);
    Serial.printf("Bus State: %d, TX Error Counter: %d\n", status_info.state, status_info.tx_error_counter);
  }

  delay(1000);
}
 
// ------------------- LIS3DHTR INIT -------------------
void initLIS3DHTR() {
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_CTRL_REG1);
  Wire.write(0x77); // 100Hz, XYZ enabled
  Wire.endTransmission();
}
 
void setRange(uint8_t range_g) {
  uint8_t reg_value = 0x00;
  switch (range_g) {
    case 2:  reg_value = 0x00; SCALE = 0.000061f; break;
    case 4:  reg_value = 0x10; SCALE = 0.000122f; break;
    case 8:  reg_value = 0x20; SCALE = 0.000244f; break;
    case 16: reg_value = 0x30; SCALE = 0.000732f; break;
  }
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_CTRL_REG4);
  Wire.write(reg_value);
  Wire.endTransmission();
}
 
// ------------------- READ VIBRATION MAGNITUDE -------------------
float readVibrationMagnitude() {
  int16_t x, y, z;
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_OUT_X_L | 0x80);
  Wire.endTransmission(false);
  Wire.requestFrom(LIS3DHTR_ADDR, 6);
 
  if (Wire.available() == 6) {
    x = Wire.read() | (Wire.read() << 8);
    y = Wire.read() | (Wire.read() << 8);
    z = Wire.read() | (Wire.read() << 8);
  }
 
  float xg = x * SCALE;
  float yg = y * SCALE;
  float zg = z * SCALE;
 
  float mag = sqrt(xg * xg + yg * yg + zg * zg);
  return mag;
}