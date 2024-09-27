# Asynchronous S/PDIF Mixer
"Asynchronous S/PDIF Mixer" is a C++ solution designed to synchronize and mix two asynchronous S/PDIF streams into a single S/PDIF output stream at 48kHz. It compensates for drift and uses interpolation for smooth audio transitions, developed for the STM32H723 board and built using CubeIDE for real-time audio processing.
## Author
This project is developed by **DAD Design**.
## License
This project is licensed under the [MIT License](LICENSE).

## Features
- **Synchronization of Asynchronous S/PDIF Streams:**  
  The project synchronizes two input audio streams, each potentially running at different sample rates (96kHz, 48kHz, 44.1kHz, 32kHz), into a unified output stream at 48kHz.
  
- **Clock Drift Compensation:**  
  Drift between the input streams is handled through periodic drift factor recalculation, ensuring the output remains smooth and synchronized.

## Hardware Platform

The code is designed to run on a **WeAct Studio STM32H723 prototype board**, a powerful ARM Cortex-M7-based microcontroller development board. The STM32H723 provides high-performance processing capabilities, which are well-suited for real-time audio processing tasks like stream synchronization and mixing.

## Development Environment
The project is developed and compiled using the **CubeIDE** environment provided by STMicroelectronics. CubeIDE is a fully integrated development environment (IDE) that supports STM32 microcontrollers and provides tools for debugging, flashing, and developing embedded systems.
