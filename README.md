# DIY Multi-Channel JBC-Compatible Soldering Station

A high-performance open hardware soldering station designed to support **JBC and other thermocouple-based soldering tips**.  
This project aims to deliver professional-grade thermal control and user experience at a fraction of the cost of a commercial JBC system.

---

## 🔍 Overview

The system provides **multi-channel support**, each with independent temperature feedback loops based on PID control.  
It includes modular PCB designs, a 3D-printable mechanical assembly, firmware for an STM32 microcontroller, a Nextion HMI interface, and a transformer subsystem.

**Key features:**
- Multi-channel temperature control with isolated feedback inputs  
- Compatible with JBC T245/T210 and other thermocouple tips  
- PID-based closed-loop control with EEPROM-stored calibration  
- Nextion-based HMI for user interaction  
- Transformer design for efficient isolated power  
- Fully open-source hardware and firmware

---

## 🧩 Repository Structure

```
V0.3/
├── mainboard/                 # Main control PCB (KiCad)
│   ├── schematic.kicad_sch
│   ├── schematic.kicad_pcb
│   ├── production/            # Gerbers and BOM
│   └── modelli 3d/            # 3D connector models
│
├── daughterboard/             # Auxiliary interface or channel expander board
│   ├── schematic.kicad_sch
│   └── production/
│
├── transformer/               # Transformer winding and design data
│
├── HMI/                       # Nextion display project
│   ├── NX3224T024_hmi.HMI
│   ├── Font_Label_16.zi
│   └── Font_lebal.zi
│
├── Software/                  # Firmware and utilities
│   ├── JBclone Firmware/      # PlatformIO project for STM32
│   │   ├── lib/               # Firmware modules
│   │   └── src/               # Core logic
│   └── jbclone Tuner/         # PC or auxiliary tuning tool
│
└── cal and tuning table.ods   # Calibration reference data
```

---

## ⚙️ Firmware Architecture

The firmware (under `Software/JBclone Firmware/`) is structured into modules:

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
Parameters `Kp`, `Ki`, and `Kd` can be tuned at runtime through serial commands or via the HMI interface.

**EEPROM Configuration:**  
All configuration data (tip profiles, PID tuning, calibration) is saved in EEPROM.  
On startup, the station loads the saved configuration automatically.

---

## 🧠 Configuration and Customization

To customize or adapt the station for a different tip:

1. **Edit configuration files** in `lib/configs/` or adjust runtime parameters via the serial terminal:
   ```bash
   > kp 1.2
   > ki 0.05
   > kd 0.15
   ```
   You can read parameters using:
   ```bash
   > kp ?
   ```
2. **Recalibrate thermocouples** using the tuning table (`cal and tuning table.ods`).
3. **Update HMI display** using `NX3224T024_hmi.HMI` in the `HMI/` folder (open with Nextion Editor).

---

## 🧰 Hardware Assembly

1. **PCBs:**  
   Fabricate the `mainboard` and `daughterboard` Gerbers (found under `/production/`).
2. **Mechanical design:**  
   View or download the CAD model on Onshape:  
   👉 [Onshape Assembly Model](https://cad.onshape.com/documents/a679f571d3dbe280f604253c/w/8a053e0b812f1b7821749418/e/78f31934af3d4bb4e1cbee4a)
3. **Transformer:**  
   Follow winding and connection details from the `transformer/` folder.

---

## 🖥️ HMI Setup

1. Open `NX3224T024_hmi.HMI` in **Nextion Editor**.
2. Connect the display to the mainboard’s UART (`Serial1`, 115200 bps).
3. Upload the `.tft` file to the screen via SD card or serial upload.
4. The interface provides:
   - Channel selection  
   - Temperature setpoint entry  
   - Live tip temperature display  
   - System settings access  

---

## 🧩 Software Build

1. **Install [PlatformIO](https://platformio.org/).**
2. Open `Software/JBclone Firmware/` in VS Code.
3. Connect your STM32 board.
4. Build and upload:
   ```bash
   pio run --target upload
   ```
5. The firmware will automatically initialize EEPROM and start monitoring the heater.

---

## 📊 Calibration & Tuning

- Refer to `cal and tuning table.ods` for measured offsets and recommended PID coefficients.  
- Store calibration through serial commands or via HMI options.  
- EEPROM ensures retention between power cycles.

---

## ⚖️ License

**Hardware (PCBs, transformer, mechanical design):**  
Released under the [CERN-OHL-P v2 License](https://ohwr.org/project/cernohl/wikis/Documents/CERN-OHL-version-2).  
You are free to use, modify, and distribute, provided attribution to the original project is maintained.  

**Firmware and software:**  
Released under the [MIT License](https://opensource.org/licenses/MIT).  

All content is provided **as-is** without any warranty or liability.  
Please include a reference to this repository when sharing or modifying the design.

---

## 👤 Credits

Project designed and developed by **[Your Name]**.  
Hardware and firmware are open for modification and improvement by the community.

---

