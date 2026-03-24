#include "chip8.h"
#include "instructions.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const uint8_t fontset[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

typedef void (*InstructionFunc)(Chip8*, uint16_t);

// 一级分派表
static const InstructionFunc main_table[16] = {
    ins_0_family, ins_1NNN, ins_2NNN, ins_3XKK,
    ins_4XKK, ins_5XY0, ins_6XKK, ins_7XKK,
    ins_8_family, ins_9XY0, ins_ANNN, ins_BNNN,
    ins_CXKK, ins_DXYN, ins_E_family, ins_F_family
};

void chip8_init(Chip8* chip8)
{

    memset(chip8, 0, sizeof(Chip8));
    chip8->pc = 0x200;
    chip8->waiting_key = -1;
    memcpy(chip8->memory, fontset, sizeof(fontset));
    srand(time(NULL));
}

bool chip8_load_rom(Chip8* chip8, const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File failed to open");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    long rom_size = ftell(fp);
    if (rom_size > 3584) {
        perror("File size is too large.");
        fclose(fp);
        return false;
    }
    rewind(fp);
    fread(&chip8->memory[0x200], 1, rom_size, fp);
    fclose(fp);
    return true;
}

void chip8_cycle(Chip8* chip8)
{
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    chip8->pc += 2;

    uint8_t index = (opcode & 0xF000) >> 12;

    main_table[index](chip8, opcode);
}

void update_timers(Chip8* chip8)
{
    chip8->delay_timer -= chip8->delay_timer > 0;
    chip8->sound_timer -= chip8->sound_timer > 0;
}
