# Predictive Maintenance for Industrial Sewing Machines 🔧

### FAG-TFT Based Forecasting and Fault Classification System

A deep learning-based predictive maintenance system for industrial sewing machines, developed and deployed on a Juki single-needle lockstitch machine at MAS Holdings, Sri Lanka.

---

## Overview

This system uses real-time IoT sensor data to:
- **Warn** operators before a breakdown happens (Forecaster)
- **Identify** the current fault in real time (Classifier)

---

## Models 🧠

### FAG-TFT Forecaster
- **Model A** — Binary forecaster: predicts whether a breakdown is coming within the next 15 pedal presses
- **Model B** — Type forecaster: identifies which of the 3 fault types is approaching

### FAG-TFT Classifier
Identifies the current machine state in real time across 4 classes:
- Healthy
- High Thread Tension
- High Foot Pressure
- Running with No Thread

---

## Results 📊

| Model | Accuracy | F1 Score |
|---|---|---|
| LSTM Baseline | 94.92% | 0.9486 |
| GRU Baseline | 94.09% | 0.9408 |
| FAG-TFT Forecaster | 96.13% | 0.9603 |
| FAG-TFT Classifier | 99.26% | 0.9928 |

---

## Dataset 📁

- 50,000 pedal press records from a Juki single-needle lockstitch machine
- 60 vibration frequency bands (10 Hz to 610 Hz)
- 6 electrical parameters (voltage and current)
- 1 temporal feature (time between pedal presses)
- Stored in real time to Azure SQL Database

---

## Project Structure
