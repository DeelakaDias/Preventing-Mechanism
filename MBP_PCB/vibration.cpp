#include "LIS3DHTR.h"
#include <driver/twai.h>
#include <vector>
#include <Arduino.h>
#include <arduinoFFT.h>
 
// -------------------------------------------------------------------------
// Configuration
// -------------------------------------------------------------------------
 
// CAN Pins
#define RX_PIN 2
#define TX_PIN 1
 
// LIS3DHTR
LIS3DHTR<TwoWire> LIS;
constexpr uint16_t SAMPLE_RATE = 1344;   // LIS3DHTR max in High Res mode
constexpr uint16_t SAMPLES     = 256;    // Power of 2 for FFT
constexpr uint8_t  TOP_N       = 25;     // (Unused in final JSON req, but kept from snippet)
 
// -------------------------------------------------------------------------
// Globals
// -------------------------------------------------------------------------
 
bool collecting = false;
unsigned long startTime = 0;
std::vector<double> vReal;
std::vector<double> vImag;
 
// FFT Object
ArduinoFFT<double> FFT = ArduinoFFT<double>(nullptr, nullptr, 0, 0);
 
// -------------------------------------------------------------------------
// CAN Setup & Helper Functions
// -------------------------------------------------------------------------
 
void setupCAN() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // Assume 500kbps, can be changed
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
 
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }
 
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
    }
}
 
void sendCANMessage(uint32_t id, uint8_t* data, uint8_t len) {
    twai_message_t message;
    message.identifier = id;
    message.extd = 0;
    message.data_length_code = len;
    memcpy(message.data, data, len);
    twai_transmit(&message, pdMS_TO_TICKS(100));
}
 
void sendLargeStringOverCAN(String data) {
    // Fragment the string into 8-byte chunks
    int len = data.length();
    int offset = 0;
    while (offset < len) {
        uint8_t chunk[8] = {0};
        int chunkLen = 0;
        for (int i = 0; i < 8 && offset < len; i++) {
            chunk[i] = (uint8_t)data[offset++];
            chunkLen++;
        }
        // Send chunk with ID 0x102 (Response)
        sendCANMessage(0x102, chunk, 8); // Always send 8 bytes or DLC=8? Standard CAN is usually fixed frame or DLC.
                                         // Send 8 bytes, padding with 0 if needed, or just send chunkLen.
                                       
        delay(5); // Small delay to prevent bus flooding
    }
}
 
// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
 
void setup() {
    Serial.begin(115200);
    while (!Serial);
 
    // Init LIS3DHTR
    LIS.begin(Wire, 0x19);
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_5KHZ);
    LIS.setFullScaleRange(LIS3DHTR_RANGE_2G);
    LIS.setHighSolution(true);
 
    // Init CAN
    setupCAN();
 
    Serial.println("System Ready. Waiting for CAN START...");
}
 
// -------------------------------------------------------------------------
// Main Loop
// -------------------------------------------------------------------------
 
void loop() {
    // Check for CAN messages
    twai_message_t message;
    if (twai_receive(&message, 0) == ESP_OK) {
        // Check for START (ID 0x100, Data[0] == 0x01)
        if (message.identifier == 0x100 && message.data_length_code > 0) {
            if (message.data[0] == 0x01) {
                Serial.println("START received (CAN). Collecting...");
                collecting = true;
                startTime = millis();
                vReal.clear();
                vImag.clear();
            }
            // Check for FINISH (ID 0x100, Data[0] == 0x02)
            else if (message.data[0] == 0x02) {
                Serial.println("FINISH received (CAN). Processing...");
                collecting = false;
                processAndSend();
            }
        }
    }
 
    // Check for Serial messages
    if (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '1' || c == 's' || c == 'S') {
            Serial.println("START received (Serial). Collecting...");
            collecting = true;
            startTime = millis();
            vReal.clear();
            vImag.clear();
        } else if (c == '2' || c == 'f' || c == 'F') {
            Serial.println("FINISH received (Serial). Processing...");
            collecting = false;
            processAndSend();
        }
    }
 
    // Collection
    if (collecting) {
        // Read accelerometer
        // This returns G values (float).
        float rx, ry, rz;
        LIS.getAcceleration(&rx, &ry, &rz);
       
        // Calculate magnitude
        double x = (double)rx;
        double y = (double)ry;
        double z = (double)rz;
       
        double mag = sqrt(x*x + y*y + z*z);
        vReal.push_back(mag);
        vImag.push_back(0.0);
    }
}
 
// -------------------------------------------------------------------------
// Processing
// -------------------------------------------------------------------------
 
void processAndSend() {
    if (vReal.empty()) {
        Serial.println("No data collected.");
        return;
    }
    // Average the magnitudes of the FFT bins across all chunks.
 
    int totalSamples = vReal.size();
    int numChunks = totalSamples / SAMPLES;
   
    if (numChunks == 0) {
        Serial.println("Not enough samples for one FFT.");
        return;
    }
 
    Serial.printf("Processing %d samples in %d chunks...\n", totalSamples, numChunks);
 
    // Prepare a vector to hold the sum of magnitudes for each bin
    std::vector<double> avgMagnitudes(SAMPLES / 2, 0.0);
 
    for (int i = 0; i < numChunks; i++) {
        // Copy chunk
        std::vector<double> chunkReal(SAMPLES);
        std::vector<double> chunkImag(SAMPLES, 0.0);
       
        for (int j = 0; j < SAMPLES; j++) {
            chunkReal[j] = vReal[i * SAMPLES + j];
        }
 
        // Perform FFT
        ArduinoFFT<double> fftChunk = ArduinoFFT<double>(chunkReal.data(), chunkImag.data(), SAMPLES, SAMPLE_RATE);
       
        fftChunk.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        fftChunk.compute(FFT_FORWARD);
        fftChunk.complexToMagnitude();
 
        // Accumulate
        for (int j = 0; j < SAMPLES / 2; j++) {
            avgMagnitudes[j] += chunkReal[j];
        }
    }
 
    // Average
    for (int j = 0; j < SAMPLES / 2; j++) {
        avgMagnitudes[j] /= numChunks;
    }
 
    // Construct JSON
   
    String json = "{\"freqRes\":\"";
   
    double resolution = (double)SAMPLE_RATE / SAMPLES;
    // So we start from 10Hz.
    for (int targetFreq = 10; targetFreq <= 600; targetFreq += 10) {
        // Find nearest bin index
        int binIndex = (int)((targetFreq / resolution) + 0.5);
       
        if (binIndex >= SAMPLES / 2) binIndex = (SAMPLES / 2) - 1;
       
        // Scale by 1000 to get milli-Gs
        int mag = (int)(avgMagnitudes[binIndex] * 1000);
       
        json += String(targetFreq) + "," + String(mag);
        if (targetFreq < 600) {
            json += ",";
        }
    }
    json += "\"}";
 
    Serial.println("Sending JSON over CAN...");
    Serial.println(json);
   
    sendLargeStringOverCAN(json);
    Serial.println("Done.");
}
 
 