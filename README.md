# 🏠 Smart House Project (TM4C123G + ESP32)

> [!CAUTION]
> ### ⚠️ PROJECT STATUS: ABANDONED
> This project is no longer maintained and is kept for archival and educational purposes. It was originally developed as part of a university course. This project was not a 100% completed (functional) so this `readme.md` file is describing the vision so that anyone can continue working on it.

This repository contains the full source code and interface files for our Smart House project, developed using the **TM4C123GXL LaunchPad** and **ESP32** microcontroller as part of the *Intro to Microprocessors* (ECE4206) course at AASTMT.

> 📄 **Note:** The full system design and theoretical explanations are available in the project report (see `MicroProcessor Project Report Pre-Final.pdf`). This GitHub repository focuses on the firmware and software implementation.

---

## 📘 Project Overview

This project demonstrates a scalable smart home system that integrates real-time sensing, wireless monitoring, and automation. The system uses a **TM4C123G** for low-level sensor interfacing and control, and an **ESP32** for Wi-Fi connectivity and a web-based dashboard.

### 🏗️ System Architecture & Logic

The project is conceptually divided into specific zones, each handling a distinct subsystem:

| Zone | Functionality | Sensors/Actuators |
| :--- | :--- | :--- |
| **Room 1** | **Auto-Lighting** | PIR Motion Sensor → LED ON/OFF |
| **Room 2** | **Smart Dimming** | LDR Sensor → LED Brightness (PWM) |
| **Room 3** | **Safety Monitoring** | Flame Sensor → LED ON/OFF |
| **Room 4** | **Central Control** | RGB LED + Buzzer (Alerts) + Tiva C & ESP32 |
| **Entrance** | **Intrusion Detection** | Laser Module + LDR (Tripwire) |
| **Garden** | **Auto-Irrigation** | Soil Moisture Sensor → Water Pump (via 2N2222) |
| **Garage** | **Smart Parking** | Ultrasonic Sensor → Distance-based Alerts |

### 🚨 Alert System Logic

The system uses a centralized notification system (RGB LED + Buzzer) to communicate status:

| Condition | RGB LED Color | Buzzer Pattern |
| :--- | :--- | :--- |
| **High Temp** (>30°C) | 🟠 **Orange** | Constant Tone |
| **Fire Detected** | 🔴 **Crimson** | Fast Pulsing |
| **Intrusion** | 🟣 **Purple** | Fast Pulsing |
| **Car Parking** | 🟡 → 🔴 **Gradient** | Freq. increases with proximity |

---

## ⚠️ Known Limitations & Demo Status

While the project provides a comprehensive codebase, users should be aware of the following based on the final demo results:

- **LCD Display**: The physical 16x2 I2C LCD was deprecated in the final demo due to timing issues and hardware constraints. All data visualization is primarily handled via the **Web Dashboard**.
- **Theoretical vs. Practical**: The demo focuses on individual room functions. In a real-world scenario, sensors would be redundant across all rooms, which was not implemented due to budget and time limitations.
- **Hardware Stability**: Serial communication (UART) between the two microcontrollers is sensitive to wiring quality and requires the **Logic Level Shifter** for stable data exchange.

---

## ⚙️ Hardware & Technologies

| Component | Role | Note |
| :--- | :--- | :--- |
| **TM4C123GXL** | Main Microcontroller | Handles all sensors/actuators logic. |
| **ESP32-S** | Wi-Fi Module | Hosts Web Dashboard & AP. |
| **Logic Level Shifter** | **CRITICAL** | Converts some sensors' 5V UART signals to ESP32's and Tiva's 3.3V. |
| **2N2222 Transistor** | Power Driver | Drives the water pump (high current). |
| **Sensors** | PIR, LM35, LDR, Flame, Soil Moisture, Ultrasonic | Standard analog/digital modules. |
| **Actuators** | 5V DC Motor (Pump), Buzzer, RGB LED, Laser | |

---

## 📁 Repository Structure

```plaintext
/Smart-House-TM4C-ESP32/
├── tiva c codes/                 # Source code for TM4C123GXL
│   ├── launchpadcode_3/          # Main Tiva C firmware (Sensors & Logic)
│   └── libraries/                # Custom libraries (e.g., PinMap)
│
├── esp32 codes/                  # Source code for ESP32
│   ├── esp32webinterface_7/      # Latest Web Dashboard firmware
│
├── Pics/                         # Photos of hardware, diagrams, and progress
├── PIOcoding/                    # PlatformIO project files
├── Report PDFs (Drafts)/         # Project reports and documentation
└── TM4C123GXL - Datasheets.../   # Datasheets for the microcontroller
```

---

## 🚀 Getting Started

### 1. Hardware Setup
- Connect sensors to the **TM4C123G** as defined in `tiva c codes/launchpadcode_3/launchpadcode_3.ino`.
- Power both boards.

### 2. Flashing the Code
- **TM4C123G**: Open `tiva c codes/launchpadcode_3/launchpadcode_3.ino` in Energia (with TM4C support) and upload.
- **ESP32**: Open `esp32 codes/esp32webinterface_7/src/main.cpp` in VS Code with PlatformIO Extension and upload.

### 3. Accessing the Dashboard
1.  Connect your phone/laptop to the Wi-Fi network:
    - **SSID**: `SmartHouse_AP`
    - **Password**: `12345678`
2.  Open a web browser and go to: `http://192.168.4.1`
3.  You should see the live dashboard! (didn't actually implement the logic of communicating the data between the 2 boards)

---

## 👥 Contributors

This project was built by a dedicated team for the **Intro to Microprocessors** course.

- **Ahmad Adham Badawy** - [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/ahmad-adham-badawy/)
- **Ali Abd El Nasser Ali** - [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/ali-abd-el-nasser-ali-970236363/)
- **Abdallah Fahmy Rabea**
- **Abdelrahman Mostafa**
- **Eslam Mohammed**
- **Mohammed Ehab Badr**

**Supervision:**
- **Instructor:** Dr. Ahmad Sayed
- **Teaching Assistant:** Eng. Fatma Sharawy [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/fatma-sharaawy-279ba3164/)

### 💡 Inspiration & Credits
This project draws conceptual inspiration from a previous multidisciplinary smart house system developed for the **AASTMT Smart House Competition** by Youssef Wagdi, Farida Hisham, Shady Magdy, and Basem Naeem.

---

### 📄 License
This project is open-source and available under the **MIT License**.
