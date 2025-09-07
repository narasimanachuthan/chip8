#include "chip8.hpp"
#include <iostream>

const unsigned char fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


Chip8::Chip8() = default;
Chip8::~Chip8() = default;

void Chip8::initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (uint8_t& i : gfx) {
        i = 0;
    }

    for (int i=0; i<16; i++) {
        stack[i] = 0;
        V[i] = 0;
    }

    for (uint8_t& i : memory) {
        i = 0;
    }

    for (int i=0; i<80; i++) {
        memory[i] = fontset[i];
    }

    delayTimer = 0;
    soundTimer = 0;

}

bool Chip8::loadROM(const char *filepath) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        perror("File opening failed.");
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*) malloc(fileSize);
    if (!buffer) {
        perror("Error creating buffer");
        fclose(fp);
        return false;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, fp);
    if (bytesRead != fileSize) {
        perror("Error reading file");
        free(buffer);
        fclose(fp);
        return false;
    }

    for (int i=0; i<bytesRead; i++) {
        memory[i + 512] = buffer[i];
    }

    free(buffer);
    fclose(fp);
    return true;
}

void Chip8::emulateCycle() {
    opcode = memory[pc] << 8 || memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    //0x00E0 - Clear the display
                    for (uint8_t& i : gfx)
                        i = 0;
                    pc += 2;
                    break;
                case 0x000E:
                    //0x00EE - Return from a subroutine
                    pc = stack[sp];
                    sp--;
                    pc += 2;
                    break;
            }
            break;

        case 0x1000:
            //0x1NNN - Jump to location NNN
            pc = opcode & 0xFFF;
            break;

        case 0x2000:
            //0x2NNN - Call subroutine at NNN
            sp++;
            stack[sp] = pc;
            pc = opcode & 0xFFF;

        case 0x3000:
            //0x3XNN - Skip next instruction if VX = NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x4000:
            //0x4XNNN - Skip next instruction if VX != NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x5000:
            //0x5XY0 - Skip next instruction if VX == VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x6000:
            //0x6XNN - Set VX = NN
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000:
            //0x7XNN - Set VX += NN
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    //0x8XY0 - Set VX = VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001:
                    //0x8XY1 - Set VX |= VY
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002:
                    //0x8XY2 - Set VX &= VY
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003:
                    //0x8XY3 - Set VX ^= VY
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0004:
                    //0x8XY4 - Set VX += VY, set VF = carry
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                        V[15] = 1;
                    } else {
                        V[15] = 0;
                    }
                    pc += 2;
                    break;
                case 0x0005:
                    //0x8XY5 - Set VX -= VY, set VF = NOT borrow
                    if (V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4]) {
                        V[15] = 1;
                    } else {
                        V[15] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
            }
        default:
            printf("Unknown opcode: 0x%X\n", opcode);
    }

    if (delayTimer > 0) {
        --delayTimer;
    }

    if (soundTimer > 0) {
        if (soundTimer == 1) {
            printf("BEEP!\n");
        }
        --soundTimer;
    }
}