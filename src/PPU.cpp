#include "PPU.h"
#include "Bus.h"

PPU::PPU() : 
    bus{nullptr}, 
    vram{}, 
    oam{},
    dotLineCounter{0}, 
    lcdc{0x91}, 
    stat{0x85}, 
    scy{0}, scx{0}, 
    ly{0}, lyc{0}, 
    bgp{0xFC}, obp0{0xFF}, obp1{0xFF}, 
    wy{0}, wx{0}, 
    spriteInLine{0},
    bgLineColorIds{},
    sprites{}, 
    windowCounter{},
    lastSignal{false},
    frameReady{false}
{}


uint8_t PPU::read(uint16_t address) {
    if(address >= 0x8000 && address <= 0x9FFF) 
    {
        return vram[address - 0x8000];
    }
    else if(address >= 0xFE00 && address <= 0xFE9F) 
    {
        return oam[address - 0xFE00];
    } else {
        //DMA ($FF46) is not an actual register therefor it cant be read
        switch (address)
        {
        case 0xFF40: return lcdc;
        case 0xFF41: return stat;
        case 0xFF42: return scy;
        case 0xFF43: return scx;
        case 0xFF44: return ly;
        case 0xFF45: return lyc;
        case 0xFF47: return bgp;
        case 0xFF48: return obp0;
        case 0xFF49: return obp1;
        case 0xFF4A: return wy;
        case 0xFF4B: return wx;        
        default: return 0xFF;
        }
    }
}

void PPU::write(uint16_t address,uint8_t data) {
    if(address >= 0x8000 && address <= 0x9FFF) 
    {
        vram[address - 0x8000] = data;
        static int count = 0;
        if (count < 100) {
            count++;
        }
    }
    else if(address >= 0xFE00 && address <= 0xFE9F) 
    {
        oam[address - 0xFE00] = data;
    } else {
        switch (address)
        {
        //LY is read only
        case 0xFF40: {
            lcdc = data; 
            //Check if display is off
            if(!(lcdc & (1 << 7))) {
                stat &= 0xF8;
                dotLineCounter = 0;
                ly = 0;
                windowCounter = 0;
            }
            break;
        }
        case 0xFF41: stat = (stat & 0x07) | (data & 0xF8) | 0x80; break;
        case 0xFF42: scy = data; break;
        case 0xFF43: scx = data; break;
        case 0xFF45: lyc = data; compareLYC(); break;
        case 0xFF46: 
        {
            uint16_t source = data << 8;
            for(int i = 0; i < 160; i++) {
                oam[i] = bus->read(source + i);
            }
            break;
        }
        case 0xFF47: bgp = data; break;
        case 0xFF48: obp0 = data; break;
        case 0xFF49: obp1 = data; break;
        case 0xFF4A: wy = data; break;
        case 0xFF4B: wx = data; break;        
        default: break;
        }
    }
}

void PPU::step(int cycles) {
    //Check if display is off
    if(!(lcdc & (1 << 7)))  return;

    dotLineCounter += cycles;
    if(dotLineCounter >= 456) {
        dotLineCounter -= 456;

        if (ly < 144) {
            renderScanline();
        }
        
        ly++;

        compareLYC();
        if(ly == 144) {
            frameReady = true;
            bus->requestInterrupt(0);
        } else if(ly > 153) {
            ly = 0;
            windowCounter = 0;
        }
    }

    //TODO Make mode 3 and 0 non static
    if(ly >= 144) {
        updateMode(1);
    } else if(dotLineCounter < 80) {
        if((stat & 0x03) != 2)
            findSprites();
        updateMode(2);
    } else if(dotLineCounter < 252) {
        updateMode(3);
    } else if(dotLineCounter < 456) {
        updateMode(0);
    }

}

bool PPU::updateMode(uint8_t mode) {
    //Check if mode has changed
    uint8_t currentMode = stat & 0x03;
    if(currentMode == mode) return false;

    stat = (stat & 0xFC) | (mode & 0x03);

    updateInterrupts();
    return true;
}

