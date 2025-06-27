zephyr_library()
zephyr_library_sources(L3.c)
zephyr_code_relocate(FILES
L3.c
LOCATION RAM NOKEEP)
