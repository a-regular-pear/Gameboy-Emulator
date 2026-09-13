cmake_minimum_required(VERSION 3.10)
project(GameboyEmulator)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather hardware-agnostic core files
file(GLOB CORE_SOURCES "src/core/*.cpp")

# Gather PC-specific files
file(GLOB PC_SOURCES "src/pc/*.cpp")

# Abstract interfaces implementation files
set(INTERFACE_SOURCES 
    "src/ICartridge.cpp"
    "src/IJoypad.cpp"
)

# Create the executable
add_executable(emulator 
    src/main.cpp
    ${CORE_SOURCES} 
    ${PC_SOURCES}
    ${INTERFACE_SOURCES}
)

# Include directories configuration
target_include_directories(emulator PRIVATE 
    src/core
    src/pc
    src/teensy
    include
)

# Find SDL2 using its package configuration
find_package(SDL2 REQUIRED)
target_include_directories(emulator PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(emulator PRIVATE ${SDL2_LIBRARIES})
