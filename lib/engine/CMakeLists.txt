zephyr_library()
zephyr_library_sources(
engine.c
engine_UI.c
four_pixel_font.c
)
zephyr_code_relocate(FILES
engine.c
engine_UI.c
LOCATION RAM NOKEEP)
