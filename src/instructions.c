#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*Instruction)(Chip8*, uint16_t);

static void ins_8XY0(Chip8* chip8, uint16_t opcode) // LD Vx, byte
{
    chip8->V[extract_x(opcode)] = chip8->V[extract_y(opcode)];
}

static void ins_8XY1(Chip8* chip8, uint16_t opcode) // OR Vx, Vy
{
    chip8->V[extract_x(opcode)] |= chip8->V[extract_y(opcode)];
}

static void ins_8XY2(Chip8* chip8, uint16_t opcode) // AND Vx, Vy
{
    chip8->V[extract_x(opcode)] &= chip8->V[extract_y(opcode)];
}

static void ins_8XY3(Chip8* chip8, uint16_t opcode) // XOR Vx, Vy
{
    chip8->V[extract_x(opcode)] ^= chip8->V[extract_y(opcode)];
}

static void ins_8XY4(Chip8* chip8, uint16_t opcode) // ADD Vx, Vy
{
    uint8_t x = extract_x(opcode);
    uint16_t result = chip8->V[x] + chip8->V[extract_y(opcode)];
    chip8->V[x] = result & 0x00FF;
    chip8->V[0xF] = result > 0xFF;
}

static void ins_8XY5(Chip8* chip8, uint16_t opcode) // SUB Vx, Vy
{
    uint8_t x = extract_x(opcode);
    uint8_t y = extract_y(opcode);
    uint8_t v_x = chip8->V[x];
    uint8_t v_y = chip8->V[y];

    uint8_t flag = v_x >= v_y;
    chip8->V[x] = v_x - v_y;
    chip8->V[0xF] = flag;
}

static void ins_8XY6(Chip8* chip8, uint16_t opcode) // SHR Vx, Vy
{
    uint8_t x = extract_x(opcode);
    uint8_t y = extract_y(opcode);

    chip8->V[x] = chip8->shift_quirk ? chip8->V[x] : chip8->V[y];
    uint8_t flag = chip8->V[x] & 0x01;
    chip8->V[x] >>= 1;
    chip8->V[0xF] = flag;
}

static void ins_8XY7(Chip8* chip8, uint16_t opcode) // SUBN Vx, Vy
{
    uint8_t x = extract_x(opcode);
    uint8_t y = extract_y(opcode);
    uint8_t v_x = chip8->V[x];
    uint8_t v_y = chip8->V[y];

    uint8_t flag = v_y >= v_x;
    chip8->V[x] = v_y - v_x;
    chip8->V[0xF] = flag;
}

static void ins_8XYE(Chip8* chip8, uint16_t opcode) // SHL Vx, Vy
{
    uint8_t x = extract_x(opcode);
    uint8_t y = extract_y(opcode);

    chip8->V[x] = chip8->shift_quirk ? chip8->V[x] : chip8->V[y];
    uint8_t flag = (chip8->V[x] & 0x80) >> 7;
    chip8->V[x] <<= 1;
    chip8->V[0xF] = flag;
}

static void ins_null(Chip8* chip8, uint16_t opcode)
{
    (void)chip8;
    (void)opcode;
    fprintf(stderr, "Unknown opcode: 0x%4X\n", opcode);
}

static const Instruction table_8[16] = {
    ins_8XY0, ins_8XY1, ins_8XY2, ins_8XY3,
    ins_8XY4, ins_8XY5, ins_8XY6, ins_8XY7,
    ins_null, ins_null, ins_null, ins_null,
    ins_null, ins_null, ins_8XYE, ins_null
};

static inline void ins_00E0(Chip8* chip8, uint16_t opcode) // CLS
{
    (void)opcode;
    chip8->draw_flag = true;
    memset(chip8->state, 0, sizeof(chip8->state));
    for (int i = 0; i < SCREEN_SIZE; i++) {
        chip8->video[i] = COLOR_BACKGROUND;
    }
}

static inline void ins_00EE(Chip8* chip8, uint16_t opcode) // RET
{
    (void)opcode;
    chip8->sp--;
    chip8->pc = chip8->stack[chip8->sp];
}

