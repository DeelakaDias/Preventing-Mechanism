# Predictive Maintenance for Industrial Sewing Machines 🔧

### FAG-TFT Based Forecasting and Fault Classification System

A deep learning-based predictive maintenance system for industrial sewing machines.

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

| Notebook | Description |
|---|---|
| `00_Data_Preparation.ipynb` | Data preprocessing and feature engineering |
| `01_Forecasting_LSTM_Baseline.ipynb` | LSTM baseline model |
| `02_Forecasting_GRU_Baseline.ipynb` | GRU baseline model |
| `03_FAG_TFT_Forecaster.ipynb` | FAG-TFT Forecaster (Model A & B) |
| `04_FAG_TFT.ipynb` | FAG-TFT Classifier |
| `05_Model_Comparison.ipynb` | Comparative analysis |
| `06_Live_Analyser.ipynb` | Live inference system |

---

## Requirements ⚙️

> **Python 3.12.5** is required to run this project.
