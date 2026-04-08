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

## 2. Wiring

> **Note:** If you are planning on changing the wiring, be sure to reference the chip pins and update the `SpotifyDisplay.ino` file (specifically the `#define` pins).

### Testing and Preparation
* **Component Testing:** Begin by testing your components. Plug in your CYD and flash a test script to the serial monitor to verify the potentiometer and buttons. 
* **Verification:** Once everything has been verified, flip the CYD over.

<img src="images/cydBack.png" alt="CYD" width="50%">

### Soldering the Connections
* **First Connection:** Connect the JST connector that came with your kit to **CN1** and pin 6 (**GPIO 34**) as shown below. 
* **Reference:** It is helpful to have the chip's datasheet handy. You can find the pinout on page 8 of the [ESP32-WROOM-32 Datasheet](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf).

<img src="images/connection1.png" alt="First Connection" width="50%">

### Power and Ground
* Take two male-to-male jumper wires and connect:
    * **3.3V** to the **red** power rail.
    * **GND** to the **black** power rail.
* **Note:** Use red and black wires respectively to stay organized, but always double-check the connection points. (You may skip the breadboard and wire directly if preferred).

<img src="images/breadboard.png" alt="Power and Ground" width="50%">

### Component Wiring
Once power is established, connect the following:

* **Mode/Reset Button:** Connect to the yellow wire (**GPIO 22**) and Ground.
* **Play/Pause Button:** Connect to the blue wire (**GPIO 27**) and Ground.
* **Potentiometer:**
    * **Pin 1:** Connect to 3.3V.
    * **Pin 2 (Control Signal):** Connect to the wire soldered in the previous step.
    * **Pin 3:** Connect to Ground.

<img src="images/completedConnections.png" alt="EverythingWired" width="50%">

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
