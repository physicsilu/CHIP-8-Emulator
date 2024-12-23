// We do this ifndef stuff no not include the header file multiple times. This is like #pragma once 

#ifndef CHIP_8
#define CHIP_8

#include<stdint.h>

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16

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

void InitializeChip8(CHIP8* chip8);
void LoadROM(CHIP8* chip8 ,const char* filename);
#endif 