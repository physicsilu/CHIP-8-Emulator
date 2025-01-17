#ifndef CHIP_8
#define CHIP_8

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MEM_SIZE 4096
#define GFX_ROWS 32
#define GFX_COLS 64
#define GFX_SIZE (GFX_ROWS* GFX_COLS)
#define STACK_SIZE 16
#define KEYPAD_SIZE 16

#define GFX_INDEX(row, col) ((row)*GFX_COLS + (col))

#define MAX_GAME_SIZE (0x1000 - 0x200)

void InitializeChip8();
void LoadGame(char* game);
void EmulateCycle();
void Tick();

#endif 