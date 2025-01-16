#include "instructions.h"

// Define DEBUG to enable debug logs
#define DEBUG

#ifdef DEBUG
    #define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define LOG(fmt, ...)
#endif

void OP_00E0(CHIP8* chip8, uint16_t opcode){
    LOG("Executing 00E0: Clear Screen\n");
    for (int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
    {
        chip8->display[i] = 0;
    }
}

void OP_00EE(CHIP8* chip8, uint16_t opcode){
    LOG("Executing 00EE: Return from subroutine\n");
    if (chip8->stkptr > 0)
    {   
        chip8->stkptr = chip8->stkptr - 1;
        chip8->PC = chip8->stack[chip8->stkptr];
    }
}

void OP_1nnn(CHIP8* chip8, uint16_t opcode){
    // Here nnn is the value that PC jumps to
    uint16_t NewAddress = 0x0FFF & opcode;
    LOG("Executing 1nnn: Jump to 0x%04X\n", NewAddress);
    chip8->PC = NewAddress;
}

void OP_2nnn(CHIP8* chip8, uint16_t opcode){
    // Here, PC's value will be stored in stack and PC will be updated with new address
    // This is basically CALL (going to perform subroutine)
    uint16_t NewAddress = 0x0FFF & opcode;
    LOG("Executing 2nnn: Call subroutine at 0x%04X\n", NewAddress);
    chip8->stkptr++;
    chip8->stack[chip8->stkptr] = chip8->PC;
    chip8->PC = NewAddress;
}

void OP_3xkk(CHIP8* chip8, uint16_t opcode){
    // Here, if Vx = kk, skip next instruction
    uint8_t kk = 0x00FF & opcode;
    uint8_t x = (0x0F00 & opcode) >> 8;
    
    LOG("Executing 3xkk: Skip next if V[%d] (0x%02X) == 0x%02X\n", x, chip8->registers[x], kk);
    if (chip8->registers[x] == kk) {
        chip8->PC += 2;
        LOG("PC incremented to 0x%04X\n", chip8->PC);
    }
}

void OP_4xkk(CHIP8* chip8, uint16_t opcode){
    // Here, if Vx != kk, skip next instruction
    uint8_t kk = 0x00FF & opcode;
    uint8_t x = (0x0F00 & opcode) >> 8;

    LOG("Executing 4xkk: Skip next if V[%d] (0x%02X) != 0x%02X\n", x, chip8->registers[x], kk);
    if (chip8->registers[x] != kk) {
        chip8->PC += 2;
        LOG("PC incremented to 0x%04X\n", chip8->PC);
    }
}

void OP_5xy0(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    LOG("Executing 5xy0: Skip next if V[%d] (0x%02X) == V[%d] (0x%02X)\n", x, chip8->registers[x], y, chip8->registers[y]);
    if (chip8->registers[x] == chip8->registers[y]) {
        chip8->PC += 2;
        LOG("PC incremented to 0x%04X\n", chip8->PC);
    }
}

void OP_6xkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = 0x00FF && opcode;

    LOG("Executing 6xkk: Set V[%d] = 0x%02X\n", x, kk); 
    chip8->registers[x] = kk;
}

void OP_7xkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = 0x00FF && opcode;

    LOG("Executing 7xkk: Add 0x%02X to V[%d] (0x%02X)\n", kk, x, chip8->registers[x]);
    chip8->registers[x] = chip8->registers[x] + kk;
}

void OP_8xy0(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    LOG("Executing 8xy0: Set V[%d] = V[%d] (0x%02X)\n", x, y, chip8->registers[y]);
    chip8->registers[x] = chip8->registers[y];
}

void OP_8xy1(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    LOG("Executing 8xy1: Set V[%d] = V[%d] OR V[%d] (0x%02X | 0x%02X)\n", x, x, y, chip8->registers[x], chip8->registers[y]);
    chip8->registers[x] = chip8->registers[x] | chip8->registers[y];
}

void OP_8xy2(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    LOG("Executing 8xy2: Set V[%d] = V[%d] AND V[%d] (0x%02X & 0x%02X)\n", x, x, y, chip8->registers[x], chip8->registers[y]);
    chip8->registers[x] = chip8->registers[x] & chip8->registers[y];
}

void OP_8xy3(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;
    
    LOG("Executing 8xy3: Set V[%d] = V[%d] XOR V[%d] (0x%02X ^ 0x%02X)\n", x, x, y, chip8->registers[x], chip8->registers[y]);
    chip8->registers[x] = chip8->registers[x] ^ chip8->registers[y];
}

void OP_8xy4(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t y = (0x00F0 & opcode) >> 4;

    uint16_t sum = chip8->registers[x] + chip8->registers[y];

    LOG("Executing 8xy4: Add V[%d] (0x%02X) + V[%d] (0x%02X), Carry=%d\n", x, chip8->registers[x], y, chip8->registers[y], sum > 255);
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

    LOG("Executing 8xy5: Subtract V[%d] (0x%02X) - V[%d] (0x%02X)\n", x, chip8->registers[x], y, chip8->registers[y]);

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

    LOG("Executing 8xy6: Shift V[%d] (0x%02X) >> 1\n", x, chip8->registers[x]);

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

    LOG("Executing 8xy7: Subtract V[%d] (0x%02X) - V[%d] (0x%02X)\n", y, chip8->registers[y], x, chip8->registers[x]);

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

    LOG("Executing 8xyE: Shift V[%d] (0x%02X) << 1\n", x, chip8->registers[x]);

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

    LOG("Executing 9xy0: Skip if V[%d] (0x%02X) != V[%d] (0x%02X)\n", x, chip8->registers[x], y, chip8->registers[y]);

    if (chip8->registers[x] != chip8->registers[y])
    {
        chip8->PC = chip8->PC + 2;
    }
}

