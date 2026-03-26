# 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Chip8Emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 寻找依赖库
find_package(SDL2 REQUIRED)

# 定义构建目标目标 (Target)
add_executable(chip8 
    src/main.c
    src/chip8.c
    src/instructions.c 
    src/window.c

    include/chip8.h
    include/instructions.h 
    include/window.h
)

# 为已定义的目标配置编译选项 (Compile Options)
if(MSVC)
    # 针对 Visual Studio 编译器
    target_compile_options(chip8 PRIVATE /Wall /WX) 
else()
    # 针对 GCC / Clang / MinGW 编译器
    target_compile_options(chip8 PRIVATE -Wall -Wextra -Werror)
endif()

# 指定头文件路径
target_include_directories(chip8 PRIVATE include ${SDL2_INCLUDE_DIRS})

# 链接库 (Libraries)
if(WIN32)
    find_package(SDL2 REQUIRED CONFIG)
    target_link_libraries(chip8 mingw32 SDL2main SDL2::SDL2)
else()
    target_link_libraries(chip8 SDL2::SDL2)
endif()