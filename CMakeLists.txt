cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(3d_game_template)

target_sources(app PRIVATE lib/L3.c lib/engine.c src/main.c)

zephyr_syscall_include_directories(include)

zephyr_include_directories(include)
zephyr_include_directories(lib)

zephyr_code_relocate(FILES
lib/L3.c
lib/engine.c
LOCATION RAM NOKEEP)
