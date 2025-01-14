// We do this ifndef stuff no not include the header file multiple times. This is like #pragma once 

#ifndef CHIP_8
#define CHIP_8

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<SDL2/SDL.h>
#include<stdbool.h>

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define FONTSET_SIZE 80
#define START_ADDRESS 0x200
#define FONTSET_START_ADDRESS 0x50


typedef struct
{
    uint8_t memory[MEMORY_SIZE];                        // 4kB RAM
    uint8_t registers[NUM_REGISTERS];                   // 16 8 bit registers
    uint16_t IndexRegister;                             // 16 bit index register
    uint16_t PC;                                        // 16 bit program counter
    uint16_t stack[STACK_SIZE];                         // 16 level stack. Each level holds 16 bit address
    uint8_t stkptr;                                     // 8 bit stack pointer
    uint8_t DelayTimer;                                 // 8 bit delay timer
    uint8_t SoundTimer;                                 // 8 bit sound timer
    uint8_t keypad[KEYPAD_SIZE];                        // 16 keys
    uint32_t display[DISPLAY_WIDTH*DISPLAY_HEIGHT];     // 64x32 display
} CHIP8;

typedef void (*InstructionHandler)(CHIP8*, uint16_t); // Function Pointer

// Declare the function pointer table
extern InstructionHandler functionTable[0x10]; // 16 primary opcode groups (0x0 to 0xF)
extern InstructionHandler table8[0x10]; // Subtable for 8xy_ instructions
extern InstructionHandler tableE[0xFF]; // Subtable for Ex__ instructions
extern InstructionHandler tableF[0xFF]; // Subtable for Fx__ instructions

void InitializeChip8(CHIP8* chip8);
void LoadROM(CHIP8* chip8 ,const char* filename);
void InitializeRNG();
uint8_t GenerateRandomByte(uint8_t mask);
void InitializeFunctionTable();
void InitializeTable8();
void InitializeTableE();
void InitializeTableF();
void Cycle(CHIP8* chip8);
void InitializeSDL();
void UpdateDisplay(CHIP8* chip8, SDL_Renderer* renderer, SDL_Texture* texture);
bool ProcessInput(CHIP8* chip8);
#endif 

// In this file the function definitions are included
#include"chip8.h"
#include"instructions.h"

InstructionHandler functionTable[0x10]; // Defining functionTable
InstructionHandler table8[0x10];        // Defining table8
InstructionHandler tableE[0xFF];        // Defining tableE
InstructionHandler tableF[0xFF];        // Defining table

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void InitializeFunctionTable() {
    functionTable[0x0] = OP_00E0; 
    functionTable[0x1] = OP_1nnn;
    functionTable[0x2] = OP_2nnn;
    functionTable[0x3] = OP_3xkk;
    functionTable[0x4] = OP_4xkk;
    functionTable[0x5] = OP_5xy0;
    functionTable[0x6] = OP_6xkk;
    functionTable[0x7] = OP_7xkk;
    functionTable[0x8] = NULL;   // Complex cases (e.g., 8xy0 to 8xyE) need additional logic
    functionTable[0x9] = OP_9xy0;
    functionTable[0xA] = OP_Annn;
    functionTable[0xB] = OP_Bnnn;
    functionTable[0xC] = OP_Cxkk;
    functionTable[0xD] = OP_Dxyn;
    functionTable[0xE] = NULL;   // Complex cases (e.g., Ex9E, ExA1) need additional logic
    functionTable[0xF] = NULL;   // Complex cases (e.g., Fx07 to Fx65) need additional logic
}

void InitializeTable8() {
    table8[0x0] = OP_8xy0;
    table8[0x1] = OP_8xy1;
    table8[0x2] = OP_8xy2;
    table8[0x3] = OP_8xy3;
    table8[0x4] = OP_8xy4;
    table8[0x5] = OP_8xy5;
    table8[0x6] = OP_8xy6;
    table8[0x7] = OP_8xy7;
    table8[0xE] = OP_8xyE;
    // Other values remain NULL
}

void InitializeTableE(){
    tableE[0x9E] = OP_Ex9E;
    tableE[0xA1] = OP_ExA1;
    // Other values remain NULL
}

void InitializeTableF(){
    tableF[0x07] = OP_Fx07;
    tableF[0x0A] = OP_Fx0A;
    tableF[0x15] = OP_Fx15;
    tableF[0x18] = OP_Fx18;
    tableF[0x1E] = OP_Fx1E;
    tableF[0x29] = OP_Fx29;
    tableF[0x33] = OP_Fx33;
    tableF[0x55] = OP_Fx55;
    tableF[0x65] = OP_Fx65;
    // Other values remain NULL
}

