cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(3d_game_template_demo)

target_sources(app PRIVATE src/main.c src/editor_render.c src/editor_UI.c)

zephyr_include_directories(include)

set(TOOLS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../tools)

add_custom_command(
OUTPUT generated/generated_objects.h
COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generate_objects.py ${TOOLS_FOLDER} ${CMAKE_CURRENT_SOURCE_DIR}/data generated
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(generate_objects DEPENDS generated/generated_objects.h)

zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

add_dependencies(app generate_objects)