void OP_Annn(CHIP8* chip8, uint16_t opcode){
    uint16_t nnn = 0x0FFF & opcode;
    LOG("Executing Annn: Set I = 0x%04X\n", nnn);
    chip8->IndexRegister = nnn;
}

void OP_Bnnn(CHIP8* chip8, uint16_t opcode){
    uint16_t nnn = 0x0FFF & opcode;
    LOG("Executing Bnnn: Jump to 0x%04X + V[0] (0x%02X)\n", nnn, chip8->registers[0]);
    chip8->PC = nnn + chip8->registers[0];
    LOG("New PC: 0x%04X\n", chip8->PC);
}

void OP_Cxkk(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t kk = (0x00FF & opcode);
    uint8_t randomByte = GenerateRandomByte(kk);
    LOG("Executing Cxkk: Set V[%d] = Random Byte (0x%02X) & 0x%02X\n", x, randomByte, kk);

    chip8->registers[x] = randomByte;
    LOG("Result: V[%d] = 0x%02X\n", x, chip8->registers[x]);
}

void OP_Dxyn(CHIP8* chip8, uint16_t opcode){
    // Extract Vx, Vy, and height (nibble)
    uint8_t x = chip8->registers[(opcode & 0x0F00) >> 8];
    uint8_t y = chip8->registers[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    LOG("Executing Dxyn: Draw sprite at V[%d]=%d, V[%d]=%d, height=%d\n", 
        (opcode & 0x0F00) >> 8, x, (opcode & 0x00F0) >> 4, y, height);
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

            // // Determine the current bit of the sprite (1 or 0)
            // uint8_t spritePixel = (spriteByte >> (7 - col)) & 1;

            // // Pointer to the pixel in the display buffer
            // uint32_t* displayPixel = &chip8->display[pixelY * DISPLAY_WIDTH + pixelX];

            // // Collision detection
            // if (spritePixel && (*displayPixel & 1)) {
            //     chip8->registers[0xF] = 1; // Collision occurred
            // }

            // // XOR the display pixel with the sprite pixel
            // *displayPixel ^= spritePixel;

            uint8_t spritePixel = (spriteByte >> (7 - col)) & 1;
            if (spritePixel && chip8->display[pixelY * DISPLAY_WIDTH + pixelX]) {
                chip8->registers[0xF] = 1;  // Set collision flag
            }
            chip8->display[pixelY * DISPLAY_WIDTH + pixelX] ^= spritePixel;

        }
    }
}

void OP_Ex9E(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t key = chip8->registers[x];

    LOG("Executing Ex9E: Skip if key 0x%02X is pressed\n", key);

    if (chip8->keypad[key]) {
        chip8->PC += 2;
        LOG("Key pressed. PC incremented to 0x%04X\n", chip8->PC);
    } else {
        LOG("Key not pressed. PC unchanged.\n");
    }
}

void OP_ExA1(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;
    uint8_t key = chip8->registers[x];

    LOG("Executing ExA1: Skip if key 0x%02X is NOT pressed\n", key);

    if (!chip8->keypad[key]) {
        chip8->PC += 2;
        LOG("Key not pressed. PC incremented to 0x%04X\n", chip8->PC);
    } else {
        LOG("Key pressed. PC unchanged.\n");
    }
}

void OP_Fx07(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    LOG("Executing Fx07: Set V[%d] = DelayTimer (0x%02X)\n", x, chip8->DelayTimer);
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

    LOG("Executing Fx15: Set DelayTimer = V[%d] (0x%02X)\n", x, chip8->registers[x]);
    chip8->DelayTimer = chip8->registers[x];
    LOG("New DelayTimer: 0x%02X\n", chip8->DelayTimer);
}

void OP_Fx18(CHIP8* chip8, uint16_t opcode){
    uint8_t x = (0x0F00 & opcode) >> 8;

    LOG("Executing Fx18: Set SoundTimer = V[%d] (0x%02X)\n", x, chip8->registers[x]);
    chip8->SoundTimer = chip8->registers[x];
    LOG("New SoundTimer: 0x%02X\n", chip8->SoundTimer);
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

    LOG("Executing Fx33: Store BCD of V[%d] (0x%02X) at I, I+1, I+2\n", x, num);

    chip8->memory[chip8->IndexRegister + 2] = num % 10;
    chip8->memory[chip8->IndexRegister + 1] = (num / 10) % 10;
    chip8->memory[chip8->IndexRegister] = num / 100;
    LOG("Memory[I]: 0x%02X, Memory[I+1]: 0x%02X, Memory[I+2]: 0x%02X\n",
        chip8->memory[chip8->IndexRegister], chip8->memory[chip8->IndexRegister + 1], chip8->memory[chip8->IndexRegister + 2]);
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