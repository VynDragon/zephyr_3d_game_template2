zephyr_library()
zephyr_library_sources(engine.c)
zephyr_code_relocate(FILES
engine.c
LOCATION RAM NOKEEP)