// ins_0系列指令
void ins_0_family(Chip8* chip8, uint16_t opcode)
{
    switch (opcode & 0X00FF) {
    case 0x00E0:
        ins_00E0(chip8, opcode);
        break;
    case 0x00EE:
        ins_00EE(chip8, opcode);
        break;
    default:
        ins_null(chip8, opcode);
        break;
    }
}

void ins_1NNN(Chip8* chip8, uint16_t opcode) // JP addr
{
    chip8->pc = extract_nnn(opcode);
}

void ins_2NNN(Chip8* chip8, uint16_t opcode) // CALL addr
{

    chip8->stack[chip8->sp] = chip8->pc;
    chip8->sp++;
    chip8->pc = extract_nnn(opcode);
}

void ins_3XKK(Chip8* chip8, uint16_t opcode) // SE Vx, byte
{
    chip8->pc += (chip8->V[extract_x(opcode)] == extract_kk(opcode)) * 2;
}

void ins_4XKK(Chip8* chip8, uint16_t opcode) // SNE Vx, Byte
{
    chip8->pc += (chip8->V[extract_x(opcode)] != extract_kk(opcode)) * 2;
}

void ins_5XY0(Chip8* chip8, uint16_t opcode) // SR Vx, Vy
{
    chip8->pc += (chip8->V[extract_x(opcode)] == chip8->V[extract_y(opcode)]) * 2;
}

void ins_6XKK(Chip8* chip8, uint16_t opcode) // LD Vx, byte
{
    chip8->V[extract_x(opcode)] = extract_kk(opcode);
}

void ins_7XKK(Chip8* chip8, uint16_t opcode) // ADD Vx, byte
{
    chip8->V[extract_x(opcode)] += extract_kk(opcode);
}

// ins_8系列指令
void ins_8_family(Chip8* chip8, uint16_t opcode)
{
    uint16_t index = opcode & 0x000F;
    table_8[index](chip8, opcode);
}

void ins_9XY0(Chip8* chip8, uint16_t opcode) // SNE Vx, Vy
{
    chip8->pc += (chip8->V[extract_x(opcode)] != chip8->V[extract_y(opcode)]) * 2;
}

void ins_ANNN(Chip8* chip8, uint16_t opcode) // LE l, addr
{
    chip8->I = extract_nnn(opcode);
}

void ins_BNNN(Chip8* chip8, uint16_t opcode) // JP V0, addr
{
    chip8->pc = extract_nnn(opcode) + chip8->V[0x0];
}

void ins_CXKK(Chip8* chip8, uint16_t opcode) // RND Vx, byte
{
    chip8->V[extract_x(opcode)] = extract_kk(opcode) & rand() % 256;
}

void ins_DXYN(Chip8* chip8, uint16_t opcode) // DRW Vx, Vy, nibble
{
    uint8_t x_start = chip8->V[extract_x(opcode)] % SCREEN_WIDTH;
    uint8_t y_start = chip8->V[extract_y(opcode)] % SCREEN_HEIGHT;
    uint8_t nibble = extract_n(opcode);
    chip8->V[0xF] = 0;

    chip8->draw_flag = true;
    for (uint8_t row = 0; row < nibble; row++) {
        if (y_start + row >= SCREEN_HEIGHT)
            break;

        uint8_t sprite_byte = chip8->memory[chip8->I + row];
        for (uint8_t col = 0; col < 8; col++) {

            if (sprite_byte & (0x80 >> col)) {
                if (x_start + col >= SCREEN_WIDTH)
                    continue;

                int index = (x_start + col) + (y_start + row) * SCREEN_WIDTH;

                chip8->V[0xF] |= chip8->state[index];

                chip8->state[index] ^= 1;

                chip8->video[index] = chip8->state[index] ? COLOR_PIXEL : COLOR_BACKGROUND;
            }
        }
    }
}
static inline void ins_EX9E(Chip8* chip8, uint16_t opcode) // SKP Vx
{
    uint8_t key = chip8->V[extract_x(opcode)] & 0xF;
    chip8->pc += chip8->keypad[key] ? 2 : 0;
}