void InitializeChip8(CHIP8* chip8){
    chip8->PC = START_ADDRESS; // Initially the program counter points to 0x200
    chip8->IndexRegister = 0;  // Initializing index register
    chip8->stkptr = 0;         // Initializing stack pointer
    chip8->DelayTimer = 0;     // Initializing delay timer
    chip8->SoundTimer = 0;     // Initializing sound timer

    // Initialize the display 
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        chip8->display[i] = 0;  // Turn all pixels off initially
    }

    // Loading the fonts into memory
    for (int i = 0; i < FONTSET_SIZE; i++)
    {
        chip8->memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    
    InitializeFunctionTable();
    InitializeTable8();
    InitializeTableE();
    InitializeTableF();
}

void LoadROM(CHIP8*chip8, const char* filename){
    FILE* fptr; // File pointer 

    fptr = fopen(filename, "rb"); // opening the file in binary mode

    if(fptr == NULL){
        printf("File is not opened. Program Terminate.\n");
        exit(0);
    }

    fseek(fptr, 0, SEEK_END); // moves the file pointer to the end
    long FileSize = ftell(fptr); // returns current position, size of file
    rewind(fptr); // Moves back to beginning of the file

    // Allocate memory for the ROM content
    uint8_t* buffer = (uint8_t*)malloc(FileSize);
    if (buffer == NULL) {
        printf("Failed to allocate memory for ROM\n");
        fclose(fptr);
        exit(1);
    }

    // Reading ROM to buffer
    fread(buffer, 1, FileSize, fptr);

    /*
        *********fread()********
        size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
            ptr: A pointer to the buffer where the data will be stored.
            size: The size of each element to be read (in bytes).
            nmemb: The number of elements to be read.
            stream: The file stream (pointer to a FILE object) from which to read.
    */
    fclose(fptr);

    // Loading to memory
    for (long i = 0; i < FileSize; i++)
    {
        chip8-> memory[START_ADDRESS + i] = buffer[i];
    }
    
    free(buffer); // very important step. never forget!
}

void InitializeRNG(){
    srand((unsigned int)time(NULL)); // This ensures random numbers are different each time the program runs
}

//This function creates a random number between 0 and 255 and applies the mask (kk)
uint8_t GenerateRandomByte(uint8_t mask){
    uint8_t RandomValue = rand() % 256;
    return RandomValue & mask;
}

// Fetch, Decode, Execute
void Cycle(CHIP8* chip8){
    // Fetch
    uint16_t opcode = ((chip8->memory[chip8->PC]) << 8) | (chip8->memory[chip8->PC + 1]); // 16 bit opcode right, so two bytes

    // Incrementing PC
    chip8->PC = chip8->PC + 2;

    // Execute
    uint8_t category = (opcode & 0xF000) >> 12; // First Nibble

    if (functionTable[category] != NULL){
        functionTable[category](chip8, opcode);
    }
    else{
        // Handling special cases

        switch (category)
        {
        case 0x8: {
            uint8_t subcode = opcode & 0x000F; // Last nibble
            if (table8[subcode] != NULL){
                table8[subcode](chip8, opcode);
            }
            else{
                printf("Unknown instruction: 0x%04X\n", opcode);
            }
            
            break;
        }
        
        case 0xE: {
            uint8_t subcode = opcode & 0x00FF; // Last 8 bits
            if (tableE[subcode] != NULL){
                tableE[subcode](chip8, opcode);
            }else{
                printf("Unknown instruction: 0x%04X\n", opcode);
            }
            
            break;
        }

        case 0xF: {
            uint8_t subcode = opcode & 0x00FF; // Last 8 bits
            if (tableF[subcode] != NULL){
                tableF[subcode](chip8, opcode);
            }else{
                printf("Unknown instruction: 0x%04X\n", opcode);
            }
            
            break;
        }

        default:
            printf("Unknown instruction: 0x%04X\n", opcode);
            break;
        }
    }
    
    // Decrement delay timer
    if (chip8->DelayTimer > 0)
    {
        chip8->DelayTimer--;
    }

    // Decrement sound timer
    if (chip8->SoundTimer > 0)
    {
        chip8->SoundTimer--;
    }
}