void PPU::compareLYC() {
    if (ly == lyc) {
        stat |= (1 << 2);     
    } else {
        stat &= ~(1 << 2);    
    } 
    updateInterrupts();

}

void PPU::updateInterrupts() {
    uint8_t mode = stat & 0x03;
    bool lycInterrupt = (stat & (1 << 6)) && (stat & (1 << 2));
    bool m0Interrupt = (stat & (1 << 3)) && (mode == 0);
    bool m1Interrupt = (stat & (1 << 4)) && (mode == 1);
    bool m2Interrupt = (stat & (1 << 5)) && (mode == 2);
    bool currentSignal = lycInterrupt || m0Interrupt || m1Interrupt || m2Interrupt;
    if(currentSignal && ! lastSignal) bus->requestInterrupt(1);
    lastSignal = currentSignal;
}

PPU::Color PPU::mapIdToRGB565(uint8_t colorId, uint8_t palette) {
    uint8_t colorValue = (palette >> (colorId << 1)) & 0x03;
    switch (colorValue)
    {
    case 0: return Color::white;
    case 1: return Color::lightGray;
    case 2: return Color::darkGray;
    case 3: return Color::black;
    default: return Color::white;
    }

}

void PPU::renderBackground() {

    uint16_t mapBase = (lcdc & (1 << 3)) ? 0x1C00 : 0x1800;
    uint16_t baseAdress = (lcdc & (1 << 4)) ? 0 : 0x1000;
    uint16_t tileY = ((ly + scy) & 0xFF) >> 3;
    uint8_t lineInTile = (ly + scy) & 0x07;

    for(int x = 0; x < 160; x++) {
        uint16_t tileX = ((x + scx) & 0xFF) >> 3;
        uint16_t mapAddress = mapBase + (tileY << 5) + tileX;
        uint8_t tileId = vram[mapAddress];

        int16_t tileOffset = (lcdc & (1 <<4)) ? 
            static_cast<uint16_t>(tileId) << 4 : 
            static_cast<int16_t>(static_cast<int8_t>(tileId)) << 4;

        uint16_t tileAddress = baseAdress + tileOffset + (lineInTile << 1);

        uint8_t data1 = vram[tileAddress];
        uint8_t data2 = vram[tileAddress + 1];

        uint8_t bitPosition = 7 - ((x + scx) & 0x07); 
        uint8_t high = (data2 >> bitPosition) &0x01;
        uint8_t low = (data1 >> bitPosition) &0x01;
        uint8_t colorId = (high << 1) | low;
        Color color = mapIdToRGB565(colorId, bgp);
        bgLineColorIds[x] = colorId;
        frameBuffer[ly * 160 + x] = static_cast<uint16_t>(color);
    }
}

void PPU::renderWindow() {

    uint16_t mapBase = (lcdc & (1 << 6)) ? 0x1C00 : 0x1800;
    uint16_t baseAdress = (lcdc & (1 << 4)) ? 0 : 0x1000;
    uint16_t tileY = windowCounter >> 3;
    uint8_t lineInTile = windowCounter & 0x07;

    for(int x = 0; x < 160; x++) {
        
        if (x < (int)wx - 7) continue;

        int windowX = x - (wx - 7);
        uint16_t tileX = windowX >> 3;

        uint16_t mapAddress = mapBase + (tileY << 5) + tileX;
        uint8_t tileId = vram[mapAddress];

        int16_t tileOffset = (lcdc & (1 <<4)) ? 
            static_cast<uint16_t>(tileId) << 4 : 
            static_cast<int16_t>(static_cast<int8_t>(tileId)) << 4;

        uint16_t tileAddress = baseAdress + tileOffset + (lineInTile << 1);

        uint8_t data1 = vram[tileAddress];
        uint8_t data2 = vram[tileAddress + 1];

        uint8_t bitPosition = 7 - ( windowX & 0x07); 
        uint8_t high = (data2 >> bitPosition) & 0x01;
        uint8_t low = (data1 >> bitPosition) & 0x01;
        uint8_t colorId = (high << 1) | low;
        Color color = mapIdToRGB565(colorId, bgp);
        bgLineColorIds[x] = colorId;
        frameBuffer[ly * 160 + x] = static_cast<uint16_t>(color);
    }
}

