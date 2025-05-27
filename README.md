# 🏠 Smart House Project (TM4C123G + ESP32)

This repository contains the full source code and interface files for our Smart House project, developed using the TM4C123GXL LaunchPad and ESP32 microcontroller as part of the *Intro to Microprocessors* course at AASTMT.

> 📄 The full system design, explanation, and documentation are available in the project report. This GitHub repository focuses exclusively on the codebase.

---

## 📘 Project Overview

The project demonstrates a scalable smart home system that automates and monitors both indoor and outdoor environments. It integrates real-time sensing, wireless data transmission, and visual/audio alerts using embedded systems.

### 👇 Key Functionalities
- **Garden Automation**: Watering system based on soil moisture; water tank level display.
- **Room Monitoring**:
  - Motion-based lighting (PIR sensor)
  - Light-based dimming (LDR + LED)
  - Temperature and flame detection (LM35 + Fire Sensor)
  - Centralized RGB LED + Buzzer alerts
- **Security**: Intrusion detection using a laser + LDR sensor
- **Parking System**: Ultrasonic sensor with proximity-based alerts
- **Web Dashboard**: Hosted on ESP32 for live monitoring over Wi-Fi

---

## ⚙️ Technologies Used

| Component | Role |
|----------|------|
| **TM4C123GXL (Tiva C LaunchPad)** | Sensor reading, local control, serial communication |
| **ESP32** | Wi-Fi access point + Web dashboard UI |
| **UART (Serial Communication)** | JSON-based data exchange between TM4C and ESP32 |
| **HTML + JavaScript** | Frontend interface hosted on ESP32 |

---
### 📁 Repository Structure

```plaintext
/SmartHouse/
├── tiva c codes/
│   ├── launchpadcode_1/
│   ├── launchpadcode_2/
│   ├── ultrasonic/
│   ├── launchpadcode_3/
│   └── libraries/
│       └── TivaCPinMap/
│
├── esp32 codes/
│   ├── esp32webinterface_1/
│   ├── esp32webinterface_2/
│   ├── esp32webinterface_3/
│   ├── esp32webinterface_4/
│   ├── esp32webinterface_5/
│   └── esp32webinterface_6/
│
├── PIOcoding/
├── Report PDFs (Drafts)/
├── Report Refrences & Inspiration/
├── TM4C123GXL - Datasheets & Pinouts/
├── Report DOCXs & Info (Drafts)/
├── Canceled/
│
├── README.md
└── LICENSE
```
#### 📦 Folder Descriptions

- **`tiva c codes/`** – Code for TM4C123GXL microcontroller (Tiva C).
- **`esp32 codes/`** – ESP32 Arduino sketches handling web dashboard and comms.
- **`PIOcoding/`** – PlatformIO project files for uploading to ESP32.
- **`Report PDFs (Drafts)/`** – Project report PDF versions.
- **`Report DOCXs & Info (Drafts)/`** – Editable DOCX files and notes.
- **`Report Refrences & Inspiration/`** – Research sources, similar projects, external docs.
- **`TM4C123GXL - Datasheets & Pinouts/`** – Reference sheets and pinout guides.
- **`Canceled/`** – Deprecated or removed content.
- **`README.md`** – Main documentation file (this file).
- **`LICENSE`** – MIT License file.
---

## 📝 Report Summary

The full documentation explains:

- System infrastructure (Interior & Exterior electronics)
- Demo vs Theoretical design differences
- Sensors/actuators distribution per room
- Alert system logic with RGB + buzzer
- Communication architecture and power setup
- Future expansion plans and challenges

> ✅ Refer to the attached report for complete circuit diagrams, design rationale, planning logic, and hardware setup.

---

## 🚀 Getting Started

1. Flash the Tiva C code using Code Composer Studio or Keil.
2. Upload the ESP32 code using Arduino IDE.
3. Power both boards via USB.
4. Connect to the ESP32 Wi-Fi AP (default: `SmartHouse_AP`, password: `12345678`).
5. Open your browser and navigate to `192.168.4.1` to access the dashboard.

---

## 📬 Contact & Contributors

### 👥 Contributors

- **Ahmad Adham Badawy**  (Me :D)

  [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/ahmad-adham-badawy/)

- **Ali Abd El Nasser Ali**  

  [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/ali-abd-el-nasser-ali-970236363/)

- **Eslam Mohammed**
  
  [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/eslam-mohammed-abdelkader)
  [![GitHub](https://img.shields.io/badge/GitHub-Profile-black?logo=github)](https://github.com/Eslam-Mohammed198)

- **Abdelrahman Mostafa**

  [![GitHub](https://img.shields.io/badge/GitHub-Profile-black?logo=github)](https://github.com/Abd0M0stafa)
  
- **Abdallah Fahmy Rabea**

- **Mohamed Sayed**

- **Abdelrahman Hamdy**

- **Mohammed Ehab Badr**

- **Mostafa Roshdy**


#### Instructor: Dr. Ahmad Sayed  
#### Teaching Assistant: Eng. Fatma Sharawy  

  [![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin)](https://www.linkedin.com/in/fatma-sharawy-279ba3164/)

#### Course: ECE4206 – Intro to Microprocessors (AASTMT)

---

### 📌 Notes

- All hardware explanations and planning considerations are included in the attached report PDF.
- This repo is designed for educational purposes and can be extended into a production-grade system with cloud connectivity, mobile app integration, and advanced control logic.

---
### 📄 License

This project is licensed under the MIT License — you are free to use, modify, and distribute the code with proper credit. While this is an open-source academic project, we kindly ask that you acknowledge the original contributors if reusing or building upon this work.

