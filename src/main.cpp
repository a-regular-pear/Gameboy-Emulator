#include <Arduino.h>
#include <SD.h>
#include <vector>

#include "Cartridge.h"
#include "PPU.h"
#include "Bus.h"
#include "CPU.h" 

PPU ppu;
Cartridge cartridge;
Bus bus(ppu, &cartridge);
CPU cpu(bus);

void setup() {
    Serial.begin(115200);

    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("Failed to read SD");
        while (true);
    }

    File file = SD.open("Test.gb");
    if (!file) {
        Serial.println("Failed to find rom");
        while (true);
    }

    size_t fileSize = file.size();
    std::vector<uint8_t> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    if (!cartridge.load_rom(buffer)) {
        Serial.println("Rom is too small");
        while (true);
    }

    Serial.println("ROM LOADED");
}

void loop() {
    cpu.step();

    if (bus.read(0xFF02) == 0x81) {
        char c = static_cast<char>(bus.read(0xFF01));
        Serial.print(c);
        
        bus.write(0xFF02, 0x00);
    }
}