void InitializeSDL(){
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create texture for displaying the screen
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void UpdateDisplay(CHIP8* chip8, SDL_Renderer* renderer, SDL_Texture* texture) {
    uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    
    // Convert CHIP8 display to SDL format
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        if (chip8->display[i] == 1) {
            pixels[i] = 0xFFFFFF;  // White pixel
        } else {
            pixels[i] = 0x000000;  // Black pixel
        }
    }

    // Update the texture with pixel data
    SDL_UpdateTexture(texture, NULL, pixels, DISPLAY_WIDTH * sizeof(uint32_t));

    // Clear the screen
    SDL_RenderClear(renderer);

    // Copy the texture to the screen
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Present the rendered screen
    SDL_RenderPresent(renderer);
}

bool ProcessInput(CHIP8* chip8) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true; // This will trigger when the user closes the window
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                quit = true; // This will trigger when the user presses ESC
            } else {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_X:
                        chip8->keypad[0] = 1;
                        break;
                    case SDL_SCANCODE_1:
                        chip8->keypad[1] = 1;
                        break;
                    case SDL_SCANCODE_2:
                        chip8->keypad[2] = 1;
                        break;
                    case SDL_SCANCODE_3:
                        chip8->keypad[3] = 1;
                        break;
                    case SDL_SCANCODE_Q:
                        chip8->keypad[4] = 1;
                        break;
                    case SDL_SCANCODE_W:
                        chip8->keypad[5] = 1;
                        break;
                    case SDL_SCANCODE_E:
                        chip8->keypad[6] = 1;
                        break;
                    case SDL_SCANCODE_A:
                        chip8->keypad[7] = 1;
                        break;
                    case SDL_SCANCODE_S:
                        chip8->keypad[8] = 1;
                        break;
                    case SDL_SCANCODE_D:
                        chip8->keypad[9] = 1;
                        break;
                    case SDL_SCANCODE_Z:
                        chip8->keypad[0xA] = 1;
                        break;
                    case SDL_SCANCODE_C:
                        chip8->keypad[0xB] = 1;
                        break;
                    case SDL_SCANCODE_4:
                        chip8->keypad[0xC] = 1;
                        break;
                    case SDL_SCANCODE_R:
                        chip8->keypad[0xD] = 1;
                        break;
                    case SDL_SCANCODE_F:
                        chip8->keypad[0xE] = 1;
                        break;
                    case SDL_SCANCODE_V:
                        chip8->keypad[0xF] = 1;
                        break;
                }
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_X:
                    chip8->keypad[0] = 0;
                    break;
                case SDL_SCANCODE_1:
                    chip8->keypad[1] = 0;
                    break;
                case SDL_SCANCODE_2:
                    chip8->keypad[2] = 0;
                    break;
                case SDL_SCANCODE_3:
                    chip8->keypad[3] = 0;
                    break;
                case SDL_SCANCODE_Q:
                    chip8->keypad[4] = 0;
                    break;
                case SDL_SCANCODE_W:
                    chip8->keypad[5] = 0;
                    break;
                case SDL_SCANCODE_E:
                    chip8->keypad[6] = 0;
                    break;
                case SDL_SCANCODE_A:
                    chip8->keypad[7] = 0;
                    break;
                case SDL_SCANCODE_S:
                    chip8->keypad[8] = 0;
                    break;
                case SDL_SCANCODE_D:
                    chip8->keypad[9] = 0;
                    break;
                case SDL_SCANCODE_Z:
                    chip8->keypad[0xA] = 0;
                    break;
                case SDL_SCANCODE_C:
                    chip8->keypad[0xB] = 0;
                    break;
                case SDL_SCANCODE_4:
                    chip8->keypad[0xC] = 0;
                    break;
                case SDL_SCANCODE_R:
                    chip8->keypad[0xD] = 0;
                    break;
                case SDL_SCANCODE_F:
                    chip8->keypad[0xE] = 0;
                    break;
                case SDL_SCANCODE_V:
                    chip8->keypad[0xF] = 0;
                    break;
            }
            break;
        }
    }

    return quit;
}

// There are roughly 35 instructions in CHIP-8. Those will be declared here.
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "chip8.h"

