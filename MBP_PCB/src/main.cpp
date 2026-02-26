#include <Arduino.h>
#include "driver/twai.h"
// current sensor library
#include <BL0942.h>
// Pin definitions for current sensor
#define BL0942_RX 17  
#define BL0942_TX 18

bl0942::BL0942 blSensor(Serial1);

// Callback function to handle received sensor data
void dataReceivedCallback(bl0942::SensorData &data) {
  Serial.print("Voltage: ");
  Serial.println(data.voltage);
  Serial.print("Current: ");
  Serial.println(data.current);
  // Serial.print("Power: ");
  // Serial.println(data.watt);
  // Serial.print("Energy: ");
  // Serial.println(data.energy);
  // Serial.print("Frequency: ");
  // Serial.println(data.frequency);
}

// Pin Definitions
#define TX_PIN GPIO_NUM_1
#define RX_PIN GPIO_NUM_2

// INTERFACE MODE: Must match Sender
#define CAN_MODE TWAI_MODE_NORMAL

void setup()
{
  pinMode(5, OUTPUT);   // Set pin 5 as an output
  digitalWrite(5, HIGH); // Turn pin 5 ON (HIGH voltage)
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, BL0942_RX, BL0942_TX); // Must be called by user
  blSensor.setup();  // Use default ModeConfig
  blSensor.onDataReceived(dataReceivedCallback);
  // delay(1000);

  // 1. Configure TWAI
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, CAN_MODE);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // 2. Install Driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    Serial.println("Receiver Driver installed.");
  }
  else
  {
    Serial.println("Receiver Driver install failed!");
    return;
  }

  // 3. Start Driver
  if (twai_start() == ESP_OK)
  {
    Serial.println("Receiver Driver started. Listening...");
  }
  else
  {
    Serial.println("Receiver Driver start failed!");
    return;
  }
}

void loop()
{
  blSensor.update();
  blSensor.loop(); 
  Serial.println("");
  delay(1000);

  twai_message_t rx_msg;

  // Receive message
  esp_err_t result = twai_receive(&rx_msg, pdMS_TO_TICKS(100)); // Polling every 100ms

  if (result == ESP_OK)
  {
    // Check if it is a standard data frame
    if (!rx_msg.rtr)
    {

      // Ensure we received the expected number of bytes for a double (8 bytes)
      if (rx_msg.data_length_code == sizeof(double))
      {

        double receivedFreq;

        // Copy the raw bytes from the message back into a double variable
        memcpy(&receivedFreq, rx_msg.data, sizeof(double));

        // Serial.printf("Received ID: 0x%x | Frequency: %.2f Hz\n", rx_msg.identifier, receivedFreq);
        Serial.printf("Vibration: %.2f Hz\n", receivedFreq);
      }
      else
      {
        Serial.printf("Received ID: 0x%x | Len: %d | Data is not a valid double\n", rx_msg.identifier, rx_msg.data_length_code);
      }
    }
  }
  else if (result != ESP_ERR_TIMEOUT)
  {
    // Report errors other than timeout
    Serial.printf("Read Error: 0x%x\n", result);
  }
}