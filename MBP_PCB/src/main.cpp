#include <Arduino.h>
#include "driver/twai.h"

// Pin Definitions
#define TX_PIN GPIO_NUM_1
#define RX_PIN GPIO_NUM_2

// INTERFACE MODE: Must match Sender
#define CAN_MODE TWAI_MODE_NORMAL 

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- SLAVE RECEIVER STARTING ---");

  // 1. Configure TWAI
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, CAN_MODE);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); 
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // 2. Install Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Receiver Driver installed.");
  } else {
    Serial.println("Receiver Driver install failed!");
    return;
  }

  // 3. Start Driver
  if (twai_start() == ESP_OK) {
    Serial.println("Receiver Driver started. Listening...");
  } else {
    Serial.println("Receiver Driver start failed!");
    return;
  }
}

void loop() {
  twai_message_t rx_msg;

  // Receive message
  esp_err_t result = twai_receive(&rx_msg, pdMS_TO_TICKS(100)); // Polling every 100ms

  if (result == ESP_OK) {
    // Check if it is a standard data frame, protects from printing empty/non-data frames
    if (!rx_msg.rtr) {
      Serial.printf("Received ID: 0x%x | Len: %d | Vibration: ", rx_msg.identifier, rx_msg.data_length_code);
      
      // Print the characters
      for (int i = 0; i < rx_msg.data_length_code; i++) {
        Serial.print((char)rx_msg.data[i]);
      }
      Serial.println();
    }
  } else if (result != ESP_ERR_TIMEOUT) {
    // Report errors other than timeout (empty bus)
    Serial.printf("Read Error: 0x%x\n", result);
  }
}