void OP_00E0(CHIP8* chip8, uint16_t opcode);                  // CLS - Clear the display.
void OP_00EE(CHIP8* chip8, uint16_t opcode);                  // RET - Return from a subroutine. The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
void OP_1nnn(CHIP8* chip8, uint16_t opcode); // JP addr - Jump to location nnn. The interpreter sets the program counter to nnn.
void OP_2nnn(CHIP8* chip8, uint16_t opcode); // CALL addr - Call subroutine at nnn. The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
void OP_3xkk(CHIP8* chip8, uint16_t opcode); // SE Vx, byte - Skip next instruction if Vx = kk. The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
void OP_4xkk(CHIP8* chip8, uint16_t opcode); // SNE Vx, byte - Skip next instruction if Vx != kk. The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
void OP_5xy0(CHIP8* chip8, uint16_t opcode); // SE Vx, Vy - Skip next instruction if Vx = Vy. The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
void OP_6xkk(CHIP8* chip8, uint16_t opcode); // LD Vx, byte - Set Vx = kk. The interpreter puts the value kk into register Vx.
void OP_7xkk(CHIP8* chip8, uint16_t opcode); // ADD Vx, byte - Set Vx = Vx + kk. Adds the value kk to the value of register Vx, then stores the result in Vx.
void OP_8xy0(CHIP8* chip8, uint16_t opcode); // LD Vx, Vy - Set Vx = Vy. Stores the value of register Vy in register Vx.
void OP_8xy1(CHIP8* chip8, uint16_t opcode); // OR Vx, Vy - Set Vx = Vx OR Vy. Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
void OP_8xy2(CHIP8* chip8, uint16_t opcode); // AND Vx, Vy - Set Vx = Vx AND Vy. Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
void OP_8xy3(CHIP8* chip8, uint16_t opcode); // XOR Vx, Vy - Set Vx = Vx XOR Vy. Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
void OP_8xy4(CHIP8* chip8, uint16_t opcode); // ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry. The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
void OP_8xy5(CHIP8* chip8, uint16_t opcode); // SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow. If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
void OP_8xy6(CHIP8* chip8, uint16_t opcode); // SHR Vx {, Vy} - Set Vx = Vx SHR 1. If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void OP_8xy7(CHIP8* chip8, uint16_t opcode); // SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow. If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
void OP_8xyE(CHIP8* chip8, uint16_t opcode); // SHL Vx {, Vy} - Set Vx = Vx SHL 1. If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
void OP_9xy0(CHIP8* chip8, uint16_t opcode); // SNE Vx, Vy - Skip next instruction if Vx != Vy. The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
void OP_Annn(CHIP8* chip8, uint16_t opcode); // LD I, addr - Set I = nnn. The value of register I is set to nnn.
void OP_Bnnn(CHIP8* chip8, uint16_t opcode); // JP V0, addr - Jump to location nnn + V0. The program counter is set to nnn plus the value of V0.
void OP_Cxkk(CHIP8* chip8, uint16_t opcode); // RND Vx, byte - Set Vx = random byte AND kk. The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx.
void OP_Dxyn(CHIP8* chip8, uint16_t opcode); // DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision. 
                                             /*The interpreter reads n bytes from memory, starting at the address stored in I. 
                                             These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. 
                                             If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
                                             If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.*/
void OP_Ex9E(CHIP8* chip8, uint16_t opcode); // SKP Vx - Skip next instruction if key with the value of Vx is pressed. Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
void OP_ExA1(CHIP8* chip8, uint16_t opcode); // SKNP Vx - Skip next instruction if key with the value of Vx is not pressed. Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
void OP_Fx07(CHIP8* chip8, uint16_t opcode); // LD Vx, DT - Set Vx = delay timer value. The value of DT is placed into Vx.
void OP_Fx0A(CHIP8* chip8, uint16_t opcode); // LD Vx, K - Wait for a key press, store the value of the key in Vx. All execution stops until a key is pressed, then the value of that key is stored in Vx.
void OP_Fx15(CHIP8* chip8, uint16_t opcode); // LD DT, Vx - Set delay timer = Vx. DT is set equal to the value of Vx.
void OP_Fx18(CHIP8* chip8, uint16_t opcode); // LD ST, Vx - Set sound timer = Vx. ST is set equal to the value of Vx.
void OP_Fx1E(CHIP8* chip8, uint16_t opcode); // ADD I, Vx - Set I = I + Vx. The values of I and Vx are added, and the results are stored in I.
void OP_Fx29(CHIP8* chip8, uint16_t opcode); // LD F, Vx - Set I = location of sprite for digit Vx. The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
void OP_Fx33(CHIP8* chip8, uint16_t opcode); // LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2. The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
void OP_Fx55(CHIP8* chip8, uint16_t opcode); // LD [I], Vx - Store registers V0 through Vx in memory starting at location I. The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
void OP_Fx65(CHIP8* chip8, uint16_t opcode); // LD Vx, [I] - Read registers V0 through Vx from memory starting at location I. The interpreter reads values from memory starting at location I into registers V0 through Vx.

