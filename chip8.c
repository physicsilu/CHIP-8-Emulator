#include "chip8.h"

#define unknown_opcode(op) \
    do \
    { \
        fprintf(stderr, "Unknown opcode: 0x%x\n", op); \
        fprintf(stderr, "kk: 0x%02x\n", kk); \
        exit(42); \
    } while (0)
    
#ifdef DEBUG
#define p(...) printf(__VA_ARGS__);
#else
#define p(...)
#endif

#define IS_BIT_SET(byte, bit) (((0x80 >> (bit)) & (byte)) != 0x0)

#define FONTSET_ADDRESS 0x00
#define FONTSET_BYTES_PER_CHAR 5
unsigned char fontset[80] =
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

uint16_t    opcode;
uint8_t     memory[MEM_SIZE];
uint8_t     registers[16];
uint16_t    IndexRegister;
uint16_t    PC;
uint8_t     gfx[GFX_ROWS][GFX_COLS];
uint8_t     DelayTimer;
uint8_t     SoundTimer;
uint16_t    stack[STACK_SIZE];
uint16_t    stkptr;
uint8_t     key[KEYPAD_SIZE];
bool        chip8_draw_flag;

static inline uint8_t randbyte(){
    return (rand() % 256);
}

/*
inline:
A hint to the compiler to replace the function call with the actual code of the function at compile time.
Reduces the overhead of a function call but may increase binary size if the function is used in many places.
*/

// This is basically Dxyn instruction

void draw_sprite(uint8_t x, uint8_t y, uint8_t n){
    unsigned row = y, col = x;
    unsigned byte_index;
    unsigned bit_index;

    // In C (and C++), when you use the unsigned keyword without explicitly specifying the type, it is implicitly treated as unsigned int

    // setting collision flag to 0
    registers[0xF] = 0;

    for (byte_index = 0; byte_index < n; byte_index++)
    {
        uint8_t byte = memory[IndexRegister + byte_index];

        for (bit_index = 0; bit_index < 8; bit_index++)
        {
            uint8_t bit = (byte >> bit_index) & 0x1; // bit value in sprite
            uint8_t *pixelp = &gfx[(row + byte_index) % GFX_ROWS][(col + (7 - bit_index)) % GFX_COLS];

            // Collision
            if (bit == 1 && *pixelp == 1)
            {
                registers[0xF] = 1;
            }

            // Draw pixel
            *pixelp = *pixelp ^ bit;
            
        }
        
    }
}

