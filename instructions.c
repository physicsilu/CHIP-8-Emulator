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