#endif

#include "instructions.h"

void OP_00E0(CHIP8* chip8, uint16_t opcode){
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        chip8->display[i] = 0;
    }
}

void OP_00EE(CHIP8* chip8, uint16_t opcode){
    if (chip8->stkptr > 0)
    {
        chip8->PC = chip8->stack[chip8->stkptr];
        chip8->stkptr = chip8->stkptr - 1;
    }
}

void OP_1nnn(CHIP8* chip8, uint16_t opcode){
    // Here nnn is the value that PC jumps to
    uint16_t NewAddress = 0x0FFF & opcode;
    chip8->PC = NewAddress;
}

void OP_2nnn(CHIP8* chip8, uint16_t opcode){
    // Here, PC's value will be stored in stack and PC will be updated with new address
    // This is basically CALL (going to perform subroutine)
    uint16_t NewAddress = 0x0FFF & opcode;
    chip8->stkptr++;
    chip8->stack[chip8->stkptr] = chip8->PC;
    chip8->PC = NewAddress;
}

void OP_3xkk(CHIP8* chip8, uint16_t opcode){
    // Here, if Vx = kk, skip next instruction
    uint8_t kk = 0x00FF & opcode;
    uint8_t x = (0x0F00 & opcode) >> 8;

    if (chip8->registers[x] == kk)
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_4xkk(CHIP8* chip8, uint16_t opcode){
    // Here, if Vx != kk, skip next instruction
    uint8_t kk = 0x00FF & opcode;
    uint8_t x = (0x0F00 & opcode) >> 8;

    if (chip8->registers[x] != kk)
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_5xy0(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    if(chip8->registers[x] == chip8->registers[y]){
        chip8->PC = chip8->PC + 2;
    }
}

void OP_6xkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = 0x00FF && opcode;

    chip8->registers[x] = kk;
}

void OP_7xkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = 0x00FF && opcode;

    chip8->registers[x] = chip8->registers[x] + kk;
}

void OP_8xy0(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    chip8->registers[x] = chip8->registers[y];
}

void OP_8xy1(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    chip8->registers[x] = chip8->registers[x] | chip8->registers[y];
}

void OP_8xy2(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    chip8->registers[x] = chip8->registers[x] & chip8->registers[y];
}

void OP_8xy3(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    chip8->registers[x] = chip8->registers[x] ^ chip8->registers[y];
}

void OP_8xy4(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    uint16_t sum = chip8->registers[x] + chip8->registers[y];

    if(sum > 255){
        chip8->registers[0xF] = 1;
    }
    else
    {
        chip8->registers[0xF] = 0;
    }

    chip8->registers[x] = sum & 0xFF;
}

void OP_8xy5(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    if(chip8->registers[x] > chip8->registers[y]){
        chip8->registers[0xF] = 1;
    }
    else{
        chip8->registers[0xF] = 0;
    }

    chip8->registers[x] = chip8->registers[x] - chip8->registers[y];
}

void OP_8xy6(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    if((chip8->registers[x] % 2) == 1){
        chip8->registers[0xF] = 1;
    }else{
        chip8->registers[0xF] = 0;
    }

    chip8->registers[x] = chip8->registers[x]/2;
}

void OP_8xy7(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    if(chip8->registers[y] > chip8->registers[x]){
        chip8->registers[0xF] = 1;
    }
    else{
        chip8->registers[0xF] = 0;
    }

    chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
}

void OP_8xyE(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    if ((chip8->registers[x] & 128) == 0)
    {
        chip8->registers[0xF] = 0;
    }
    else{
        chip8->registers[0xF] = 1;
    }
    
    chip8->registers[x] = chip8->registers[x] * 2;
}

void OP_9xy0(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    if (chip8->registers[x] != chip8->registers[y])
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_Annn(CHIP8* chip8, uint16_t opcode){
    uint16_t nnn = 0x0FFF & opcode;
    chip8->IndexRegister = nnn;
}

void OP_Bnnn(CHIP8* chip8, uint16_t opcode){
    uint16_t nnn = 0x0FFF & opcode;
    uint16_t address = chip8->registers[0] + nnn;
    chip8->PC = address;
}

void OP_Cxkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = (0x00FF & opcode);

    chip8->registers[x] = GenerateRandomByte(kk);
}

