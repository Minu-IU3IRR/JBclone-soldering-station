# DIY Multi-Channel JBC-Compatible Soldering Station

A high-performance open hardware soldering station designed to support **JBC and other thermocouple-based soldering tips**.  
This project aims to deliver professional-grade thermal control and user experience at a fraction of the cost of a commercial JBC system.

---

## Overview

The system provides **multi-channel support**, each with independent temperature feedback loops based on PID control.  
It includes modular PCB designs, 3D-printable case that fits in a standard 200x200mm 3d printer, STM32F103 microcontroller, Nextion HMI interface

**Key features:**
- Multi-channel indipendent temperature control
- Compatible with any arbitrary tip, JBC or otherwise, as long as it uses a thermocouple
- PID temperature control with zero cross switching
- external EEprom stored calcibration and tuning  
- Nextion-based HMI for user interaction
- Fully open-source hardware and firmware

---
## Firmware Architecture

The firmware is structured into modules:

| Module | Description |
|---------|-------------|
| **Heater/** | PID controller, temperature processing, and power control routines |
| **EEprom/** | Persistent storage for calibration and configuration |
| **Hardware_definition/** | Board pin mapping, timing constants, and I/O configuration |
| **display/** | Nextion display communication handler |
| **hartbeat/** | System heartbeat and status indicator |
| **Parser/** | Command parser for serial/HMI communication |

**PID Control:**  
The controller continuously adjusts heater drive based on thermocouple feedback.  
Parameters `Kp`, `Ki`, and `Kd` can be tuned at runtime through serial commands or tuning software.

**EEPROM Configuration:**  
All configuration data (tip profiles, PID tuning, calibration) is saved in EEPROM.  
On startup, the station loads the saved configuration automatically.

---

## Hardware Assembly

1. **PCBs:**  
   Fabricate the `mainboard` and `daughterboard` Gerbers (found under `/production/`).
2. **Mechanical design:**  
   View or download the CAD model on Onshape:  
   👉 [Onshape Assembly Model](https://cad.onshape.com/documents/a679f571d3dbe280f604253c/w/8a053e0b812f1b7821749418/e/78f31934af3d4bb4e1cbee4a)
3. **Transformer:**  
   Follow winding and connection details from the `transformer/` folder or pick a closely matched one (MUST HAVE INDIPENDENT WINDINGS FOR EACH OUTPUT)

---

## 🖥️ HMI Setup

1. Open `NX3224T024_hmi.HMI` in **Nextion Editor**.
2. Connect the display to the mainboard’s UART (`Serial1`, 115200 bps).
3. Upload the `.tft` file to the screen via SD card or serial upload or upload via SD-card, check Nextion documentation
5. The interface provides:
   - Channel selection  
   - Temperature setpoint entry  
   - Live tip temperature display

---

## Software Build

1. **Install [PlatformIO](https://platformio.org/).**
2. Open `Software/JBclone Firmware/` in VS Code.
3. Connect your STM32 board using an ST-link dongle.
4. Build and upload:
   ```bash
   pio run --target upload
   ```
5. The firmware will automatically initialize EEPROM and start monitoring the heater.

---

## Calibration & Tuning

- Refer to `cal and tuning table.ods` for measured offsets and recommended PID coefficients.  
- Store calibration through serial commands or via tuning software.  
- EEPROM ensures retention between power cycles.

---

## License

**Hardware (PCBs, transformer, mechanical design):**  
Released under the [CERN-OHL-P v2 License](https://ohwr.org/project/cernohl/wikis/Documents/CERN-OHL-version-2).  
You are free to use, modify, and distribute, provided attribution to the original project is maintained.  

**Firmware and software:**  
Released under the [MIT License](https://opensource.org/licenses/MIT).  

All content is provided **as-is** without any warranty or liability.  
Please include a reference to this repository when sharing or modifying the design.

---

## Credits

Project designed and developed by Manuel Minutello.  
Hardware and firmware are open for modification and improvement by the community.

---

