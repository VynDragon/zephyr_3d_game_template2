cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

zephyr_syscall_include_directories(include)

zephyr_include_directories(include)

add_subdirectory(lib)
