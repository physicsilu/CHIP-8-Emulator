#include "instructions.h"

void OP_00E0(CHIP8* chip8){
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        chip8->display[i] = 0;
    }
}

void OP_00EE(CHIP8* chip8){
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
    }else
    {
        chip8->PC++;
    }
}

void OP_4xkk(CHIP8* chip8, uint16_t opcode){
    // Here, if Vx != kk, skip next instruction
    uint8_t kk = 0x00FF & opcode;
    uint8_t x = (0x0F00 & opcode) >> 8;

    if (chip8->registers[x] != kk)
    {
        chip8->PC = chip8->PC + 2;
    }else
    {
        chip8->PC++;
    }
}