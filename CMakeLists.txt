# 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Chip8Emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 寻找依赖库
find_package(SDL2 REQUIRED)

# 定义源文件
set(SOURCES
    src/main.c
    src/core/chip8.c
    src/core/instructions.c
    src/sys/config.c
    src/sys/system.c
    src/frontend/window.c
    src/frontend/gui.c
    src/frontend/renderer.c
)

add_executable(chip8 ${SOURCES})

add_library(tinyfd STATIC vendor/tinyfd/tinyfiledialogs.c)
target_include_directories(tinyfd PUBLIC vendor/tinyfd)

# 为已定义的目标配置编译选项 (Compile Options)
if(MSVC)
    # 针对 Visual Studio 编译器
    target_compile_options(chip8 PRIVATE /Wall /WX)
    target_compile_options(chip8 PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    # 针对 GCC / Clang / MinGW 编译器
    target_compile_options(chip8 PRIVATE -Wall -Wextra -Werror)
    target_compile_options(chip8 PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_property(TARGET chip8 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

#指定头文件路径
target_include_directories(chip8 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nuklear
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyfd
    ${SDL2_INCLUDE_DIRS}
)

# 链接库 (Libraries)
if(WIN32)
    target_link_libraries(chip8
        tinyfd
        mingw32
        SDL2main
        SDL2::SDL2
        comdlg32
        ole32
        shell32
        user32
        uuid
    )
elseif(APPLE)
    target_link_libraries(chip8 SDL2::SDL2
        tinyfd
        "-framework Cocoa"
    )
else()
    target_link_libraries(chip8
        SDL2::SDL2
        tinyfd
        m
        pthread
    )
endif()
