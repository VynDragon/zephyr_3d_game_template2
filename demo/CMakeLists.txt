cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(3d_game_template_demo)

target_sources(app PRIVATE src/main.c)

zephyr_include_directories(include)
