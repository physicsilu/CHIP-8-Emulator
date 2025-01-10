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