/*
* APACHE 2
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/timing/timing.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(engine_UI);

#include "engine.h"

static timing_t engine_FPS_last_time;
static uint32_t engine_FPS_total_time_avg = 0;
int init_engine_UI(void)
{
	engine_FPS_last_time = timing_counter_get();

	return 0;
}

int engine_render_UI(void)
{
	timing_t fps_time = timing_counter_get();
	uint32_t total_time_us = timing_cycles_to_ns(timing_cycles_get(&engine_FPS_last_time, &fps_time)) / 1000;
	engine_FPS_last_time = fps_time;

	engine_FPS_total_time_avg = (engine_FPS_total_time_avg * 4 + total_time_us) / 5;

// #if defined(CONFIG_FPU)
// 	LOG_ERR("RFPS: %0.1f", (double)engine_rFPS);
// #else
// 	LOG_ERR("RFPS: %d", (int)engine_rFPS);
// #endif
	return 0;
}
