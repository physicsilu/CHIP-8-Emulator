// There are roughly 35 instructions in CHIP-8. Those will be declared here.
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "chip8.h"

void OP_00E0(CHIP8* chip8); // CLS - Clear the display.
void OP_00EE(CHIP8* chip8); // RET - Return from a subroutine. The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
void OP_1nnn(CHIP8* chip8, uint16_t opcode); // JP addr - Jump to location nnn. The interpreter sets the program counter to nnn.
void OP_2nnn(CHIP8* chip8, uint16_t opcode); // CALL addr - Call subroutine at nnn. The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
void OP_3xkk(CHIP8* chip8, uint16_t opcode); // SE Vx, byte - Skip next instruction if Vx = kk. The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
void OP_4xkk(CHIP8* chip8, uint16_t opcode); // SNE Vx, byte - Skip next instruction if Vx != kk. The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.

#endif