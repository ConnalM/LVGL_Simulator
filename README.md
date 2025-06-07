# LVGL Simulator with SDL2

A PlatformIO-based LVGL simulator using SDL2 for Windows. This project provides a quick way to develop and test LVGL applications on a PC before deploying to embedded hardware.

## Features

- LVGL 8.4.0 with SDL2 backend
- PlatformIO project structure
- Windows-compatible build system
- Example UI with button and label

## Prerequisites

- [PlatformIO Core](https://platformio.org/install/cli)
- SDL2 development libraries (included in the project)

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/YOUR_USERNAME/LVGL_Simulator.git
   cd LVGL_Simulator
   ```

2. Build the project:
   ```bash
   pio run
   ```

3. Run the simulator:
   ```bash
   .pio\build\native\program.exe
   ```

## Project Structure

- `src/` - Source files
  - `main.cpp` - Main application code
- `include/` - Header files
- `lib/` - External libraries
- `platformio.ini` - PlatformIO configuration

## License

MIT License - See LICENSE file for details
