# OmniTrack - Smart Object Journey Recorder (IoT Project)
**Course:** IoT & Cybersecurity - L4 S1 2026
**Team Leader:** Abed Abdullah Sahari

## 📌 Project Overview
OmniTrack is a smart, event-driven IoT "black box" designed to secure sensitive cargo, luggage, and medical shipments. It uses an **ESP32** microcontroller and an **MPU6050** IMU sensor to detect physical anomalies.

## ⚙️ Core Features
- **Shock Detection:** Calculates total acceleration ($\sqrt{x^2 + y^2 + z^2}$) to detect impacts > 15.0 G.
- **Orientation Monitoring:** Detects if the cargo is inverted (Z-axis < -5.0).
- **Unauthorized Access:** Uses edge-detection on a magnetic switch to record tampering.
- **Cloud Integration:** Uses **MQTT Protocol** via PubSubClient to transmit lightweight, reliable payloads to **ThingSpeak**.

## 🛠️ Hardware & Wiring (I2C Standard)
- MPU6050 SDA -> GPIO 21
- MPU6050 SCL -> GPIO 22
- Door Sensor -> GPIO 4 (INPUT_PULLUP)
