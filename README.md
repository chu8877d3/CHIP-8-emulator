# CHIP-8 Emulator

[![Build and Release](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/release.yml/badge.svg)](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/release.yml)
[![CI](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/ci.yml/badge.svg)](https://github.com/chu8877d3/chip-8-emulator/actions/workflows/ci.yml)

## 1. Introduction / 简介
A high-performance CHIP-8 emulator implemented in **C23** and **SDL2**.  
一个基于 C23 标准和 SDL2 库实现的 CHIP-8 模拟器。具有轻量化的即使模式 GUI。

---

## 2. Download / 下载
Don't want to build? Grab the latest pre-compiled binaries from the [Releases Page](https://github.com/chu8877d3/chip-8-emulator/releases).  
不想手动编译？直接从 [发布页面](https://github.com/chu8877d3/chip-8-emulator/releases) 下载对应平台的压缩包即可。

---

## 3. Build / 编译 (Modern CMake)
### Prerequisites / 前提条件
* **Windows**: [MSYS2](https://www.msys2.org/) (UCRT64) with `mingw-w64-ucrt-x86_64-SDL2`
* **Linux**: `sudo apt install libsdl2-dev zenity`
* **macOS**: `brew install sdl2`

### Compilation / 执行编译
```bash
# 1. 配置构建目录 (Modern Way)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 2. 执行编译
cmake --build build --config Release
```
编译产物位于 `build/` 目录下。

---

## 4. Usage / 使用方法
Simply run the executable and use the GUI to load a ROM file:  
直接运行程序，通过界面选择 ROM 文件即可：

```bash
./Chip8Emulator [path_to_rom]
```

---

## 5. Keymap / 按键映射
CHIP-8 使用 4x4 数字键盘，对应电脑按键如下：

| CHIP-8 | PC Key |       | CHIP-8 | PC Key |
| :----: | :----: | :---: | :----: | :----: |
| **1**  |   1    |       | **2**  |   2    |
| **3**  |   3    |       | **C**  |   4    |
| **4**  |   Q    |       | **5**  |   W    |
| **6**  |   E    |       | **D**  |   R    |
| **7**  |   A    |       | **8**  |   S    |
| **9**  |   D    |       | **E**  |   F    |
| **A**  |   Z    |       | **0**  |   X    |
| **B**  |   C    |       | **F**  |   V    |

---

## 6. Credits / 鸣谢
* [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) - GUI 引擎
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - 原生对话框
* [SDL2](https://www.libsdl.org/) - 媒体底层支撑