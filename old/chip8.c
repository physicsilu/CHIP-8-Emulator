// In this file the function definitions are included
#include"chip8.h"
#include"instructions.h"

InstructionHandler functionTable[0x10]; // Defining functionTable
InstructionHandler table8[0x10];        // Defining table8
InstructionHandler tableE[0xFF];        // Defining tableE
InstructionHandler tableF[0xFF];        // Defining table

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
    
    InitializeFunctionTable();
    InitializeTable8();
    InitializeTableE();
    InitializeTableF();
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

    

    // Execute
    uint8_t category = (opcode & 0xF000) >> 12; // First Nibble

    if (functionTable[category] != NULL){
        functionTable[category](chip8, opcode);

        // Incrementing PC
        chip8->PC = chip8->PC + 2;
    }
    else{
        // Handling special cases

        switch (category)
        {
        case 0x8: {
            uint8_t subcode = opcode & 0x000F; // Last nibble
            if (table8[subcode] != NULL){
                table8[subcode](chip8, opcode);

                // Incrementing PC
                chip8->PC = chip8->PC + 2;
            }
            else{
                printf("Unknown instruction: 0x%04X\n", opcode);
                // Incrementing PC
                chip8->PC = chip8->PC + 2;
            }
            
            break;
        }
        
        case 0xE: {
            uint8_t subcode = opcode & 0x00FF; // Last 8 bits
            if (tableE[subcode] != NULL){
                tableE[subcode](chip8, opcode);

                // Incrementing PC
                chip8->PC = chip8->PC + 2;  
            }else{
                printf("Unknown instruction: 0x%04X\n", opcode);
                // Incrementing PC
                chip8->PC = chip8->PC + 2;
            }
            
            break;
        }

        case 0xF: {
            uint8_t subcode = opcode & 0x00FF; // Last 8 bits
            if (tableF[subcode] != NULL){
                tableF[subcode](chip8, opcode);
                // Incrementing PC
                chip8->PC = chip8->PC + 2;
            }else{
                printf("Unknown instruction: 0x%04X\n", opcode);
                // Incrementing PC
                chip8->PC = chip8->PC + 2;
            }
            
            break;
        }

        default:{
            printf("Unknown instruction: 0x%04X\n", opcode);
            // Incrementing PC
            chip8->PC = chip8->PC + 2;
        }
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

bool ProcessInput(CHIP8* chip8) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true; // This will trigger when the user closes the window
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                quit = true; // This will trigger when the user presses ESC
            } else {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_X:
                        chip8->keypad[0] = 1;
                        break;
                    case SDL_SCANCODE_1:
                        chip8->keypad[1] = 1;
                        break;
                    case SDL_SCANCODE_2:
                        chip8->keypad[2] = 1;
                        break;
                    case SDL_SCANCODE_3:
                        chip8->keypad[3] = 1;
                        break;
                    case SDL_SCANCODE_Q:
                        chip8->keypad[4] = 1;
                        break;
                    case SDL_SCANCODE_W:
                        chip8->keypad[5] = 1;
                        break;
                    case SDL_SCANCODE_E:
                        chip8->keypad[6] = 1;
                        break;
                    case SDL_SCANCODE_A:
                        chip8->keypad[7] = 1;
                        break;
                    case SDL_SCANCODE_S:
                        chip8->keypad[8] = 1;
                        break;
                    case SDL_SCANCODE_D:
                        chip8->keypad[9] = 1;
                        break;
                    case SDL_SCANCODE_Z:
                        chip8->keypad[0xA] = 1;
                        break;
                    case SDL_SCANCODE_C:
                        chip8->keypad[0xB] = 1;
                        break;
                    case SDL_SCANCODE_4:
                        chip8->keypad[0xC] = 1;
                        break;
                    case SDL_SCANCODE_R:
                        chip8->keypad[0xD] = 1;
                        break;
                    case SDL_SCANCODE_F:
                        chip8->keypad[0xE] = 1;
                        break;
                    case SDL_SCANCODE_V:
                        chip8->keypad[0xF] = 1;
                        break;
                }
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_X:
                    chip8->keypad[0] = 0;
                    break;
                case SDL_SCANCODE_1:
                    chip8->keypad[1] = 0;
                    break;
                case SDL_SCANCODE_2:
                    chip8->keypad[2] = 0;
                    break;
                case SDL_SCANCODE_3:
                    chip8->keypad[3] = 0;
                    break;
                case SDL_SCANCODE_Q:
                    chip8->keypad[4] = 0;
                    break;
                case SDL_SCANCODE_W:
                    chip8->keypad[5] = 0;
                    break;
                case SDL_SCANCODE_E:
                    chip8->keypad[6] = 0;
                    break;
                case SDL_SCANCODE_A:
                    chip8->keypad[7] = 0;
                    break;
                case SDL_SCANCODE_S:
                    chip8->keypad[8] = 0;
                    break;
                case SDL_SCANCODE_D:
                    chip8->keypad[9] = 0;
                    break;
                case SDL_SCANCODE_Z:
                    chip8->keypad[0xA] = 0;
                    break;
                case SDL_SCANCODE_C:
                    chip8->keypad[0xB] = 0;
                    break;
                case SDL_SCANCODE_4:
                    chip8->keypad[0xC] = 0;
                    break;
                case SDL_SCANCODE_R:
                    chip8->keypad[0xD] = 0;
                    break;
                case SDL_SCANCODE_F:
                    chip8->keypad[0xE] = 0;
                    break;
                case SDL_SCANCODE_V:
                    chip8->keypad[0xF] = 0;
                    break;
            }
            break;
        }
    }

    return quit;
}
