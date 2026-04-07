<div align="center">

# 🧵 FAG-TFT: Fault Anticipation Gateway using Temporal Fusion Transformer

### An AI-powered breakdown prevention system for industrial sewing machines

[![Python](https://img.shields.io/badge/Python-3.9%2B-blue?logo=python&logoColor=white)](https://www.python.org/)
[![PyTorch](https://img.shields.io/badge/PyTorch-2.x-EE4C2C?logo=pytorch&logoColor=white)](https://pytorch.org/)
[![Jupyter](https://img.shields.io/badge/Jupyter-Notebooks-F37626?logo=jupyter&logoColor=white)](https://jupyter.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

</div>

---

## 📌 Overview

Industrial sewing machines in garment manufacturing are subject to recurring mechanical breakdowns that cause costly production downtime. This project presents the **FAG-TFT** (Fault Anticipation Gateway — Temporal Fusion Transformer), a two-stage deep learning pipeline that:

1. **Detects** whether a breakdown is imminent (binary classification)
2. **Identifies** the specific fault type before it occurs (multi-class classification)

The system uses **67 sensor features** extracted from vibration (FFT bands) and electrical signals captured during normal sewing operations, enabling real-time predictive maintenance without interrupting production.

> 📄 This repository accompanies the IRP thesis: *"FAG-TFT: A Temporal Fusion Transformer Approach to Predictive Fault Anticipation in Industrial Sewing Machines"*

---

## 🏗️ System Architecture

The FAG-TFT pipeline operates as a two-stage decision system:

```
Raw Sensor Data (Vibration + Electrical)
        │
        ▼
┌─────────────────────────┐
│   Data Preprocessing    │  ← 00_Data_Preparation.ipynb
│  FFT + Feature Eng.     │
│  67-dimensional vector  │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│   Model A: FAG-TFT      │  ← 03_FAG_TFT_Forecaster.ipynb
│   Binary Forecaster     │
│   "Is a fault coming?"  │
│   Accuracy: 97.37%      │
└────────────┬────────────┘
             │  [Fault Detected]
             ▼
┌─────────────────────────┐
│   Model B: FAG-TFT      │  ← 04_FAG_TFT.ipynb
│   Type Classifier       │
│   "What type of fault?" │
│   Accuracy: 91.79%      │
└─────────────────────────┘
             │
             ▼
   ┌─────────────────┐
   │  Live Analyser  │  ← 06_Live_Analyser.ipynb
   │  Real-time UI   │
   └─────────────────┘
```

---

## 📁 Repository Structure

```
Preventing-Mechanism/
│
├── 📂 Datasets/                        # Raw and processed sensor data
│
├── 📂 Implementation/                  # All notebooks, models, and results
│   ├── 00_Data_Preparation.ipynb       # Data loading, FFT, feature engineering
│   ├── 01_Forecasting_LSTM_Baseline.ipynb   # LSTM baseline model
│   ├── 02_Forecasting_GRU_Baseline.ipynb    # GRU baseline model
│   ├── 03_FAG_TFT_Forecaster.ipynb     # Model A: Binary fault detection (TFT)
│   ├── 04_FAG_TFT.ipynb                # Model B: Fault type classification (TFT)
│   ├── 05_Model_Comparison.ipynb       # Cross-model evaluation & visualisation
│   ├── 06_Live_Analyser.ipynb          # Real-time inference pipeline
│   │
│   ├── best_tft_binary.pt              # Saved weights — Model A (Binary)
│   ├── best_tft_type.pt                # Saved weights — Model B (Type)
│   ├── best_forecast_lstm.pt           # Saved weights — LSTM baseline
│   ├── best_forecast_gru.pt            # Saved weights — GRU baseline
│   ├── best_fag_tft.pt                 # Saved weights — FAG-TFT combined
│   ├── encoder.pkl                     # Label encoder — binary model
│   ├── encoder_reason.pkl              # Label encoder — type classifier
│   │
│   └── 📊 Result Figures
│       ├── fag_tft_loss.png
│       ├── fag_tft_confusion.png
│       ├── fag_tft_group_importance.png
│       ├── forecast_comparison.png
│       └── forecast_comparison_forecasters_only.png
│
├── 📂 MBP_PCB/                         # PCB design files for sensor hardware
├── 📂 MBP_interface/                   # Monitoring interface source files
└── README.md
```

---

## 🔬 Dataset & Features

### Sensor Data Collection

Sensor data is collected from an industrial sewing machine using a custom PCB (see `MBP_PCB/`) that captures:

| Sensor Type | Features | Description |
|---|---|---|
| **Vibration** | 60 features | FFT frequency band magnitudes (Band 0 – Band 59) |
| **Electrical** | 6 features | Voltage and current measurements across machine phases |
| **Temporal** | 1 feature | Time elapsed between consecutive pedal press events |
| **Total** | **67 features** | Final input vector per sewing cycle |

### Fault Classes

The system is trained to detect and classify three breakdown types:

| Label | Fault Type | Description |
|---|---|---|
| `HTT` | **High Thread Tension** | Thread tension exceeds normal operating range |
| `HFP` | **High Foot Pressure** | Presser foot pressure abnormally elevated |
| `RNT` | **Running with No Thread** | Machine operating without thread loaded |
| `Normal` | **Normal Operation** | No fault present |

---

## 🤖 Models

### Model A — FAG-TFT Binary Forecaster (`03_FAG_TFT_Forecaster.ipynb`)

Predicts **whether a fault will occur** in the near future (binary: fault / no-fault).

| Hyperparameter | Value |
|---|---|
| Architecture | Temporal Fusion Transformer (TFT) |
| Hidden size | 64 |
| Attention heads | 4 |
| LSTM layers | 2 |
| Dropout | 0.3 |
| **Accuracy** | **97.37%** |
| **F1 Score** | **0.9739** |

---

### Model B — FAG-TFT Type Classifier (`04_FAG_TFT.ipynb`)

Identifies **which fault type** is present when Model A triggers an alert.

| Hyperparameter | Value |
|---|---|
| Architecture | Temporal Fusion Transformer (TFT) |
| Hidden size | 16 |
| Attention heads | 2 |
| LSTM layers | 1 |
| Dropout | 0.4 |
| **Accuracy** | **91.79%** |
| **F1 Score** | **0.9190** |

#### Per-Class Performance

| Fault Type | Precision | Recall |
|---|---|---|
| High Thread Tension | 0.83 | 0.96 |
| High Foot Pressure | 0.97 | 0.89 |
| Running with No Thread | 0.98 | 0.90 |

---

### Baseline Comparison

| Model | Task | Accuracy | F1 Score |
|---|---|---|---|
| LSTM Baseline | Binary Forecasting | — | — |
| GRU Baseline | Binary Forecasting | — | — |
| **FAG-TFT Model A (Ours)** | **Binary Detection** | **97.37%** | **0.9739** |
| **FAG-TFT Model B (Ours)** | **Type Classification** | **91.79%** | **0.9190** |

> Full comparison with charts available in `05_Model_Comparison.ipynb`

---

## 📊 Results

### Training Loss Curve
![FAG-TFT Training Loss](Implementation/fag_tft_loss.png)

### Confusion Matrix — Type Classifier
![FAG-TFT Confusion Matrix](Implementation/fag_tft_confusion.png)

### Feature Group Importance
![Feature Group Importance](Implementation/fag_tft_group_importance.png)

### Model Forecast Comparison
![Forecast Comparison](Implementation/forecast_comparison.png)

---

## 🚀 Getting Started

### Prerequisites

```bash
pip install torch torchvision
pip install numpy pandas matplotlib seaborn scikit-learn
pip install jupyter
```

### Running the Notebooks

Run notebooks in order for the full pipeline:

```bash
cd Implementation/

# Step 1: Prepare data
jupyter notebook 00_Data_Preparation.ipynb

# Step 2 (optional): Run baseline models
jupyter notebook 01_Forecasting_LSTM_Baseline.ipynb
jupyter notebook 02_Forecasting_GRU_Baseline.ipynb

# Step 3: Train Model A — Binary Forecaster
jupyter notebook 03_FAG_TFT_Forecaster.ipynb

# Step 4: Train Model B — Type Classifier
jupyter notebook 04_FAG_TFT.ipynb

# Step 5: Compare all models
jupyter notebook 05_Model_Comparison.ipynb

# Step 6: Run live inference
jupyter notebook 06_Live_Analyser.ipynb
```

### Using Pre-trained Models

Pre-trained weights are included and ready to use:

```python
import torch

# Load Model A — Binary Forecaster
model_a = torch.load('Implementation/best_tft_binary.pt')

# Load Model B — Type Classifier
model_b = torch.load('Implementation/best_tft_type.pt')
```

---

## 🔧 Hardware

The `MBP_PCB/` folder contains PCB design files for the **Machine Breakdown Prevention (MBP)** sensor board — a custom circuit that interfaces with industrial sewing machines to capture real-time vibration and electrical signals.

The `MBP_interface/` folder contains the monitoring interface software for real-time visualisation of machine state on the factory floor.

---

## 🖥️ Live Analyser

`06_Live_Analyser.ipynb` implements a **real-time inference pipeline** that:
- Accepts live sensor readings as input
- Runs the two-stage FAG-TFT classification chain
- Outputs the current machine state: `Normal` or `Fault Imminent`
- When a fault is detected, outputs the predicted fault type (`HTT` / `HFP` / `RNT`)
- Designed for deployment on factory floor monitoring stations

---

## 🔮 Future Work

- [ ] Unsupervised anomaly detection module for novel/unknown fault types (CNN-LSTM Autoencoder)
- [ ] Edge deployment on embedded hardware (Raspberry Pi / Jetson Nano)
- [ ] Expansion to additional sewing machine models and fault categories
- [ ] Real-time dashboard with alert notifications

---

## 📜 Citation

If you use this work, please cite:

```bibtex
@thesis{dias2026fagtft,
  title  = {FAG-TFT: A Temporal Fusion Transformer Approach to Predictive Fault Anticipation in Industrial Sewing Machines},
  author = {Dias, Deelaka},
  year   = {2026},
  type   = {IRP Thesis}
}
```

---

## 📬 Contact

**Deelaka Dias** — [@DeelakaDias](https://github.com/DeelakaDias)

--
