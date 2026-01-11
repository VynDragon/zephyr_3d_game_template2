zephyr_library()
zephyr_library_sources(
utility.c
default_scene.c
logo_scene.c
filters.c
)

if (CONFIG_SOC_SERIES_RP2350)

	set(rp2_common_dir ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/rp2_common)
	set(rp2xxx_dir ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/${CONFIG_SOC_SERIES})
	set(common_dir ${ZEPHYR_HAL_RPI_PICO_MODULE_DIR}/src/common)

	zephyr_include_directories(
	    ${rp2xxx_dir}/hardware_regs/include
		${rp2xxx_dir}/hardware_structs/include
	)
	zephyr_library_sources(rp2350.c)

endif()
