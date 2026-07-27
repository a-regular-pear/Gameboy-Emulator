#include "PCRomLoader.h"
#include "PCDisplay.h"
#include "IJoypad.h"
#include <filesystem>
#include <thread>
#include <chrono>
#include <cstring>

namespace fs = std::filesystem;

// Πρέπει να ταιριάζουν με τα ίδια GB_WIDTH/GB_HEIGHT που χρησιμοποιεί το PCDisplay
constexpr int GB_WIDTH  = 160;
constexpr int GB_HEIGHT = 144;

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static const uint16_t COLOR_BLACK     = rgb565(0, 0, 0);
static const uint16_t COLOR_WHITE     = rgb565(255, 255, 255);
static const uint16_t COLOR_DARKGRAY  = rgb565(80, 80, 80);
static const uint16_t COLOR_LIGHTGRAY = rgb565(200, 200, 200);

// Πολύ μικρή 3x5 bitmap font. Καλύπτει A-Z, 0-9, ".", "-", "_", ">", " ".
// Κάθε char = 5 bytes, κάθε byte = 1 γραμμή, τα 3 LSB είναι τα pixels (bit2..bit0).
static uint8_t getFontRow(char c, int row) {
    c = toupper(c);
    static const uint8_t A[5] = {0b010,0b101,0b111,0b101,0b101};
    static const uint8_t B[5] = {0b110,0b101,0b110,0b101,0b110};
    static const uint8_t C[5] = {0b011,0b100,0b100,0b100,0b011};
    static const uint8_t D[5] = {0b110,0b101,0b101,0b101,0b110};
    static const uint8_t E[5] = {0b111,0b100,0b110,0b100,0b111};
    static const uint8_t F[5] = {0b111,0b100,0b110,0b100,0b100};
    static const uint8_t G[5] = {0b011,0b100,0b101,0b101,0b011};
    static const uint8_t H[5] = {0b101,0b101,0b111,0b101,0b101};
    static const uint8_t I[5] = {0b111,0b010,0b010,0b010,0b111};
    static const uint8_t J[5] = {0b001,0b001,0b001,0b101,0b010};
    static const uint8_t K[5] = {0b101,0b101,0b110,0b101,0b101};
    static const uint8_t L[5] = {0b100,0b100,0b100,0b100,0b111};
    static const uint8_t M[5] = {0b101,0b111,0b111,0b101,0b101};
    static const uint8_t N[5] = {0b101,0b111,0b111,0b111,0b101};
    static const uint8_t O[5] = {0b010,0b101,0b101,0b101,0b010};
    static const uint8_t P[5] = {0b110,0b101,0b110,0b100,0b100};
    static const uint8_t Q[5] = {0b010,0b101,0b101,0b111,0b011};
    static const uint8_t R[5] = {0b110,0b101,0b110,0b101,0b101};
    static const uint8_t S[5] = {0b011,0b100,0b010,0b001,0b110};
    static const uint8_t T[5] = {0b111,0b010,0b010,0b010,0b010};
    static const uint8_t U[5] = {0b101,0b101,0b101,0b101,0b111};
    static const uint8_t V[5] = {0b101,0b101,0b101,0b101,0b010};
    static const uint8_t W[5] = {0b101,0b101,0b111,0b111,0b101};
    static const uint8_t X[5] = {0b101,0b101,0b010,0b101,0b101};
    static const uint8_t Y[5] = {0b101,0b101,0b010,0b010,0b010};
    static const uint8_t Z[5] = {0b111,0b001,0b010,0b100,0b111};
    static const uint8_t D0[5]= {0b010,0b101,0b101,0b101,0b010};
    static const uint8_t D1[5]= {0b010,0b110,0b010,0b010,0b111};
    static const uint8_t D2[5]= {0b110,0b001,0b010,0b100,0b111};
    static const uint8_t D3[5]= {0b110,0b001,0b010,0b001,0b110};
    static const uint8_t D4[5]= {0b101,0b101,0b111,0b001,0b001};
    static const uint8_t D5[5]= {0b111,0b100,0b110,0b001,0b110};
    static const uint8_t D6[5]= {0b011,0b100,0b110,0b101,0b010};
    static const uint8_t D7[5]= {0b111,0b001,0b010,0b010,0b010};
    static const uint8_t D8[5]= {0b010,0b101,0b010,0b101,0b010};
    static const uint8_t D9[5]= {0b010,0b101,0b011,0b001,0b110};
    static const uint8_t DOT[5]={0,0,0,0,0b010};
    static const uint8_t DASH[5]={0,0,0b111,0,0};
    static const uint8_t US[5]  ={0,0,0,0,0b111};
    static const uint8_t GT[5]  ={0b100,0b010,0b001,0b010,0b100};
    static const uint8_t SP[5]  ={0,0,0,0,0};

    switch (c) {
        case 'A': return A[row]; case 'B': return B[row]; case 'C': return C[row];
        case 'D': return D[row]; case 'E': return E[row]; case 'F': return F[row];
        case 'G': return G[row]; case 'H': return H[row]; case 'I': return I[row];
        case 'J': return J[row]; case 'K': return K[row]; case 'L': return L[row];
        case 'M': return M[row]; case 'N': return N[row]; case 'O': return O[row];
        case 'P': return P[row]; case 'Q': return Q[row]; case 'R': return R[row];
        case 'S': return S[row]; case 'T': return T[row]; case 'U': return U[row];
        case 'V': return V[row]; case 'W': return W[row]; case 'X': return X[row];
        case 'Y': return Y[row]; case 'Z': return Z[row];
        case '0': return D0[row]; case '1': return D1[row]; case '2': return D2[row];
        case '3': return D3[row]; case '4': return D4[row]; case '5': return D5[row];
        case '6': return D6[row]; case '7': return D7[row]; case '8': return D8[row];
        case '9': return D9[row];
        case '.': return DOT[row]; case '-': return DASH[row]; case '_': return US[row];
        case '>': return GT[row];
        default:  return SP[row];
    }
}

