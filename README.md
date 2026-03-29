# STM32F429I Smart Temperature Controller 🌡️

**Course:** Embedded Systems (2nd Year Electronics-ICT)  
**Platform:** STM32F429I-Discovery Board (ARM Cortex-M4)  
**RTOS:** CMSIS-RTOS2 (Keil RTX5)  

## 📖 About The Project
This project is a complete RTOS-driven embedded smart temperature controller. It reads environmental data from an external I2C sensor, processes it using a multi-threaded architecture, and displays the data on a custom TFT graphical user interface built with the **emWin** library. 

It also features USB FAT32 data logging, automatic/manual fan control (simulated via onboard LEDs), and direct hardware-register manipulation to read the STM32's internal core temperature diode.

## ⚙️ Features
* **I2C Sensor Communication:** Reads ambient temperature from a TC74 sensor.
* **Bare-Metal ADC Reading:** Bypasses standard HAL libraries to read the internal STM32 processor core temperature (Channel 18) via direct register manipulation.
* **Graphical Interface:** Custom UI with vertical "mercury column" progress bars, dynamic text, and state-toggling buttons.
* **RTOS Architecture:** Thread-safe hardware access preventing race conditions between the GUI updates and background system tasks.
* **USB Data Logging:** Utilizes the Keil File System (`rl_fs`) to mount a FAT32 USB stick and log active temperatures to `templog.txt` every 5 seconds.
* **Smart Fan Control:** Auto-mode with a UI-adjustable threshold slider, and a Manual override mode.

---

## 🔌 Hardware Setup & Wiring (TC74 Sensor)
To run this project, you need to connect the **TC74 I2C Temperature Sensor** to the STM32F429I-Discovery board. 

The TC74 is a 5-pin component. Hold it so the flat side with the text is facing you, and the pins are pointing down. Count the pins from left to right (1 to 5):

| TC74 Pin | Function | STM32F429I Pin | Notes |
| :---: | :--- | :--- | :--- |
| **1** | NC (No Connect) | *Leave Empty* | Do not connect this pin. |
| **2** | SDA (Data) | **PC9** | I2C3 Data line. |
| **3** | GND (Ground) | **GND** | Connect to any Ground pin on the board. |
| **4** | SCLK (Clock) | **PA8** | I2C3 Clock line. |
| **5** | VDD (Power) | **3V3** | Powers the sensor (Do NOT use 5V to keep logic levels safe for the STM32). |

**Additional Hardware:**
* **Micro-USB OTG Cable:** Plugged into the bottom `USB USER` port of the Discovery board.
* **USB Flash Drive:** Formatted to **FAT32** for the data logger.

---

## 💻 Software Architecture Highlights
As a student project, several specific engineering choices were made to solve embedded roadblocks:
1. **The "Haunted" LED Fix:** Standard Keil projects often have a hidden background "Blinky" thread using `vioLED0`. To prevent race conditions and a flickering LED, the fan-control logic in this project is strictly routed to `vioLED1`.
2. **Safe USB Writing:** The GUI thread executes 10 times a second. To prevent memory corruption on the USB drive, an RTOS timer acts as a throttle, only triggering the `fopen()` and `fprintf()` commands once every 50 loops (5 seconds).
3. **State-Machine UI:** The right side of the screen features a button that toggles the data source of the second progress bar between the actual Internal Processor Diode and a Gyroscope placeholder, demonstrating clean variable passing between the GUI frontend and the backend C logic.

## 🚀 How to Run
1. Clone this repository.
2. Open the `.uvprojx` project file in **Keil uVision 5**.
3. Connect your STM32F429I-Discovery board via the top ST-LINK Mini-USB port.
4. Ensure your TC74 sensor is wired correctly according to the table above.
5. Plug your FAT32 USB stick into the bottom Micro-USB OTG port.
6. Click **Build (F7)** and then **Download (F8)** to flash the code to the board.
7. Press the black **Reset (B2)** button on the board to start the OS!

---
*Developed as part of the Electronics-ICT curriculum.*
