cmake_minimum_required(VERSION 3.10)
project(GameboyEmulator)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather hardware-agnostic core files
file(GLOB CORE_SOURCES "src/core/*.cpp")

# Gather PC-specific files
file(GLOB PC_SOURCES "src/pc/*.cpp")

# Based on your file tree, the abstract interfaces (ICartridge.cpp, IJoypad.cpp) 
# reside in the teensy directory. They must be compiled for the PC build to prevent linker errors.
set(INTERFACE_SOURCES 
    "src/ICartridge.cpp"
    "src/IJoypad.cpp"
)

# Create the executable (adding src/main.cpp which was missing)
add_executable(emulator 
    src/main.cpp
    ${CORE_SOURCES} 
    ${PC_SOURCES}
    ${INTERFACE_SOURCES}
)

# Replace include_directories(include) with the actual paths used in your project.
# target_include_directories is preferred in modern CMake.
target_include_directories(emulator PRIVATE 
    src/core
    src/pc
    src/teensy
    include
)

# Find and link required PC libraries (SDL2)
find_package(SDL2 REQUIRED)
target_include_directories(emulator PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(emulator PRIVATE ${SDL2_LIBRARIES})