#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "chip8.h"

void ins_0_family(Chip8* chip8, uint16_t opcode);
void ins_1NNN(Chip8* chip8, uint16_t opcode);
void ins_2NNN(Chip8* chip8, uint16_t opcode);
void ins_3XKK(Chip8* chip8, uint16_t opcode);
void ins_4XKK(Chip8* chip8, uint16_t opcode);
void ins_5XY0(Chip8* chip8, uint16_t opcode);
void ins_6XKK(Chip8* chip8, uint16_t opcode);
void ins_7XKK(Chip8* chip8, uint16_t opcode);
void ins_8_family(Chip8* chip8, uint16_t opcode);
void ins_9XY0(Chip8* chip8, uint16_t opcode);
void ins_ANNN(Chip8* chip8, uint16_t opcode);
void ins_BNNN(Chip8* chip8, uint16_t opcode);
void ins_CXKK(Chip8* chip8, uint16_t opcode);
void ins_DXYN(Chip8* chip8, uint16_t opcode);
void ins_E_family(Chip8* chip8, uint16_t opcode);
void ins_F_family(Chip8* chip8, uint16_t opcode);

static inline uint16_t extract_nnn(uint16_t opcode)
{
    return (opcode & 0x0FFF);
}
static inline uint8_t extract_n(uint16_t opcode)
{
    return (opcode & 0x000F);
}
static inline uint8_t extract_x(uint16_t opcode)
{
    return (opcode & 0x0F00) >> 8;
}
static inline uint8_t extract_y(uint16_t opcode)
{
    return (opcode & 0x00F0) >> 4;
}
static inline uint8_t extract_kk(uint16_t opcode)
{
    return (opcode & 0x00FF);
}
static inline int get_currently_pressed_key(Chip8* chip8)
{
    for (int i = 0; i < 16; i++) {
        if (chip8->keypad[i] == 1) return i;
    }
    return -1;
}

#endif