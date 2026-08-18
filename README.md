# Open-Source ESP32-C3 Smart Switch

A compact **Wi-Fi controlled 230 VAC smart switch** based on the **ESP32-C3**, designed as an open-source platform for learning, prototyping, and home automation.

The project combines wireless connectivity, an AC power supply, and a solid-state TRIAC driver into a single PCB.

---

## Features

- ESP32-C3 with **Wi-Fi & Bluetooth LE**
- **230 VAC / 50 Hz** input
- **LNK306** offline power supply
- **5 V → 3.3 V** regulation using AMS1117-3.3
- **Optotriac + BT136** AC load driver
- Ceramic **2.4 GHz antenna**
- RF impedance matching network
- Status LED
- UART debugging interface
- Custom compact PCB

---

## Hardware

### Schematic

![Smart Switch Schematic](https://github.com/moovma/smart-switch/blob/main/Capture%20d'%C3%A9cran%202026-08-18%20190547.png?raw=true)
The schematic is divided into several main blocks:

### ESP32-C3 Wireless Control Unit

The ESP32-C3 is the main controller of the system. It handles the wireless communication and controls the connected AC load.

### 2.4 GHz RF Interface

A ceramic antenna and matching network provide the wireless interface for Wi-Fi and Bluetooth communication.

### 230 VAC Power Supply

The 230 VAC input is rectified and converted to a regulated **5 V supply using the LNK306**. The 5 V rail is then regulated to **3.3 V** for the ESP32-C3 and other low-voltage circuitry.

### 230 VAC TRIAC Load Driver

The ESP32-C3 drives an optotriac, which controls the **BT136 TRIAC** and switches the connected AC load.

### Status & Debug Interface

A status LED provides visual feedback, while UART is available for firmware development and debugging.

---

## PCB Layout

![Smart Switch PCB Layout](https://github.com/moovma/smart-switch/blob/main/Capture%20d'%C3%A9cran%202026-08-16%20052827.png?raw=true)

The PCB separates the **mains section** from the low-voltage and RF sections.

Particular attention was given to:

- RF routing
- Antenna placement
- Power-supply layout
- Mains creepage and clearance
- Ground-plane design
- Thermal management
- Compact component placement

---

## 3D Model

![Smart Switch 3D Model](https://github.com/moovma/smart-switch/blob/main/Capture%20d'%C3%A9cran%202026-08-16%20052753.png?raw=true)

The final PCB integrates the wireless controller, power supply, and AC switching stage into a single compact board.

---

## How It Works

```text
                 ┌──────────────────┐
                 │     ESP32-C3     │
                 │   Wi-Fi / BLE    │
                 └────────┬─────────┘
                          │
                       GPIO
                          │
                          ▼
                    ┌───────────┐
                    │ Optotriac │
                    └─────┬─────┘
                          │
                          ▼
                     ┌─────────┐
230 VAC ─────────────►│  BT136  │──────► AC LOAD
                     └─────────┘

230 VAC
   │
   ▼
 LNK306
   │
   ▼
  5 V
   │
   ▼
AMS1117
   │
   ▼
 3.3 V
   │
   ▼
ESP32-C3