static void debug_draw(){
    int x,y;
    for (y = 0; y < GFX_ROWS; y++)
    {
        for (x = 0; x < GFX_COLS; x++)
        {
            if (gfx[y][x] == 0)
            {
                printf("0");
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }
    
    printf("\n");
}

static void print_state(){
    printf("------------------------------------------\n");
    printf("\n");

    printf("V0: 0x%02x  V4: 0x%02x  V8: 0x%02x  VC: 0x%02x\n",
            registers[0], registers[4], registers[8], registers[12]);

    printf("V1: 0x%02x  V5: 0x%02x  V9: 0x%02x  VD: 0x%02x\n",
            registers[1], registers[5], registers[9], registers[13]);

    printf("V2: 0x%02x  V6: 0x%02x  VA: 0x%02x  VE: 0x%02x\n",
            registers[2], registers[6], registers[10], registers[14]);

    printf("V3: 0x%02x  V7: 0x%02x  VB: 0x%02x  VF: 0x%02x\n",
            registers[3], registers[7], registers[11], registers[15]);

    printf("\n");
    printf("PC: 0x%04x\n", PC);
    printf("\n");
    printf("\n");
}

void InitializeChip8(){
    PC              = 0x200;
    opcode          = 0;
    IndexRegister   = 0;
    stkptr          = 0;

    memset(memory, 0, sizeof(uint8_t)*MEM_SIZE);
    memset(registers, 0, sizeof(uint8_t)*16);
    memset(gfx,    0, sizeof(uint8_t)  * GFX_SIZE);
    memset(stack,  0, sizeof(uint16_t) * STACK_SIZE);
    memset(key,    0, sizeof(uint8_t)  * KEYPAD_SIZE);

    for (int i = 0; i < 80; i++)
    {
        memory[FONTSET_ADDRESS + i] = fontset[i];
    }

    chip8_draw_flag = true;
    DelayTimer = 0;
    SoundTimer = 0;
    srand(time(NULL));
}

void LoadGame(char* game){
    FILE* fptr;

    fptr = fopen(game, "rb");

    if (fptr == NULL)
    {
        fprintf(stderr, "Unable to open game: %s\n", game);
        exit(42);
    }

    fread(&memory[0x200], 1, MAX_GAME_SIZE, fptr);

    fclose(fptr);    
}

// The detailed explanations of each opcode functionalities are there in old file
void EmulateCycle(){
    int i;
    uint8_t x, y, n;
    uint8_t kk;
    uint16_t nnn;

    // Instruction fetch
    opcode = memory[PC] << 8 | memory[PC + 1];
    x   = (opcode >> 8) & 0x000F;
    y   = (opcode >> 4) & 0x000F;
    n   = opcode & 0x000F;
    kk  = opcode & 0x00FF;
    nnn = opcode & 0x0FFF;

    #ifdef DEBUG 
    printf("PC: 0x%04x Op: 0x%04x\n", PC, opcode);
    #endif

    // Instruction decode and execute
    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch(kk){
                case 0x00E0:
                    p("Clear Screen\n");
                    memset(gfx, 0, sizeof(uint8_t)*GFX_SIZE);
                    chip8_draw_flag = true;
                    PC = PC + 2;
                    break;
                case 0x00EE:
                    p("Return from subroutine\n");
                    PC = stack[--stkptr];
                    break;
                default:
                    unknown_opcode(opcode);
            }
        break;

        case 0x1000:
            p("Jump to address 0x%x\n", nnn);
            PC = nnn;
            break;
        
        case 0x2000:
            p("Call subroutine at 0x%04X\n", nnn);
            stack[stkptr++] = PC + 2;
            PC = nnn;
            break;
        
        case 0x3000:
            p("Skip next instruction if 0x%x == 0x%x\n", registers[x], kk);
            PC += (registers[x] == kk) ? 4 : 2;
            break;
        
        case 0x4000:
            p("Skip next instruction if 0x%x != 0x%x\n", registers[x], kk);
            PC += (registers[x] != kk) ? 4 : 2;
            break;

        case 0x5000:
            p("Skip next instruction if 0x%x == 0x%x\n", registers[x], registers[y]);
            PC += (registers[x] == registers[y]) ? 4 : 2;
            break;

        case 0x6000:
            p("Set V[0x%x] to 0x%x\n", x, kk);
            registers[x] = kk;
            PC += 2;
            break;

        case 0x7000:
            p("Set V[0x%d] to V[0x%d] + 0x%x\n", x, x, kk);
            registers[x] += kk;
            PC += 2;
            break;

        case 0x8000:
            switch(n){
                case 0x0:
                    p("V[0x%x] = V[0x%x] = 0x%x\n", x, y, registers[y]);
                    registers[x] = registers[y];
                    break;
                
                case 0x1:
                    p("V[0x%x] |= V[0x%x] = 0x%x\n", x, y, registers[y]);
                    registers[x] = registers[x] | registers[y];
                    break;

                case 0x2:
                    p("V[0x%x] &= V[0x%x] = 0x%x\n", x, y, registers[y]);
                    registers[x] = registers[x] & registers[y];
                    break;

                case 0x3:
                    p("V[0x%x] ^= V[0x%x] = 0x%x\n", x, y, registers[y]);
                    registers[x] = registers[x] ^ registers[y];
                    break;

                case 0x4:
                    p("Add V[%d] (0x%02X) + V[%d] (0x%02X)", x, registers[x], y, registers[y]);
                    registers[0xF] = ((int) registers[x] + (int) registers[y]) > 255 ? 1 : 0;
                    registers[x] = registers[x] + registers[y];
                    break;

                case 0x5:
                    p("Subtract V[%d] (0x%02X) - V[%d] (0x%02X)", x, registers[x], y, registers[y]);
                    registers[0xF] = (registers[x] > registers[y]) ? 1 : 0;
                    registers[x] = registers[x] - registers[y];
                    break;

                case 0x6:
                    p("V[0x%x] = V[0x%x] >> 1 = 0x%x >> 1\n", x, x, registers[x]);
                    registers[0xF] = registers[x] & 0x1;
                    registers[x] = (registers[x] >> 1);
                    break;

                case 0x7:
                    p("Subtract V[%d] (0x%02X) - V[%d] (0x%02X)\n", y, registers[y], x, registers[x]);
                    registers[0xF] = (registers[y] > registers[x]) ? 1 : 0;
                    registers[x] = registers[y] - registers[x];
                    break;
                
                case 0xE:
                    p("V[0x%x] = V[0x%x] << 1 = 0x%x << 1\n", x, x, registers[x]);
                    registers[0xF] = (registers[x] >> 7) & 0x1;
                    registers[x] = (registers[x] << 1);
                    break;

                default:
                    unknown_opcode(opcode);
            }
            PC += 2;
            break;
        
        case 0x9000:
            switch(n){
                case 0x0:
                    p("Skip next instruction if 0x%x != 0x%x\n", registers[x], registers[y]);
                    PC += (registers[x] != registers[y]) ? 4 : 2;
                    break;
                default:
                    unknown_opcode(opcode); 
            }
            break;

        case 0xA000:
            p("Set I to 0x%x\n", nnn);
            IndexRegister = nnn;
            PC += 2;
            break;
        
        case 0xB000:
            p("Jump to 0x%x + V[0] (0x%x)\n", nnn, registers[0]);
            PC = nnn + registers[0];
            break;

        case 0xC000:
            p("V[0x%x] = random byte\n", x);
            registers[x] = randbyte() & kk;
            PC += 2;
            break;

        case 0xD000:
            p("Draw sprite at (V[0x%x], V[0x%x]) = (0x%x, 0x%x) of height %d", 
               x, y, registers[x], registers[y], n);
            draw_sprite(registers[x], registers[y], n);
            PC += 2;
            chip8_draw_flag = true;
            break;

        case 0xE000:
            switch(kk){
                case 0x9E:
                    p("Skip next instruction if key[%d] is pressed\n", x);
                    PC += (key[registers[x]]) ? 4 : 2;
                    break;

                case 0xA1:
                    p("Skip next instruction if key[%d] is NOT pressed\n", x);
                    PC += (!key[registers[x]]) ? 4 : 2;
                    break;

                default:
                    unknown_opcode(opcode);
            }
            break;
        
        case 0xF000:
            switch(kk){
                case 0x07:
                    p("V[0x%x] = delay timer = %d\n", x, DelayTimer);
                    registers[x] = DelayTimer;
                    PC += 2;
                    break;
                
                case 0x0A:
                    i = 0;
                    p("Wait for key instruction\n");
                    while (true)
                    {
                        for (i = 0; i < KEYPAD_SIZE; i++)
                        {
                            if(key[i]){
                                registers[x] = i;
                                goto got_key_press;
                            }
                        }
                    }
                    got_key_press:
                        PC += 2;
                        break;
                
                case 0x15:
                    p("delay timer = V[0x%x] = %d\n", x, registers[x]);
                    DelayTimer = registers[x];
                    PC += 2;
                    break;

                case 0x18:
                    p("sound timer = V[0x%x] = %d\n", x, registers[x]);
                    SoundTimer = registers[x];
                    PC += 2;
                    break;

                case 0x1E:
                    p("I = I + V[0x%x] = 0x%x + 0x%x\n", x, IndexRegister, registers[x]);
                    registers[0xF] = (IndexRegister + registers[x] > 0xFFF) ? 1 : 0;
                    IndexRegister = IndexRegister + registers[x];
                    PC += 2;
                    break;

                case 0x29:
                    p("I = location of font for character V[0x%x] = 0x%x\n", x, registers[x]);
                    IndexRegister = FONTSET_BYTES_PER_CHAR * registers[x];
                    PC += 2;
                    break;

                case 0x33:
                    p("Store BCD for %d starting at address 0x%x\n", registers[x], IndexRegister);
                    memory[IndexRegister]   = (registers[x] % 1000) / 100; // hundred's digit
                    memory[IndexRegister+1] = (registers[x] % 100) / 10;   // ten's digit
                    memory[IndexRegister+2] = (registers[x] % 10);         // one's digit
                    PC += 2;
                    break;

                case 0x55:
                    p("Copy sprite from registers 0 to 0x%x into memory at address 0x%x\n", x, IndexRegister);
                    for (i = 0; i <= x; i++) { 
                        memory[IndexRegister + i] = registers[i]; 
                    }
                    IndexRegister += x + 1;
                    PC += 2;
                    break;

                case 0x65:
                    p("Copy sprite from memory at address 0x%x into registers 0 to 0x%x\n", x, IndexRegister);
                    for (i = 0; i <= x; i++) { 
                        registers[i] = memory[IndexRegister + i]; 
                    }
                    IndexRegister += x + 1;
                    PC += 2;
                    break;

                default:
                    unknown_opcode(opcode);
            }
            break;
        
        default:
            unknown_opcode(opcode);
    }

    #ifdef DEBUG
        print_state()
    #endif
}

void Tick(){
    if (DelayTimer > 0)
    {
        DelayTimer--;
    }
    if (SoundTimer > 0)
    {
        SoundTimer--;
        if (SoundTimer == 0)
        {
            p("BEEP!\n");
        }
    }
}

