# State of Charge and Degradation Estimation of a Li-ion Battery Using Neural Networks

This project focuses on estimating the State of Charge (SoC) of Li-ion batteries using real-time measurements of voltage, current, and temperature. A neural network model (LSTM/RNN) is trained and deployed on an Arduino Nano 33 BLE to provide lightweight, on-device SoC estimation.

---

## 📌 Table of Contents
- [Overview](#overview)
- [Hardware Used](#hardware-used)
- [Software Used](#software-used)
- [Data Collection](#data-collection)
- [Model Development](#model-development)
- [Model Deployment](#model-deployment)
- [Prototype Demo](#prototype-demo)
- [Challenges & Learnings](#challenges--learnings)
  

---

## Overview

This project demonstrates the end-to-end pipeline of:
1. Collecting real-time battery parameters (V, I, T),
2. Computing ground truth SoC via Coulomb counting,
3. Training and validating LSTM/RNN models,
4. Compressing and quantizing the model,
5. Deploying on Arduino Nano 33 BLE for embedded inference.

Both Li-ion NMC 18650 (3Ah) and LiFePO₄ battery cells were tested.

---

## Hardware Used

- Arduino Nano 33 BLE
- 18650 Li-ion 3Ah cell
-  LiFePO₄ cell
- 10W Power Resistor
- DHT11 Temperature Sensor
-10kΩ Resistors
- Voltage Divider Circuit
- Shunt Resistor (for current sensing)

---

## Software Used

- Google Colab (Model training & quantization)
-  Arduino IDE (Data acquisition)
- TensorFlow & Keras
- NumPy, Pandas, Matplotlib
- Arduino CLI / xxd for conversion to .cc model
- .tflite and .cc format model deployment

---

##  Data Collection

Real-time data was collected using an Arduino-based setup:
-Voltage, current (via shunt), and temperature (via DHT11) were recorded.
- SoC was computed using **Coulomb Counting**.
-  ~160,000 samples collected during controlled discharge from 4.2V to ~3.2V.
- Dataset format: [Voltage, Current, Temperature, Ah, Wh, Ground Truth SoC]

---

##  Model Development

Two architectures were tested:

### LSTM Model

model = Sequential([
    LSTM(128, return_sequences=True, input_shape=(X_train.shape[1], X_train.shape[2]), recurrent_activation='sigmoid'),
    Dropout(0.2),
    LSTM(128, recurrent_activation='sigmoid'),
    Dropout(0.2),
    Dense(1, activation='linear')
])

-Trained with MSE loss, Adam optimizer.

-Achieved validation loss as low as 3.88e-4.

-Final test loss: 0.0025

## Model Deployment 
 -The trained model was first converted to .tflite format.
 
-Further compressed to float16 using TFLite Converter.

-Converted to .cc for Arduino deployment using:

xxd -i soc_model_fp16.tflite > soc_model.cc

## Prototype Demo

-The deployed Nano 33 BLE reads real-time voltage, current, and temperature.

-The model predicts SoC and outputs it over serial or to a connected display.

-Demo video included in this repo

## Challenges & Learnings
-Multiple hardware iterations were needed due to resistor overheating and circuit failures.

-Data acquisition required careful calibration and syncing.

-LSTM models are resource-intensive and difficult to deploy on embedded devices.

-Float16 quantization preserved accuracy while fitting into Nano BLE memory.

-Learned practical applications of embedded ML and battery SoC estimation.












