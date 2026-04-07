# FAG-TFT - Frequency Aware Gated Temporal Fusion Transformer

A machine learning system for predictive maintenance using time-series forecasting,
applied to industrial sewing machines - built as part of my undergraduate thesis at IIT.

## 🔍 What it does
The system works in two stages:
1. Detects whether a fault is coming (97.37% accuracy)
2. Identifies the fault type - HTT, HFP, or RNT (91.79% accuracy)

It reads 67 features from vibration and electrical sensors attached to the machine.

## 🔌 Hardware & IoT
Two ESP32 boards work together to collect sensor data during each sewing cycle:

**Vibration Board** - reads acceleration via LIS3DHTR sensor, runs FFT and sends 60 frequency band values (10Hz - 600Hz) over CAN bus.

**Main Board** - monitors voltage and current via BL0942 sensor, triggers the vibration board via CAN, combines both readings and sends the full 67-feature payload to Azure IoT Hub via MQTT.
