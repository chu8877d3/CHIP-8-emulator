#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "chip8.h"
#include <string.h>

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
        if (chip8->keypad[i] == 1)
            return i;
    }
    return -1;
}
static inline void remove_col(Chip8* chip8, size_t col)
{
    for (size_t row = 0; row < chip8->height; row++) {
        chip8->state[row * chip8->width + col]  = 0;
    }
}

static inline void remove_row(Chip8* chip8, size_t row)
{
    memset(&chip8->state[row * chip8->width], 0, chip8->width * sizeof(bool));
}

static inline void copy_col(Chip8* chip8, size_t col_dst, size_t col_src)
{
    for (size_t row = 0; row < chip8->height; row++) {
        chip8->state[row * chip8->width + col_dst] = chip8->state[row * chip8->width + col_src];
    }
}

static inline void copy_row(Chip8* chip8, size_t row_dst, size_t row_src)
{
    memmove(&chip8->state[row_dst * chip8->width], &chip8->state[row_src * chip8->width], chip8->width * sizeof(bool));
}


#endif