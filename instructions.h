// There are roughly 35 instructions in CHIP-8. Those will be declared here.
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "chip8.h"

void OP_00E0(CHIP8* chip8);                  // CLS - Clear the display.
void OP_00EE(CHIP8* chip8);                  // RET - Return from a subroutine. The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
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
                                             /*The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.*/
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