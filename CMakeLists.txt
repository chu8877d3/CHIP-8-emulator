# 最低版本要求
cmake_minimum_required(VERSION 3.21...4.3)

# 项目名称
project(Chip8Emulator VERSION 1.0.0 LANGUAGES C)

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

add_executable(Chip8Emulator ${SOURCES})

add_library(tinyfd STATIC vendor/tinyfd/tinyfiledialogs.c)
target_include_directories(tinyfd PUBLIC vendor/tinyfd)

# 为已定义的目标配置编译选项 (Compile Options)
if(MSVC)
    # 针对 Visual Studio 编译器
    target_compile_options(Chip8Emulator PRIVATE /Wall /WX)
    target_compile_options(Chip8Emulator PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    # 针对 GCC / Clang / MinGW 编译器
    target_compile_options(Chip8Emulator PRIVATE -Wall -Wextra -Werror)
    target_compile_options(Chip8Emulator PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_property(TARGET Chip8Emulator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

#指定头文件路径
target_include_directories(Chip8Emulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nuklear
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyfd
)

target_include_directories(Chip8Emulator SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nuklear
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyfd)
# 链接库 (Libraries)
if(WIN32)
    target_link_libraries(Chip8Emulator
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
    target_link_libraries(Chip8Emulator SDL2::SDL2
        tinyfd
        "-framework Cocoa"
    )
else()
    target_link_libraries(Chip8Emulator
        SDL2::SDL2
        tinyfd
        m
        pthread
    )
endif()

# 11. 安装规则 (Installation)
install(TARGETS Chip8Emulator
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/roms/
    DESTINATION bin/roms
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    DESTINATION .
)

# 12. 运行时依赖处理 (仅限 Windows/CMake 3.21+)
if(WIN32)
    install(TARGETS Chip8Emulator
        RUNTIME_DEPENDENCY_SET chip8_deps
        DESTINATION bin
    )
endif()

# 13. CPack 打包配置
set(CPACK_PACKAGE_NAME "CHIP-8-Emulator")
set(CPACK_PACKAGE_VENDOR "chu8877d3")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")

include(CPack)
