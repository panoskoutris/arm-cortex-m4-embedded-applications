# ARM Cortex-M4 Embedded Applications

Real-time embedded systems projects on the STM32F411RE Nucleo board (ARM Cortex-M4), developed using C and ARM Assembly. These applications demonstrate practical microcontroller programming, low-level peripheral control, and real-time responsiveness through UART, GPIO, timers, interrupts, and sensors such as DHT11.

Developed as part of the course Microprocessors and Peripherals at the Aristotle University of Thessaloniki (AUTH), School of Electrical & Computer Engineering.

---

## 📦 Contents

This repository contains three standalone projects, each designed to highlight key embedded systems concepts:

### 🔢 1. Hash & Fibonacci Calculator (Assembly-C Hybrid)

- Parses alphanumeric strings received via UART
- Computes a hash based on character types:
  - Uppercase: `ASCII × 2`
  - Lowercase: `(ASCII - 'a')²`
  - Digits: mapped via lookup table
- Reduces hash using digit-sum + `mod 7`
- Calculates Fibonacci of final value in ARM Assembly
- Final result is printed via UART

📁 Folder: `lab1-hash-fibonacci/`

---

### 💡 2. LED Control via Digit Analysis & Interrupts

- User inputs integer via UART
- Timer interrupt triggers digit-by-digit analysis every 0.5 sec
- Even digit → blink LED (200ms)
- Odd digit → toggle LED state
- Button interrupt can lock/unlock LED activity
- New number input restarts the process
- Input ending in `-` triggers continuous loop mode

📁 Folder: `lab2-digit-led-interrupt/`

---

### 🌡️ 3. Environmental Monitoring System (DHT11 + Touch + Panic Reset)

- UART menu to configure sampling frequency and display mode
- Touch sensor toggles between Normal and Alert modes
- Alert mode:
  - If Temp > 25°C or Humidity > 60% → LED blinks every 1 sec
  - Resets to normal after 5 stable readings
- Dynamic frequency based on last AEM digits
- UART `status` command prints current state and readings
- Software panic reset if:
  - Temp > 35°C or Hum > 80% for 3 consecutive reads

📁 Folder: `lab3-environment-monitor/`

---

## 🛠️ Development Environment

- **MCU**: STM32F411RE (ARM Cortex-M4 @ 100MHz)
- **Board**: Nucleo-F411RE
- **IDE**: Keil uVision 5
- **Toolchain**: Arm Keil MDK + CMSIS
- **Languages**: C, ARM Assembly
- **Interfaces**: UART, GPIO, Timers, Interrupts

---

## 📁 Repository Structure

```
arm-cortex-m4-embedded-applications/
│
├── lab1-hash-fibonacci/
│ ├── src/
│ └── report-lab1.pdf
│
├── lab2-digit-led-interrupt/
│ ├── src/
│ └── report-lab2.pdf
│
├── lab3-environment-monitor/
│ ├── src/
│ └── report-lab3.pdf
│
└── README.md
```

## ✍️ Author

**Panagiotis Koutris**  
📧 pkoutris@ece.auth.gr  
🎓 Undergraduate Student, AUTh – School of Electrical & Computer Engineering

---

## 📝 License

This project is provided for **academic and educational purposes** only.
