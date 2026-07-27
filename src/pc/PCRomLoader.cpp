#include "PCRomLoader.h"
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

PCRomLoader::PCRomLoader(IDisplay& d, IJoypad& j) : IRomLoader(d, j) {}

std::string PCRomLoader::selectROM() {
    std::vector<std::string> files;
    
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".gb" || ext == ".GB") {
                files.push_back(entry.path().string());
            }
        }
    }

    if (files.empty()) {
        std::cerr << "No Gameboy ROMs (.gb) found in the current directory.\n";
        return "";
    }

    std::cout << "--- PC ROM LOADER ---\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "[" << i << "] " << files[i] << "\n";
    }

    int selected = -1;
    while (selected < 0 || selected >= static_cast<int>(files.size())) {
        std::cout << "\nSelect a ROM by number: ";
        std::cin >> selected;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            selected = -1;
        }
    }

    return files[selected];
}