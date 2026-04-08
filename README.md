# ECE Embedded Devices Thread Challenge
### by Max Zhu

---

## 📌 Introduction

This project is part of the **ECE Embedded Devices Thread Challenge**, focusing on hands-on embedded systems development using microcontrollers, sensors, and real-time interaction.

The goal of this project was to design, build, and program a fun embedded system that integrates hardware and software to perform meaningful tasks. The final project was a fun ESP-32 based desk toy that displays the live feedback from Spotify, allows Spotify control, and current weather and time. This project offers a blend of assembling physical compoenents and calling pre-existing open-source APIs.

---

## ⚙️ Setting Up the Project
22 27 34

### 🔧 Physical Build

1. **Components Needed**
   - CYD has built in(ESP32-2432S028)
   - Buttons(any work)
   - Potentiometer(10kohm)
   - Jumper Wires
   - Soldering Iron
   - Solder
   - Eye Protection
   - Breadboard(optional can hand solder everything)

   <img src="images/components.png" alt="Layout of Components" width="50%">

2. **Wiring**
If you are planning on changing the wiring, be sure to reference the chip pins and also update the SpotifyDisplay.ino(change the DEFINE pins)

   - Begin with testing your components. Plugging your CYD in and flashing something to the serial monitor, testing potentiometer, and buttons. Once everything has been verified, flip the CYD over.
   <img src="images/CYDBack.png" alt="Layout of Components" width="50%">
   - Once everything has been verified, we can start soldering. The first connection we are going to make is to pin 6 or GPIO 34 and connecting the JST connector that came with our kit to CN1 as show. It's always helpful to have a photo of the chip's datasheet on hand like the one below. Our pinout is on page 8.
   https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf 
   <img src="images/connection1.png" alt="Layout of Components" width="50%">
   - 

3. **Notes**
   - Double-check pin mappings before powering
   - Avoid using restricted GPIO pins on ESP32

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
