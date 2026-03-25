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
Joypad joypad;
Bus bus(ppu, &cartridge, timer, joypad);
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
    joypad.init();

    Serial.println("Initializing SD Card...");
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("CRITICAL ERROR: Failed to read SD card.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(100); // Fast blink: SD card initialization error
        }
    }

    const char* romFilename = "Tetris.gb"; 

    // Open the file briefly to determine its actual size
    File file = SD.open(romFilename); 
    if (!file) {
        Serial.println("CRITICAL ERROR: Failed to find ROM file.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(500); 
        }
    }

    size_t fileSize = file.size();
    Serial.printf("File opened. Size: %u bytes\n", fileSize);

    Serial.println("Attempting RAM allocation...");
    // Allocate memory with protection against hardware faults
    gameMemory = new (std::nothrow) uint8_t[fileSize];

    bool loadSuccess = false;

    // Try RAM first, fallback to Streaming
    if (gameMemory != nullptr) {
        Serial.println("Memory allocated. Loading full ROM into RAM...");
        file.read(gameMemory, fileSize);
        file.close();
        loadSuccess = cartridge.load_rom(gameMemory, fileSize);
    } else {
        Serial.println("Out of memory for full load. Falling back to SD streaming...");
        file.close(); // Close so Cartridge::load_rom can reopen it
        loadSuccess = cartridge.load_rom(nullptr, fileSize, true, romFilename);
    }

    if (!loadSuccess) {
        Serial.println("CRITICAL ERROR: Invalid ROM size or format.");
        while (true) {
            digitalWrite(13, HIGH); // Solid light: Invalid ROM size
        }
    }

    Serial.println("ROM LOADED SUCCESSFULLY.");
    digitalWrite(13, LOW); // Turn off LED: Successful boot sequence
}

void loop() {
    static uint32_t rtcMicrosAccumulator = 0;
    uint32_t frameStart = micros();

    joypad.checkInput();
    // Execute CPU instructions and synchronize the timer and PPU cycles
    while (!ppu.getFrameReady()) {
        int cycles = cpu.step();
        timer.step(cycles);
        ppu.step(cycles);

    }
    // Update the physical display when the PPU completes a full frame
    display.update(ppu.getFrameBuffer()); 
    ppu.setFrameReady(false);
    
    uint32_t frameTime = micros() - frameStart;
    if(frameTime < 16742) {
        delayMicroseconds(16742 - frameTime);
        frameTime = 16742;
    }

    rtcMicrosAccumulator += frameTime;
    if (rtcMicrosAccumulator >= 1000000 && (cartridge.getMBC() == 3)) { // 1 million micros = 1 second
        cartridge.addSeconds(1);
        rtcMicrosAccumulator -= 1000000;
    }
}