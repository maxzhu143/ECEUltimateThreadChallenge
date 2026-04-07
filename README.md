# ECE Embedded Devices Thread Challenge
### by Max Zhu

---

## 📌 Introduction

This project is part of the **ECE Embedded Devices Thread Challenge**, focusing on hands-on embedded systems development using microcontrollers, sensors, and real-time interaction.

The goal of this project was to design, build, and program a fun embedded system that integrates hardware and software to perform meaningful tasks. The final project was a fun ESP-32 based desk toy that displays the live feedback from Spotify, allows Spotify control, and current weather and time. This project offers a blend of assembling physical compoenents and calling pre-existing open-source APIs.

---

## ⚙️ Setting Up the Project

### 🔧 Physical Build

1. **Components Needed**
   - CYD has built in(ESP32-2432S028)
   - Buttons(any work)
   - Potentiometer(10kohm)
   - Jumper Wires
   - Breadboard(optional can hand solder everything)

   ![Layout of Components](images/components.jpeg)

2. **Wiring**
   - Connect power (3.3V / GND) correctly
   - Connect input devices (buttons, potentiometer) to GPIO pins
   - Connect sensors via I2C or SPI as required
   - Ensure common ground across all components

3. **Notes**
   - Double-check pin mappings before powering
   - Avoid using restricted GPIO pins on ESP32
   - Use pull-up or pull-down resistors where necessary

---

### 💻 Software Setup

1. **Install Required Tools**
   - Arduino IDE or PlatformIO
   - ESP32 board support package

2. **Configure Environment**
   - Select correct board (e.g., ESP32 Dev Module)
   - Set appropriate COM/serial port
   - Install required libraries (e.g., WiFi, sensor libraries)

3. **Upload Code**
   - Connect ESP32 via USB
   - Compile and upload firmware
   - Open Serial Monitor for debugging (baud rate: 115200)

4. **Optional Setup**
   - Configure WiFi credentials if needed
   - Adjust code for your specific hardware configuration

---

## 🚀 Usage

- Power the device via USB or external supply
- Interact using buttons, sensors, or inputs
- Monitor output via display or serial monitor

---

## 📁 Project Structure
