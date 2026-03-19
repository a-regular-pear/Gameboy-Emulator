#include <Arduino.h>
#include <SD.h>
#include <new> // Required for std::nothrow memory allocation

#include "Cartridge.h"
#include "PPU.h"
#include "Bus.h"
#include "CPU.h" 
#include "Display.h"

// Hardware components
PPU ppu;
Cartridge cartridge;
Timer timer;
Bus bus(ppu, &cartridge, timer);
CPU cpu(bus);
Display display;

// Global pointer for the ROM memory (Frontend ownership)
uint8_t* gameMemory = nullptr;

void setup() {
    // Initialize the built-in LED for visual error indication
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH); 

    Serial.begin(115200);
    // Wait up to 3 seconds for the Serial Monitor to open
    while (!Serial && millis() < 3000); 

    Serial.println("\n=== TEENSY BOOTING ===");

    // Connect components to the memory bus
    ppu.setBus(&bus);
    timer.setBus(&bus);

    display.init();

    Serial.println("Initializing SD Card...");
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("CRITICAL ERROR: Failed to read SD card.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(100); // Fast blink: SD card initialization error
        }
    }

    // Define the target ROM filename (case-sensitive)
    const char* romFilename = "Tetris.gb"; 
    Serial.printf("Opening ROM file: %s\n", romFilename);
    
    File file = SD.open(romFilename); 
    if (!file) {
        Serial.println("CRITICAL ERROR: Failed to find ROM file.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(500); // Slow blink: File not found on SD card
        }
    }

    size_t fileSize = file.size();
    Serial.printf("File opened. Size: %u bytes\n", fileSize);

    Serial.println("Attempting memory allocation...");
    
    // Allocate memory with protection against hardware faults
    gameMemory = new (std::nothrow) uint8_t[fileSize];

    if (gameMemory == nullptr) {
        Serial.println("CRITICAL ERROR: OUT OF MEMORY!");
        Serial.println("The RAM of Teensy cannot provide a contiguous block for this file size.");
        file.close();
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(50); // Very fast blink: Out of Memory condition
        }
    }

    Serial.println("Memory allocated. Reading data...");
    file.read(gameMemory, fileSize);
    file.close();

    Serial.println("Loading ROM into Cartridge...");
    // Call the hardware-agnostic ROM loading function
    if (!cartridge.load_rom(gameMemory, fileSize)) {
        Serial.println("CRITICAL ERROR: Invalid ROM size or format.");
        while (true) {
            digitalWrite(13, HIGH); // Solid light: Invalid ROM size
        }
    }

    Serial.println("ROM LOADED SUCCESSFULLY.");
    digitalWrite(13, LOW); // Turn off LED: Successful boot sequence
}

void loop() {
    // Execute one CPU instruction and synchronize the timer and PPU cycles
    int cycles = cpu.step();
    timer.step(cycles);
    ppu.step(cycles);

    // Update the physical display when the PPU completes a full frame
    if (ppu.getFrameReady()) {
        display.update(ppu.getFrameBuffer()); 
        ppu.setFrameReady(false);
    }
}