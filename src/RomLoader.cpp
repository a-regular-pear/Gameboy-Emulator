#include "RomLoader.h"

RomLoader::RomLoader(Display& d, Joypad& j) : display(d), joypad(j) {}

String RomLoader::selectROM() {
    // Play the nostalgic boot animation first
    bootAnimation();

    File root = SD.open("/");
    String files[64]; 
    int count = 0;

    while (count < 64) {
        File entry = root.openNextFile();
        if (!entry) break;
        String name = entry.name();
        if (name.endsWith(".gb") || name.endsWith(".GB")) {
            files[count++] = name;
        }
        entry.close();
    }

    int selected = 0;
    int scrollOffset = 0;

    while (true) {
        joypad.checkInput();
        
        if (joypad.isPressed(Joypad::Button::Up)) {
            if (selected > 0) {
                selected--;
                if (selected < scrollOffset) scrollOffset = selected;
            }
            delay(150); 
        }
        
        if (joypad.isPressed(Joypad::Button::Down)) {
            if (selected < count - 1) {
                selected++;
                if (selected >= scrollOffset + MAX_VISIBLE) scrollOffset++;
            }
            delay(150);
        }

        if (joypad.isPressed(Joypad::Button::A)) {
            return files[selected];
        }

        draw(files, count, selected, scrollOffset);
    }
}

void RomLoader::bootAnimation() {
    auto& tft = display.getTFT();
    int logoY = -50; // Start off-screen
    int targetY = 100;

    // Standard GameBoy boot behavior: Logo scrolls down
    while (logoY < targetY) {
        tft.fillScreen((uint16_t)Color::black);
        
        // Draw the "NINTENDO" replacement
        tft.setTextColor((uint16_t)Color::darkGray);
        tft.setTextSize(4);
        tft.setCursor(60, logoY);
        tft.print("TEENSY"); // Or "TEENSY"
        
        // Draw the register symbol (the little 'R')
        tft.setTextSize(1);
        tft.drawCircle(225, logoY + 5, 5, (uint16_t)Color::darkGray);
        tft.setCursor(223, logoY + 2);
        tft.print("R");

        tft.updateScreen();
        logoY += 4;
        delay(10);
    }

    // The "Bling" moment
    tft.setTextColor((uint16_t)Color::white);
    tft.setCursor(60, targetY);
    tft.setTextSize(4);
    tft.print("TEENSY");
    tft.updateScreen();
    
    // Hold the logo for a second like the real hardware
    delay(1200);
}

void RomLoader::draw(const String files[], int count, int selected, int offset) {
    auto& tft = display.getTFT();
    tft.fillScreen((uint16_t)Color::black); 

    // Header bar
    tft.fillRect(0, 0, 320, 35, (uint16_t)Color::darkGray);
    tft.setCursor(20, 10);
    tft.setTextColor((uint16_t)Color::white);
    tft.setTextSize(2);
    tft.println("TEENSY LOADER");

    for (int i = 0; i < MAX_VISIBLE && (i + offset) < count; i++) {
        int idx = i + offset;
        int yPos = 50 + (i * 22);
        
        if (idx == selected) {
            tft.fillRect(10, yPos - 2, 300, 20, (uint16_t)Color::lightGray);
            tft.setTextColor((uint16_t)Color::black); 
            tft.setCursor(25, yPos);
            tft.print("> ");
        } else {
            tft.setTextColor((uint16_t)Color::white);
            tft.setCursor(25, yPos);
            tft.print("  ");
        }
        tft.println(files[idx]);
    }
    tft.updateScreen();
}