void PPU::findSprites() {
    spriteInLine = 0;
    for(int i = 0; i <0xA0; i += 4) {
        if(spriteInLine == 10) return;

        // Sprites have a y offset of 16 and a x of 8
        Sprite sprite{static_cast<int16_t>(oam[i] - 16), static_cast<int16_t>(oam[i+1] - 8), oam[i+2], oam[i+3]};

        uint8_t height = (lcdc & (1 << 2)) ? 16 : 8;
        if((ly >= sprite.y) && (ly < (sprite.y + height))){
            sprites[spriteInLine++] = sprite;
        }
    }
}

void PPU::renderSprites() {
    if (!(lcdc & (1 << 1))) return;
    for(int i = spriteInLine - 1; i >= 0; i--) {
        Sprite sprite = sprites[i];
        uint8_t height = (lcdc & (1 << 2)) ? 16 : 8;

        int line = ly - sprite.y;

        //Flip sprite if necessary
        if(sprite.attributes & (1 << 6)) {
            line = (height - 1) - line;
        };

        uint8_t tileIndex = sprite.tileIndex;
        //If 2 tiles are used per sprite
        if(height == 16) {
            if(line <= 7)
                tileIndex = sprite.tileIndex & 0xFE;
            else
                tileIndex = sprite.tileIndex | 0x01;
        }

        int lineInTile = line % 8;

        uint16_t address = (tileIndex << 4) + (lineInTile << 1);
        uint8_t data1 = vram[address];
        uint8_t data2 = vram[address + 1];

        for(int x = 0; x < 8 ; x++) {
            int targetX = sprite.x + x;

            // Check if the pixel is inside the display
            if(targetX < 0 || targetX >= 160) continue;

            // Position of tile bit based on if its flipped 
            int bitPosition = (sprite.attributes & (1 << 5)) ? x : (7 - x);
            uint8_t high = (data2 >> bitPosition) &0x01;
            uint8_t low = (data1 >> bitPosition) &0x01;
            uint8_t colorId = (high << 1) | low;

            //Transparent
            if(colorId == 0) continue;

            // The sprite is draw if priority != 0 or if bg color ID is not 0
            bool priority = (sprite.attributes & (1 << 7));

            // Targeted volatile cast forces physical RAM fetch for this specific check,
            // bypassing register caching without slowing down renderBackground.
            uint8_t bgPixel = *(volatile uint8_t*)(&bgLineColorIds[targetX]);
            
            if(priority && bgPixel != 0) continue;

            uint8_t palette = (sprite.attributes & (1 << 4) ? obp1 : obp0);
            Color color = mapIdToRGB565(colorId, palette);
            frameBuffer[ly * 160 + targetX] = static_cast<uint16_t>(color);
        }
    }
}

void PPU::renderScanline() {

    //Is the background visible
    if (lcdc & (1 << 0)) {
        renderBackground();
    } else {
        // If the background is disabled treat it as color ID 0
        for (int x = 0; x < 160; x++) {
            bgLineColorIds[x] = 0;
            frameBuffer[ly * 160 + x] = static_cast<uint16_t>(mapIdToRGB565(0, bgp));
        }
    }

    //Is the window visible
    if(lcdc & (1 << 5) && (ly >= wy)) {
        renderWindow();
        windowCounter++;
    }

    renderSprites();


}

void PPU::setBus(Bus* b) {
    this->bus = b;
}

