// In this file the function definitions are included
#include"chip8.h"
#include"instructions.h"

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

