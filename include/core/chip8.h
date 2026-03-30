#ifndef CHIP8_H
#define CHIP8_H

#include <stddef.h>
#include <stdint.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH)

typedef enum {
    QUIRK_PROFILE_COSMAC_VIP,
    QUIRK_PROFILE_SCHIP_LEGACY,
    QUIRK_PROFILE_MODERN
} QuirkProfile;

typedef struct Chip8
{
    uint8_t memory[4096]; // 4k 主存 (RAM)

    uint16_t stack[16]; // 调用栈

    // 显存（单色 + 状态缓冲区）
    uint32_t video[SCREEN_SIZE]; // 单色显存
    bool state[SCREEN_SIZE]; // 显存状态缓冲区

    // 16位寄存器
    uint16_t I; // 16位地址寄存器
    uint16_t pc; // 16位程序计数器

    // 寄存器组
    uint8_t V[16]; // 16个通用寄存器组
    uint8_t rpl[8]; // 8个用户寄存器（SCHIP新增）

    // 定时器
    uint8_t delay_timer; // 延迟定时器
    uint8_t sound_timer; // 声音定时器

    uint8_t sp; // 栈指针

    // 按键相关
    uint8_t waiting_key; // 按键编号
    bool key_was_pressed; // 按键是否被按下
    bool keypad[16]; // 16个十六进制按键状态

    // 显示参数
    int width; // 显示宽度（像素）
    int height; // 显示高度（像素）

    // 状态标志
    bool draw_flag; // 绘图标志位
    bool running; // 是否运行

    QuirkProfile mode;
    // quirk 兼容性开关：true 为开，false 为关
    bool shift_quirk; // 8XY6, 8XYE 两个移位怪癖，Vx = Vy >> 1 or Vx >>= 1; 是否使用Vy
    bool loadstore_quirk; // FX55, FX65 I += X + 1 or I 不变； I是否变化
    bool clip_quirk; // DXYN 画图是否裁剪
    bool vf_reset_quirk; // 8XY1 8XY2 8XY3 是否重置 VF
    bool jump_quirk; // BNNN NNN + V0 or NNN +Vx 是否变种
} Chip8;

void chip8_init(Chip8* chip8);
bool chip8_load_rom(Chip8* chip8, const char* filename);
void chip8_cycle(Chip8* chip8);
void chip8_update_timers(Chip8* chip8);
void chip8_load_quirks(Chip8* chip8, QuirkProfile profile);
void chip8_restart(Chip8* chip8);

#endif