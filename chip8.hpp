#ifndef CHIP8_EMULATOR_HPP
#define CHIP8_EMULATOR_HPP

#include <cstdint>

class Chip8 {
private:
    uint16_t opcode{};
    uint8_t memory[4096]{};
    uint8_t V[16]{};
    uint16_t I{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    void initialize();
public:
    uint8_t key[16]{};
    uint8_t gfx[64 * 32]{};
    bool drawFlag;

    Chip8();
    bool loadROM(const char* filepath);
    void emulateCycle();
    ~Chip8();
};

#endif //CHIP8_EMULATOR_HPP