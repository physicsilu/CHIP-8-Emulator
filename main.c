// #include "chip8.h"
// #include <stdio.h>

// int main(int argc, char* argv[]) {
//     CHIP8 chip8; // Create CHIP-8 instance
//     InitializeChip8(&chip8); // Initialize the chip8 structure
//     InitializeRNG(); // Initialize random number generator

//     // Initialize SDL
//     if (SDL_Init(SDL_INIT_VIDEO) != 0) {
//         printf("SDL Initialization Error: %s\n", SDL_GetError());
//         return -1;
//     }

//     // Create SDL window
//     SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
//     if (!window) {
//         printf("Window Creation Error: %s\n", SDL_GetError());
//         SDL_Quit();
//         return -1;
//     }

//     // Create SDL renderer
//     SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//     if (!renderer) {
//         printf("Renderer Creation Error: %s\n", SDL_GetError());
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return -1;
//     }

//     // Create texture for display
//     SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 64, 32);
//     if (!texture) {
//         printf("Texture Creation Error: %s\n", SDL_GetError());
//         SDL_DestroyRenderer(renderer);
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return -1;
//     }

//     // Load a ROM (pass the path of the ROM)
//     LoadROM(&chip8, "Tetris.ch8"); 

//     // Main loop
//     bool running = true;
    
//     while (running) {
//         running != ProcessInput(&chip8);

//         // Cycle through instructions
//         Cycle(&chip8);

//         // Update the display
//         UpdateDisplay(&chip8, renderer, texture);

//         // Delay for the emulation to run at a consistent speed (60 Hz refresh rate for the display)
//         SDL_Delay(1000 / 60);
//     }

//     // Cleanup and shutdown
//     SDL_DestroyTexture(texture);
//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     SDL_Quit();

//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"  // Assuming you have a header file defining the CHIP-8 structure and functions.

int main(int argc, char* argv[])
{
    int videoScale = 10;
    int cycleDelay = 8;
    const char* romFilename = "Tetris.ch8";

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL Initialization Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "Window Creation Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "Renderer Creation Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create SDL texture for video
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!texture)
    {
        fprintf(stderr, "Texture Creation Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Initialize CHIP-8 and load the ROM
    CHIP8 chip8;
    InitializeChip8(&chip8);  // Make sure this function initializes your CHIP-8 structure
    LoadROM(&chip8, romFilename);

    // Video pitch (row size in bytes for the display)
    int videoPitch = sizeof(chip8.display[0]) * DISPLAY_WIDTH;

    // Timing for the cycle delay
    Uint64 lastCycleTime = SDL_GetPerformanceCounter();
    bool quit = false;

    // Main loop
    while (!quit)
    {
        // Process input
        quit = ProcessInput(&chip8);

        // Get the current time
        Uint64 currentTime = SDL_GetPerformanceCounter();
        float dt = (currentTime - lastCycleTime) / (float)SDL_GetPerformanceFrequency() * 1000;  // in milliseconds

        // If it's time for the next cycle, process it
        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;

            // Cycle through the CHIP-8 instructions
            Cycle(&chip8);

            // Update the display
            UpdateDisplay(&chip8, renderer, texture);
        }

        // Delay to keep the emulation speed consistent
        SDL_Delay(1);
    }

    // Clean up and shut down SDL
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
