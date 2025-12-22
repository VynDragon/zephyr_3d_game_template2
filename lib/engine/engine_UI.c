/*
* APACHE 2
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <stdlib.h>
#include <math.h>

#include <lvgl.h>
#include <lvgl_mem.h>
#include <lvgl_zephyr.h>

#include <zephyr/timing/timing.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(engine_UI);

#include "engine.h"

static lv_display_t *lvgl_display;

static lv_style_t engine_default_transparency;

static lv_obj_t *engine_trianglecount;
static lv_obj_t *engine_FPS;
static timing_t engine_FPS_last_time;
static uint32_t engine_FPS_total_time_avg = 0;

LV_FONT_DECLARE(four_pixel_font);

void engine_UI_set_area(int32_t x, int32_t y, int32_t w, int32_t h)
{
	lv_display_set_resolution(lvgl_display, w, h);
	lv_display_set_offset(lvgl_display, x, y);
}

void engine_UI_lvgl_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
	L3_COLORTYPE *start = &(L3_video_buffer[area->x1 + (area->y1 - 1) * L3_RESOLUTION_X]);
	uint16_t w = area->x2 - area->x1 + 1;
	uint16_t h = area->y2 - area->y1;

	for (uint16_t i = 0; i <= h; i++) {
		memcpy(start, px_map, w * sizeof(L3_COLORTYPE));
		/* Need to clear the buffer ourself for some reason? */
		memset(px_map, 0, w * sizeof(L3_COLORTYPE));
		px_map = &(px_map[w]);
		start = &(L3_video_buffer[area->x1 + (area->y1 + i) * L3_RESOLUTION_X]);
	}
	lv_display_flush_ready(display);
}

int init_engine_UI(void)
{
	lv_display_t *d = NULL;
	lv_style_init(&engine_default_transparency);
	lv_style_set_bg_opa(&engine_default_transparency, LV_OPA_TRANSP);

	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		d = lv_display_get_next(d);
		if (d == NULL) {
			LOG_ERR("Invalid LV display %d object", i);
			return -1;
		}
		lv_display_set_flush_cb(d, engine_UI_lvgl_flush_cb);
		lvgl_display = d;
	}

	lv_display_set_default(lvgl_display);

	lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_set_style_text_font(lv_screen_active(), &four_pixel_font, LV_PART_MAIN);
	engine_UI_set_area(0, L3_RESOLUTION_Y - 3, L3_RESOLUTION_X, 4);

	engine_trianglecount = lv_label_create(lv_screen_active());
	lv_label_set_text_fmt(engine_trianglecount, "Tris: %d", engine_drawnTriangles);
	lv_obj_align(engine_trianglecount, LV_ALIGN_TOP_LEFT, 0, 0);

	engine_FPS = lv_label_create(lv_screen_active());
	lv_label_set_text_fmt(engine_FPS, "FPS: %d", 0);
	lv_obj_align(engine_FPS, LV_ALIGN_TOP_LEFT, 40, 0);

	engine_FPS_last_time = timing_counter_get();

	return 0;
}

int engine_render_UI(void)
{
	timing_t fps_time = timing_counter_get();
	uint32_t total_time_us = timing_cycles_to_ns(timing_cycles_get(&engine_FPS_last_time, &fps_time)) / 1000;
	engine_FPS_last_time = fps_time;

	engine_FPS_total_time_avg = (engine_FPS_total_time_avg * 4 + total_time_us) / 5;

	lv_label_set_text_fmt(engine_trianglecount, "Tris: %d", engine_drawnTriangles);
	lv_label_set_text_fmt(engine_FPS, "FPS: %d", 1000000 / (engine_FPS_total_time_avg != 0 ? engine_FPS_total_time_avg : 1));
	lv_obj_invalidate(lv_screen_active());
	lv_timer_handler();
	return 0;
}
