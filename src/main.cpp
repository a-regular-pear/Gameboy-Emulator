#if defined(ARDUINO)
    #include <Arduino.h>
    #include <SD.h>
    #include <TimeLib.h>
    
    #include "TeensyCartridge.h"
    #include "TeensyDisplay.h"
    #include "TeensyJoypad.h"
    #include "TeensyRomLoader.h"
    
    using PlatformCartridge = TeensyCartridge;
    using PlatformDisplay = TeensyDisplay;
    using PlatformJoypad = TeensyJoypad;
    using PlatformRomLoader = TeensyRomLoader;
#else
    #include <iostream>
    #include <fstream>
    #include <chrono>
    #include <thread>
    #include <cstdlib>
    
    #include "PCCartridge.h"
    #include "PCDisplay.h"
    #include "PCJoypad.h"
    #include "PCRomLoader.h"
    
    using PlatformCartridge = PCCartridge;
    using PlatformDisplay = PCDisplay;
    using PlatformJoypad = PCJoypad;
    using PlatformRomLoader = PCRomLoader;
#endif

#include <new> // Required for std::nothrow memory allocation
#include "PPU.h"
#include "Bus.h"
#include "CPU.h"
#include "Timer.h"

// Hardware components
PPU ppu;
PlatformCartridge cartridge;
Timer timer;
PlatformJoypad joypad;
Bus bus(ppu, &cartridge, timer, joypad);
CPU cpu(bus);
PlatformDisplay display;

// Global pointer for the ROM memory (Frontend ownership)
uint8_t* gameMemory = nullptr;

#if defined(ARDUINO)
time_t getTeensy3Time() {
    return Teensy3Clock.get();
}

#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#define PRINTF(...) Serial.printf(__VA_ARGS__)

#else

uint32_t millis() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

uint32_t micros() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#define PRINT(x) std::cout << (x)
#define PRINTLN(x) std::cout << (x) << std::endl
#define PRINTF(...) printf(__VA_ARGS__)

#endif

void setup() {
#if defined(ARDUINO)
    setSyncProvider(getTeensy3Time);

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH); 
    Serial.begin(115200);
    while (!Serial && millis() < 3000); 
#endif

    PRINTLN("\n=== EMULATOR BOOTING ===");

    ppu.setBus(&bus);
    timer.setBus(&bus);
    
    display.init();
    joypad.init();

#if defined(ARDUINO)
    PRINTLN("Initializing SD Card...");
    if (!SD.begin(BUILTIN_SDCARD)) {
        PRINTLN("CRITICAL ERROR: Failed to read SD card.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(100); 
        }
    }
#endif

    PlatformRomLoader loader(display, joypad);
    std::string romName = loader.selectROM();
    const char* romFilename = romName.c_str(); 

#if defined(ARDUINO)
    display.getTFT().fillScreen(0x09C1); 

    File file = SD.open(romFilename); 
    if (!file) {
        PRINTLN("CRITICAL ERROR: Failed to find ROM file.");
        while (true) {
            digitalWrite(13, !digitalRead(13));
            delay(500); 
        }
    }
    size_t fileSize = file.size();
#else
    std::ifstream file(romFilename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        PRINTLN("CRITICAL ERROR: Failed to find ROM file.");
        exit(1);
    }
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
#endif

    PRINTF("File opened. Size: %u bytes\n", static_cast<unsigned int>(fileSize));
    PRINTLN("Attempting RAM allocation...");
    
    gameMemory = new (std::nothrow) uint8_t[fileSize];
    bool loadSuccess = false;

    if (gameMemory != nullptr) {
        PRINTLN("Memory allocated. Loading full ROM into RAM...");
#if defined(ARDUINO)
        file.read(gameMemory, fileSize);
        file.close();
#else
        file.read(reinterpret_cast<char*>(gameMemory), fileSize);
        file.close();
#endif
        loadSuccess = cartridge.load_rom(gameMemory, fileSize, false, romFilename);
    } else {
        PRINTLN("Out of memory for full load. Falling back to SD streaming...");
#if defined(ARDUINO)
        file.close(); 
#else
        file.close();
#endif
        loadSuccess = cartridge.load_rom(nullptr, fileSize, true, romFilename);
    }

    if (!loadSuccess) {
        PRINTLN("CRITICAL ERROR: Invalid ROM size or format.");
#if defined(ARDUINO)
        while (true) {
            digitalWrite(13, HIGH); 
        }
#else
        exit(1);
#endif
    }
    
    PRINTLN("ROM LOADED SUCCESSFULLY.");
#if defined(ARDUINO)
    digitalWrite(13, LOW); 
#endif
}

void loop() {
#if !defined(ARDUINO)
    if (!display.processEvents()) {
        exit(0);
    }
#endif

    static uint32_t lastSaveMillis = 0;
    static uint32_t rtcMicrosAccumulator = 0;
    static uint32_t lastRtcTime = micros(); 

    static uint32_t savePendingTime = 0;
    static bool isSavePending = false;

    uint32_t frameStart = micros();

    joypad.checkInput();

    while (!ppu.getFrameReady()) {
        int cycles = cpu.step();
        timer.step(cycles);
        ppu.step(cycles);
    }

    display.update(ppu.getFrameBuffer()); 
    ppu.setFrameReady(false);
    
    uint32_t frameTime = micros() - frameStart;
    if(frameTime < 16742) {
        delayMicroseconds(16742 - frameTime);
        frameTime = 16742;
    }

    uint32_t nowMicros = micros();
    uint32_t deltaRtc = nowMicros - lastRtcTime;
    lastRtcTime = nowMicros;
    
    rtcMicrosAccumulator += deltaRtc;
    while (rtcMicrosAccumulator >= 1000000 && (cartridge.getMBC() == 3)) {
        cartridge.addSeconds(1);
        rtcMicrosAccumulator -= 1000000;
    }

    if (cartridge.getForceSave()) {
        isSavePending = true;
        savePendingTime = millis();
    }

    if (isSavePending && (millis() - savePendingTime > 500)) {
        cartridge.save(); 
        cartridge.setIsDirty(false);
        lastSaveMillis = millis(); 
        isSavePending = false;
        PRINTLN("Hardware Save committed to storage.");
    }

    if (cartridge.gethasBattery() && cartridge.getIsDirty() && (millis() - lastSaveMillis > 30000)) {
        cartridge.save(); 
        cartridge.setIsDirty(false);
        lastSaveMillis = millis();
        isSavePending = false; 
        PRINTLN("Autosave complete.");
    }
}

#if !defined(ARDUINO)
int main(int argc, char* argv[]) {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
#endif