void OP_Dxyn(CHIP8* chip8, uint16_t opcode){
    // Extract Vx, Vy, and height (nibble)
    uint8_t x = chip8->registers[(opcode & 0x0F00) >> 8];
    uint8_t y = chip8->registers[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    // Reset collision flag (VF)
    chip8->registers[0xF] = 0;

    // Loop through the height of the sprite
    for (uint8_t row = 0; row < height; ++row) {
        uint8_t spriteByte = chip8->memory[chip8->IndexRegister + row]; // Fetch sprite row from memory

        // Loop through each bit in the sprite byte
        for (uint8_t col = 0; col < 8; ++col) {
            // Calculate the x and y coordinates (with wrapping)
            uint8_t pixelX = (x + col) % DISPLAY_WIDTH;
            uint8_t pixelY = (y + row) % DISPLAY_HEIGHT;

            // Determine the current bit of the sprite (1 or 0)
            uint8_t spritePixel = (spriteByte >> (7 - col)) & 1;

            // Pointer to the pixel in the display buffer
            uint32_t* displayPixel = &chip8->display[pixelY * DISPLAY_WIDTH + pixelX];

            // Collision detection
            if (spritePixel && (*displayPixel & 1)) {
                chip8->registers[0xF] = 1; // Collision occurred
            }

            // XOR the display pixel with the sprite pixel
            *displayPixel ^= spritePixel;
        }
    }
}

void OP_Ex9E(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t key = chip8->registers[x];

    if (chip8->keypad[key])
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_ExA1(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t key = chip8->registers[x];

    if (!chip8->keypad[key])
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_Fx07(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    chip8->registers[x] = chip8->DelayTimer;
}

void OP_Fx0A(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    if (chip8->keypad[0]){
        chip8->registers[x] = 0;
    }
    else if (chip8->keypad[1]){
        chip8->registers[x] = 1;
    }
    else if (chip8->keypad[2]){
        chip8->registers[x] = 2;
    }
    else if (chip8->keypad[3]){
        chip8->registers[x] = 3;
    }
    else if (chip8->keypad[4]){
        chip8->registers[x] = 4;
    }
    else if (chip8->keypad[5]){
        chip8->registers[x] = 5;
    }
    else if (chip8->keypad[6]){
        chip8->registers[x] = 6;
    }
    else if (chip8->keypad[7]){
        chip8->registers[x] = 7;
    }
    else if (chip8->keypad[8]){
        chip8->registers[x] = 8;
    }
    else if (chip8->keypad[9]){
        chip8->registers[x] = 9;
    }
    else if (chip8->keypad[10]){
        chip8->registers[x] = 10;
    }
    else if (chip8->keypad[11]){
        chip8->registers[x] = 11;
    }
    else if (chip8->keypad[12]){
        chip8->registers[x] = 12;
    }
    else if (chip8->keypad[13]){
        chip8->registers[x] = 13;
    }
    else if (chip8->keypad[14]){
        chip8->registers[x] = 14;
    }
    else if (chip8->keypad[15]){
        chip8->registers[x] = 15;
    }
    else{
        chip8->PC = chip8->PC - 2;
    }
}

void OP_Fx15(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    chip8->DelayTimer = chip8->registers[x];
}

void OP_Fx18(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    chip8->SoundTimer = chip8->registers[x];
}

void OP_Fx1E(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    chip8->IndexRegister = chip8->IndexRegister + chip8->registers[x];
}

void OP_Fx29(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    uint8_t digit = chip8->registers[x];

    chip8->IndexRegister = FONTSET_START_ADDRESS + (5 * digit); // Setting the address of the digit to index register
}

void OP_Fx33(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t num = chip8->registers[x];
    uint16_t index = chip8->IndexRegister;

    for (int i = 0; i < 3; i++)
    {
        uint8_t alpha = num%10;
        chip8->memory[index + i] = alpha;
        num = num/10;
    }
}

void OP_Fx55(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint16_t addr = chip8->IndexRegister;

    for (uint8_t i = 0; i < x; i++)
    {
        chip8->memory[addr + i] = chip8->registers[i];
    }
}

void OP_Fx65(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint16_t addr = chip8->IndexRegister;

    for (uint8_t i = 0; i < x; i++)
    {
        chip8->registers[i] = chip8->memory[addr + i];
    }
}

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
    const char* romFilename = "test_opcode.ch8";

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create texture for displaying the screen
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, DISPLAY_WIDTH, DISPLAY_HEIGHT);
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
