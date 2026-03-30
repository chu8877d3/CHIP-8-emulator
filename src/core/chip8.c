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
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static const uint8_t big_fontest[160] = {
    0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, // 0
    0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF, // 1
    0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // 2
    0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 3
    0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, // 4
    0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 5
    0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 6
    0xFF, 0xFF, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, // 7
    0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 8
    0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 9
    0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, // A
    0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, // B
    0x3C, 0xFF, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xFF, 0x3C, // C
    0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC, // D
    0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // E
    0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0  // F
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
    chip8->running = true;
    chip8->width = 64;
    chip8->height = 32;
    chip8_load_quirks(chip8, QUIRK_PROFILE_SCHIP_LEGACY);
    memcpy(chip8->memory, fontset, sizeof(fontset));
    memcpy(chip8->memory + sizeof(fontset), big_fontest, sizeof(big_fontest));
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
    if (rom_size > 3584) { // 0x100 - 0x200 = 3584
        perror("File size is too large.");
        fclose(fp);
        return false;
    }
    rewind(fp);
    size_t bytes_read = fread(&chip8->memory[0x200], 1, (size_t)rom_size, fp);
    fclose(fp);
    if (bytes_read != (size_t)rom_size) {
        fprintf(stderr, "Error: Failed to read ROM. Read %zu and %ld bytes\n",
            bytes_read, rom_size);
        return false;
    }
    return true;
}

void chip8_cycle(Chip8* chip8)
{
   if (chip8->pc >= 4094) {
        fprintf(stderr, "ERROR: PC out of bounds! PC=0x%x\n", chip8->pc);
        chip8->running = false;
        return;
    }

    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    chip8->pc += 2;

    uint8_t index = (opcode & 0xF000) >> 12;
    
    main_table[index](chip8, opcode);
}

void chip8_update_timers(Chip8* chip8)
{
    chip8->delay_timer -= chip8->delay_timer > 0;
    chip8->sound_timer -= chip8->sound_timer > 0;
}

void chip8_load_quirks(Chip8* chip8, QuirkProfile profile)
{
    switch (profile) {
    case QUIRK_PROFILE_COSMAC_VIP:
        chip8->shift_quirk = true;
        chip8->loadstore_quirk = true;
        chip8->clip_quirk = true;
        chip8->vf_reset_quirk = false;
        chip8->jump_quirk = false;
        break;
    case QUIRK_PROFILE_SCHIP_LEGACY:
        chip8->shift_quirk = false;
        chip8->loadstore_quirk = false;
        chip8->clip_quirk = true;
        chip8->vf_reset_quirk = false;
        chip8->jump_quirk = true;
        break;
    case QUIRK_PROFILE_MODERN:
        chip8->shift_quirk = false;
        chip8->loadstore_quirk = false;
        chip8->clip_quirk = false;
        chip8->vf_reset_quirk = false;
        chip8->jump_quirk = false;
        break;
    }
}

void chip8_restart(Chip8* chip8)
{
    chip8->pc = 0x200;
    chip8->sp = 0;
    chip8->I = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    memset(chip8->rpl, 0, sizeof(chip8->rpl));
    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->keypad, 0, sizeof(chip8->keypad));
    memset(chip8->state, 0, sizeof(chip8->state));
}