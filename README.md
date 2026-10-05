# Gameboy Emulator

A Gameboy emulator for the Teensy 4.1 microcontroller and PC (Windows and Linux).

## Features

* Nearly complete LR35902 (Gameboy CPU) implementation.
* Support for Memory Bank Controllers: MBC0, MBC1, MBC3, and MBC5.
* Native support for Teensy 4.1, Windows, and Linux.
* Runs on the Teensy 4.1 using internal memory (does not require external PSRAM to work).
* APU supports Channel 1 and Channel 2 on PC

## TODO

* Add audio (APU) support.
* Make the PPU (Picture Processing Unit) modes dynamic.
* Add support for the remaining memory banks (e.g., MBC2).
* Design a custom PCB for the Teensy 4.1 hardware setup.

## Software Needed

### For Teensy 4.1

* [PlatformIO](https://platformio.org/)
* [ILI9341_t3n](https://github.com/KurtE/ILI9341_t3n) (Hardware-optimized display library)

### For PC

* A C++17 compatible compiler (GCC)
* [CMake](https://cmake.org/)
* [SDL2](https://www.libsdl.org/) and SDL2_ttf
* For Windows: [MSYS2](https://www.msys2.org/) (UCRT64 environment)

## Build Instructions

### Teensy 4.1

Make sure the hardware pins for the buttons are connected as follows:

* **Right:** 0, **Left:** 1, **Up:** 2, **Down:** 3, **B:** 4, **A:** 5, **Select:** 6, **Start:** 7

Connect the ILI9341 display to the following SPI pins:

* **CS:** 10, **DC:** 9, **RST:** 8, **MOSI:** 11, **SCK:** 13, **MISO:** 12

To build:

1. Open the project in PlatformIO.
2. Make sure you are using the `teensy41` project environment.
3. Compile and upload to the board.
4. To add ROMs, format a MicroSD card (FAT32), load the `.gb` files onto the root directory, and insert it into the Teensy.

### Linux

Install `cmake`, `gcc`, `g++`, and the SDL2 development libraries using your distribution's package manager (e.g., `sudo apt install cmake gcc g++ libsdl2-dev libsdl2-ttf-dev` on Debian/Ubuntu).

Then, navigate to the project directory and run:

```bash
mkdir -p build
cd build
cmake ..
make
./emulator

```

To load ROMs, place the `.gb` files in the same directory as the executable.

### Windows (MSYS2)

**1. Install MSYS2**
Download the installer from [msys2.org](https://www.msys2.org/) and run it. Accept the default installation folder (usually `C:\msys64`). Once finished, open the **MSYS2 UCRT64** terminal (this is the recommended environment for modern Windows development).

**2. Update the System**
In the UCRT64 terminal, update the package database and core system packages:

```bash
pacman -Syu

```

**3. Install the Toolchain and Libraries**
Install the GCC compiler, CMake, standard Make tool, SDL2, and SDL2_ttf using the `pacman` package manager.

*(Note: We are explicitly installing `mingw-w64-ucrt-x86_64-make` instead of Ninja).*

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-cmake \
          mingw-w64-ucrt-x86_64-make \
          mingw-w64-ucrt-x86_64-SDL2 \
          mingw-w64-ucrt-x86_64-SDL2_ttf

```

**4. Configure and Build**
Open the **MSYS2 UCRT64** terminal (or standard CMD/PowerShell if you have added MSYS2 to your PATH), navigate to your project folder using the `cd` command, and run the following commands:

**Generate the build files (using MinGW Makefiles):**

```bash
cmake -G "MinGW Makefiles" -B build

```

**Compile the executable:**

```bash
cmake --build build

```

**Run the compiled application:**

```bash
./build/emulator.exe

```

To load ROMs, place the `.gb` files in the directory where `./build/emulator.exe` is executed.