#include "PCDisplay.h"
#include <iostream>

PCDisplay::PCDisplay() : window(nullptr), renderer(nullptr), texture(nullptr) {}

PCDisplay::~PCDisplay() {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    // In a full application, you might manage SDL_Init/SDL_Quit in a core System class, 
    // but subsystem management fits here for isolation.
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void PCDisplay::init() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Video initialization failed: " << SDL_GetError() << "\n";
        return;
    }

    window = SDL_CreateWindow(
        "Gameboy Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GB_WIDTH * SCALE,
        GB_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    if (!window) return;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) return;

    // Assuming the frame buffer is RGB565 (which matches the ILI9341 format)
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        GB_WIDTH,
        GB_HEIGHT
    );
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void PCDisplay::update(const uint16_t* frameBuffer) {
    if (!texture || !renderer) return;

    // SDL automatically handles the scaling to the window dimensions when rendering.
    // We just pass the raw 160x144 Game Boy buffer directly to the texture.
    // Pitch is width (160) * 2 bytes per pixel (RGB565) = 320 bytes.
    SDL_UpdateTexture(texture, nullptr, frameBuffer, GB_WIDTH * sizeof(uint16_t));
    
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool PCDisplay::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}