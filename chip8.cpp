#include "chip8.hpp"
#include <iostream>
#include <random>

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
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, 255);

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
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;

        case 0x1000:
            //0x1NNN - Jump to location NNN
            pc = opcode & 0xFFF;
            break;

        case 0x2000:
            //0x2NNN - Call subroutine at NNN
            stack[sp] = pc;
            sp++;
            pc = opcode & 0xFFF;
            break;

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
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    pc += 2;
                    break;

                case 0x0005:
                    //0x8XY5 - Set VX -= VY, set VF = NOT borrow
                    if (V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0006:
                    //0x8XY6 - Set VX = VX SHR 1, stores the least significant bit of VX prior to the shift into VF
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                case 0x0007:
                    //0x8XY7 - Set VX = VY - VX, set VF = NOT borrow
                    if (V[(opcode & 0x00F0) >> 4] >= V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x000E:
                    //0x8XYE - Set VX = VX SHL 1, stores the most significant bit of VX prior to the shift into VF
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;

        case 0x9000:
            //0x9XY0 - Skip next instruction if VX != VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            pc += 2;
            break;

        case 0xA000:
            //0xANNN - Set I = NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB000:
            //0xBNNN - Jump to location NNN + V0
            pc = (opcode & 0xFFF) + V[0x0];
            break;

        case 0xC000:
            //0xCXNN - Set VX = random byte AND NN
            V[(opcode & 0x0F00) >> 8] = (uint8_t) (dis(gen)) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: {
            //0xDXYN - Draws a sprite at coordinate (VX, VY), set VF = 1 if any pixel is flipped
            uint8_t X = V[(opcode & 0x0F00) >> 8];
            uint8_t Y = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;
            uint8_t width = 8;
            V[0xF] = 0;

            for (int i = 0; i < height; i++) {
                uint8_t pixel = memory[I + i];
                for (int j = 0; j < width; j++) {
                    if ((pixel & (0x80 >> j)) != 0) {
                        if (gfx[(X + j + ((Y + i) * 64))] == 1) {
                            V[0xF] = 1;
                        }
                        gfx[(X + j + ((Y + i) * 64))] ^= 1;
                    }
                }
            }

            pc += 2;
            break;
        }

        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E:
                    //0xEX9E - Skip next instruction if key with the value of VX is pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
                        pc += 2;
                    }
                    pc += 2;
                    break;

                case 0x0001:
                    //0xEXA1 - Skip next instruction if key with value of VX is not pressed
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
                        pc += 2;
                    }
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    //0xFX07 - Set VX = delay timer value
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    pc += 2;
                    break;

                case 0x000A: {
                    //0xFX0A - Wait for key press and store the value in VX
                    bool keyPressed = false;
                    while (!keyPressed) {
                        for (int i = 0; i < 16; i++) {
                            if (key[i] != 0) {
                                V[(opcode & 0x0F00) >> 8] = i;
                                keyPressed = true;
                            }
                        }
                    }
                    pc += 2;
                    break;
                }

                case 0x0015:
                    //0xFX15 - Set delay timer to VX
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0018:
                    //0xFX18 - Set sound timer to VX
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x001E:
                    //0xFX1E - Set I += VX
                    if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0029:
                    //0xFX29 -  Set I = location of sprite for digit VX
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                case 0x0033:
                    //0xFX33 -  Store BCD representation of VX in memory locations I, I+1, and I+2
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                    break;

                case 0x0055:
                    //0xFX55 - Store registers V0 through VX in memory starting at location I
                    for (int i=0; i<=((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = V[i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                case 0x0065:
                    //0xFX65 - Read registers V0 through Vx from memory starting at location I
                    for (int i=0; i<=((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I + i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;

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