#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "arduinoFFT.h"
 
// ------------------- SENSOR & SYSTEM CONFIG -------------------
#define LIS3DHTR_ADDR 0x19
#define LIS3DHTR_CTRL_REG1 0x20
#define LIS3DHTR_CTRL_REG4 0x23
#define LIS3DHTR_OUT_X_L 0x28
 
#define CURRENT_SENSOR_PIN 36     // Analog pin for ACS712

#define SAMPLE_SIZE 512           // Must be power of 2
#define SAMPLING_FREQUENCY 400    // Hz (max detectable freq = Fs/2 = 200 Hz)
 
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
  Serial.println("\n=== MACHINE VIBRATION + CURRENT ANALYZER ===");
  Wire.begin();
 
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
 
  initLIS3DHTR();
  setRange(2);
  Serial.println("           LIS3DHTR initialized");
  Serial.println("\n");
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

  String payload = String("{\"freq\":") + String(dominantFreq, 2) + "}";
  Serial.println(payload);
 
  delay(100);
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