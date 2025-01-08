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