static inline void ins_EXA1(Chip8* chip8, uint16_t opcode) // SKNP Vx
{
    uint8_t key = chip8->V[extract_x(opcode)] & 0xF;
    chip8->pc += (!chip8->keypad[key]) ? 2 : 0;
}

void ins_E_family(Chip8* chip8, uint16_t opcode)
{
    switch (opcode & 0x00FF) {
    case 0x009E:
        ins_EX9E(chip8, opcode);
        break;
    case 0x00A1:
        ins_EXA1(chip8, opcode);
        break;
    default:
        ins_null(chip8, opcode);
    }
}

static inline void ins_FX07(Chip8* chip8, uint16_t opcode) // LD Vx, DT
{
    chip8->V[extract_x(opcode)] = chip8->delay_timer;
}

static inline void ins_FX0A(Chip8* chip8, uint16_t opcode) // LD Vx, K
{
    int pressed_key = get_currently_pressed_key(chip8);

    if (pressed_key >= 0) {
        chip8->waiting_key = pressed_key;
        chip8->key_was_pressed = true;
        chip8->pc -= 2;
    } else {
        if (chip8->key_was_pressed) {
            chip8->V[extract_x(opcode)] = chip8->waiting_key;
            chip8->key_was_pressed = false;
            chip8->waiting_key = -1;
        } else {
            chip8->pc -= 2;
        }
    }
}

static inline void ins_FX15(Chip8* chip8, uint16_t opcode) // LD DT, Vx
{
    chip8->delay_timer = chip8->V[extract_x(opcode)];
}

static inline void ins_Fx18(Chip8* chip8, uint16_t opcode) // LD ST, Vx
{
    chip8->sound_timer = chip8->V[extract_x(opcode)];
}

static inline void ins_FX1E(Chip8* chip8, uint16_t opcode) // ADD I, Vx
{
    chip8->I += chip8->V[extract_x(opcode)];
}

static inline void ins_FX29(Chip8* chip8, uint16_t opcode) // LD F, Vx
{
    chip8->I = chip8->V[extract_x(opcode)] * 5;
}

static inline void ins_FX33(Chip8* chip8, uint16_t opcode) // LD B, Vx
{
    uint8_t value = chip8->V[extract_x(opcode)];
    chip8->memory[chip8->I] = value / 100;
    chip8->memory[chip8->I + 1] = (value / 10) % 10;
    chip8->memory[chip8->I + 2] = value % 10;
}

static inline void ins_FX55(Chip8* chip8, uint16_t opcode) // LD  I, Vx
{
    uint8_t x = extract_x(opcode);
    memcpy(&chip8->memory[chip8->I], chip8->V, x + 1);
    if (!chip8->shift_quirk) {
        chip8->I += x + 1;
    }
}

static inline void ins_FX65(Chip8* chip8, uint16_t opcode) // LD Vx, I
{
    uint8_t x = extract_x(opcode);
    memcpy(chip8->V, &chip8->memory[chip8->I], x + 1);

    if (!chip8->shift_quirk) {
        chip8->I += x + 1;
    }
}

void ins_F_family(Chip8* chip8, uint16_t opcode)
{
    switch (opcode & 0x00FF) {
    case 0x0007:
        ins_FX07(chip8, opcode);
        break;
    case 0x000A:
        ins_FX0A(chip8, opcode);
        break;
    case 0x0015:
        ins_FX15(chip8, opcode);
        break;
    case 0x0018:
        ins_Fx18(chip8, opcode);
        break;
    case 0x001E:
        ins_FX1E(chip8, opcode);
        break;
    case 0x0029:
        ins_FX29(chip8, opcode);
        break;
    case 0x0033:
        ins_FX33(chip8, opcode);
        break;
    case 0x0055:
        ins_FX55(chip8, opcode);
        break;
    case 0x0065:
        ins_FX65(chip8, opcode);
        break;
    default:
        ins_null(chip8, opcode);
    }
}