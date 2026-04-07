# 最低版本要求
cmake_minimum_required(VERSION 3.21...4.3)

# 项目名称
project(Chip8Emulator VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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


add_library(tinyfd_lib STATIC vendor/tinyfd/tinyfiledialogs.c)
target_include_directories(tinyfd_lib PUBLIC vendor/tinyfd)

add_library(json_lib STATIC vendor/json/cJSON.c)
target_include_directories(json_lib PUBLIC vendor/json)

add_library(nuklear_lib STATIC vendor/nuklear/nuklear_impl.c)
target_include_directories(nuklear_lib PUBLIC vendor/nuklear)
target_link_libraries(nuklear_lib PUBLIC SDL2::SDL2)

# 为已定义的目标配置编译选项
if(MSVC)
    target_compile_options(Chip8Emulator PRIVATE /Wall /WX)
    target_compile_options(Chip8Emulator PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    # 启用严格警告，但屏蔽第三方库 Nuklear 常见的未使用变量警告
    target_compile_options(Chip8Emulator PRIVATE -Wall -Wextra -Werror -Wno-unused-but-set-variable)
    target_compile_options(Chip8Emulator PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_property(TARGET Chip8Emulator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# 指定头文件路径
target_include_directories(Chip8Emulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sys
)

# 链接库
if(WIN32)
    target_link_libraries(Chip8Emulator
        tinyfd_lib
        json_lib
        nuklear_lib
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
    target_link_libraries(Chip8Emulator
        SDL2::SDL2
        tinyfd_lib
        json_lib
        nuklear_lib
        "-framework Cocoa"
    )
else()
    target_link_libraries(Chip8Emulator
        SDL2::SDL2
        nuklear_lib
        json_lib
        tinyfd_lib
        m
        pthread
    )
endif()

# 安装规则
install(TARGETS Chip8Emulator RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/roms/ DESTINATION bin/roms)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    DESTINATION .
)

# 运行时依赖处理 (Windows)
if(WIN32)
    install(TARGETS Chip8Emulator
        RUNTIME_DEPENDENCY_SET chip8_deps
        DESTINATION bin
    )
endif()

# CPack 打包配置
set(CPACK_PACKAGE_NAME "CHIP-8-Emulator")
set(CPACK_PACKAGE_VENDOR "chu8877d3")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")

include(CPack)