PCRomLoader::PCRomLoader(IDisplay& d, IJoypad& j)
    : IRomLoader(d, j), frameBuffer(GB_WIDTH * GB_HEIGHT, COLOR_BLACK) {}

void PCRomLoader::clear(uint16_t color) {
    std::fill(frameBuffer.begin(), frameBuffer.end(), color);
}

void PCRomLoader::fillRect(int x, int y, int w, int h, uint16_t color) {
    for (int yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= GB_HEIGHT) continue;
        for (int xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= GB_WIDTH) continue;
            frameBuffer[yy * GB_WIDTH + xx] = color;
        }
    }
}

void PCRomLoader::drawChar(int x, int y, char c, uint16_t color) {
    for (int row = 0; row < 5; ++row) {
        uint8_t bits = getFontRow(c, row);
        for (int col = 0; col < 3; ++col) {
            if (bits & (1 << (2 - col))) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < GB_WIDTH && py >= 0 && py < GB_HEIGHT)
                    frameBuffer[py * GB_WIDTH + px] = color;
            }
        }
    }
}

void PCRomLoader::drawText(int x, int y, const std::string& text, uint16_t color) {
    int cx = x;
    for (char c : text) {
        drawChar(cx, y, c, color);
        cx += 4; // 3px char + 1px κενό
    }
}

std::string PCRomLoader::selectROM()
{
    bootAnimation();

    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".gb" || ext == ".GB") {
                files.push_back(entry.path().filename().string());
            }
        }
    }

    int selected = 0;
    int scrollOffset = 0;
    
    // Αρχικοποίηση χρονομετρητή για το debounce του πληκτρολογίου
    auto lastInputTime = std::chrono::steady_clock::now();

    while (true) {
        // Λύση 1: Ασφαλές casting για πρόσβαση στη processEvents
        auto* pcDisplay = dynamic_cast<PCDisplay*>(&display);
        if (pcDisplay && !pcDisplay->processEvents()) {
            return ""; 
        }

        joypad.checkInput();

        // Λύση 3: Χρήση χρονομετρητή αντί για sleep_for (150ms cooldown)
        auto now = std::chrono::steady_clock::now();
        bool inputAllowed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputTime).count() > 150;

        if (inputAllowed) {
            if (joypad.isPressed(IJoypad::Button::Up)) {
                if (selected > 0) {
                    selected--;
                    if (selected < scrollOffset) scrollOffset = selected;
                }
                lastInputTime = now;
            }

            if (joypad.isPressed(IJoypad::Button::Down)) {
                if (selected < static_cast<int>(files.size()) - 1) {
                    selected++;
                    if (selected >= scrollOffset + MAX_VISIBLE) scrollOffset++;
                }
                lastInputTime = now;
            }

            if (joypad.isPressed(IJoypad::Button::A)) {
                // Λύση 2: Έλεγχος αν η λίστα είναι άδεια πριν την επιλογή
                if (!files.empty()) {
                    return files[selected];
                }
            }
        }

        draw(files, selected, scrollOffset);
    }
}
void PCRomLoader::bootAnimation()
{
    int logoY = -20;
    int targetY = 60;

    while (logoY < targetY) {
        clear(COLOR_BLACK);
        drawText(60, logoY, "PC LOADER", COLOR_DARKGRAY);
        display.update(frameBuffer.data());
        logoY += 2;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    clear(COLOR_BLACK);
    drawText(60, targetY, "PC LOADER", COLOR_WHITE);
    display.update(frameBuffer.data());
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
}

void PCRomLoader::draw(const std::vector<std::string>& files, int selected, int offset)
{
    clear(COLOR_BLACK);

    fillRect(0, 0, GB_WIDTH, 12, COLOR_DARKGRAY);
    drawText(4, 3, "PC ROM LOADER", COLOR_WHITE);

    for (int i = 0; i < MAX_VISIBLE && (i + offset) < static_cast<int>(files.size()); ++i) {
        int idx = i + offset;
        int yPos = 16 + (i * 8);

        if (idx == selected) {
            fillRect(2, yPos - 1, GB_WIDTH - 4, 8, COLOR_LIGHTGRAY);
            drawText(4, yPos, "> " + files[idx], COLOR_BLACK);
        } else {
            drawText(4, yPos, "  " + files[idx], COLOR_WHITE);
        }
    }

    display.update(frameBuffer.data());
}