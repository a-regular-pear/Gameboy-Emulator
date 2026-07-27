#pragma once
#include "IDisplay.h"
#include <SDL.h>

class PCDisplay : public IDisplay
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    static constexpr int GB_WIDTH = 160;
    static constexpr int GB_HEIGHT = 144;
    static constexpr int SCALE = 3; 

public:
    PCDisplay();
    ~PCDisplay() override;
    
    void init() override;
    void update(const uint16_t* frameBuffer) override;
    
    bool processEvents(); 
};