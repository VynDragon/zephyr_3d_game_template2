zephyr_library()
zephyr_library_sources(
engine.c
)

if (NOT CONFIG_HEADLESS)
zephyr_library_sources(
engine_UI.c
four_pixel_font.c
)
else()
zephyr_library_sources(
engine_UI_mock.c
)
endif()

zephyr_code_relocate(FILES
engine.c
engine_UI.c
LOCATION RAM NOKEEP)
