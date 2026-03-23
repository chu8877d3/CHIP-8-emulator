#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH)
#define MAKE_RGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | a)

#define COLOR_BACKGROUND MAKE_RGBA(0, 5, 0, 255)
#define COLOR_PIXEL MAKE_RGBA(0, 255, 65, 255)

typedef struct
{
    uint8_t memory[4096]; // 4k 主存 (RAM)
    uint8_t V[16]; // 16个通用寄存器组
    uint16_t I; // 16位地址寄存器
    uint16_t pc; // 16位程序计数器

    uint16_t stack[16]; // 调用栈
    uint8_t sp; // 栈指针

    uint8_t delay_timer; // 延迟定时器
    uint8_t sound_timer; // 声音定时器

    bool keypad[16]; // 16个十六进制按键状态
    bool state[SCREEN_SIZE]; // 64x32 显存状态缓冲区
    uint32_t video[SCREEN_SIZE]; // 64x32 单色显存

    bool shift_quirk; // true 为现代模式，false 为原始模式
} Chip8;

void chip8_init(Chip8* chip8);
bool chip8_load_rom(Chip8* chip8, const char* filename);
void chip8_cycle(Chip8* chip8);
void update_timers(Chip8* chip8);

#endif