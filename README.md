# CHIP-8 Emulator

[![Build and Release](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/release.yml/badge.svg)](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/release.yml)
[![CI](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/ci.yml/badge.svg)](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/ci.yml)

### 1. Introduction / 简介
A high-performance CHIP-8 emulator implemented in C23 and SDL2, featuring a lightweight GUI and native file dialogs.  
一个基于 C23 标准和 SDL2 库实现的 CHIP-8 模拟器，具备轻量化图形界面和原生文件对话框

### 2. Build / 编译
**Prerequisites / 前提条件:** SDL2 library.

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles"
cmake --build .
```

### 3. Usage / 使用方法
Run the emulator with a ROM file and an optional quirk flag:  
运行模拟器时需指定 ROM 路径及可选的兼容性模式：

```bash
./chip8 <rom_path> [--old|--new|--modern]
```
*   `--old`: Original behavior (COMAC VIP). / 原始模式。
*   `--new`: Modern behavior (Shift quirk enabled, default). / 现代模式（默认）。

### 4. Keymap / 按键映射
The 4x4 keypad is mapped to your keyboard as follows:  
4x4 数字键盘与电脑按键的映射关系如下：

| CHIP-8 Key | PC Key |     | CHIP-8 Key | PC Key |
| :--------: | :----: | --- | :--------: | :----: |
|   **1**    |   1    |     |   **2**    |   2    |
|   **3**    |   3    |     |   **C**    |   4    |
|   **4**    |   Q    |     |   **5**    |   W    |
|   **6**    |   E    |     |   **D**    |   R    |
|   **7**    |   A    |     |   **8**    |   S    |
|   **9**    |   D    |     |   **E**    |   F    |
|   **A**    |   Z    |     |   **0**    |   X    |
|   **B**    |   C    |     |   **F**    |   V    |

### 5.Credits / 第三方库说明
This project uses the following lightweight libraries:
*   [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear): Immediate mode GUI library (Unlicense).
*   [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/): Native dialog library (zlib license).