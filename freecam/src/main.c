#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/input/input.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/util.h>
#include <math.h>

#include <lvgl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define ENGINE_BLIT_FUNCTION blit_display

#include "engine.h"
#include "utility.h"
#include "logo_scene.h"

#include "map.h"

#include "filters.h"

#include "skybox/h2s_bk.h"
#include "skybox/h2s_dn.h"
#include "skybox/h2s_ft.h"
#include "skybox/h2s_lf.h"
#include "skybox/h2s_rt.h"
#include "skybox/h2s_up.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int blit_display_L8(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
	buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;

	display_write(display_device, x, y, &buf_desc, buffer);

	return 0;
}

int blit_display_MONO(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[128] = {0};
	
	if (size_y < 8 || (y & 0x7) != 0 || (size_y & 0x7) != 0) {
		LOG_ERR("Bad position or size");
	}
	
	buf_desc.buf_size = size_x;
	buf_desc.width = size_x;
	buf_desc.height = 8;
	buf_desc.pitch = size_x;

	for (int j = 0; j < size_y; j+= 8) {
		for (int i = 0; i < size_x; i++) {
			buf[i] = buffer[i + (j + 0) * size_x] > 128 ? buf[i] | 1<<0 : buf[i] & ~(1<<0);
			buf[i] = buffer[i + (j + 1) * size_x] > 128 ? buf[i] | 1<<1 : buf[i] & ~(1<<1);
			buf[i] = buffer[i + (j + 2) * size_x] > 128 ? buf[i] | 1<<2 : buf[i] & ~(1<<2);
			buf[i] = buffer[i + (j + 3) * size_x] > 128 ? buf[i] | 1<<3 : buf[i] & ~(1<<3);
			buf[i] = buffer[i + (j + 4) * size_x] > 128 ? buf[i] | 1<<4 : buf[i] & ~(1<<4);
			buf[i] = buffer[i + (j + 5) * size_x] > 128 ? buf[i] | 1<<5 : buf[i] & ~(1<<5);
			buf[i] = buffer[i + (j + 6) * size_x] > 128 ? buf[i] | 1<<6 : buf[i] & ~(1<<6);
			buf[i] = buffer[i + (j + 7) * size_x] > 128 ? buf[i] | 1<<7 : buf[i] & ~(1<<7);
		}
		display_write(display_device, x, j, &buf_desc, buf);
	}

	return 0;
}

int blit_display_MONO_dither(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[128] = {0};
	int error = 0;
	
	if (size_y < 8 || (y & 0x7) != 0 || (size_y & 0x7) != 0) {
		LOG_ERR("Bad position or size");
	}
	
	buf_desc.buf_size = size_x;
	buf_desc.width = size_x;
	buf_desc.height = 8;
	buf_desc.pitch = size_x;

#define ERR_THRE 255
#define ERR_MUL 2
	
	for (int j = 0; j < size_y; j+= 8) {
		for (int i = 0; i < size_x; i++) {
			error += buffer[i + (j + 0) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 0;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<0);
			}
			error += buffer[i + (j + 1) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 1;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<1);
			}
			error += buffer[i + (j + 2) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 2;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<2);
			}
			error += buffer[i + (j + 3) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 3;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<3);
			}
			error += buffer[i + (j + 4) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 4;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<4);
			}
			error += buffer[i + (j + 5) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 5;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<5);
			}
			error += buffer[i + (j + 6) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 6;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<6);
			}
			error += buffer[i + (j + 7) * size_x];
			if (error > ERR_THRE) {
				buf[i] |= 1 << 7;
				error -= ERR_THRE * ERR_MUL;
			} else {
				buf[i] &= ~(1<<7);
			}
			/* buf[i] = buffer[i + (j + 0) * size_x] > 85 ? buf[i] | 1<<0 : buf[i] & ~(1<<0);
			buf[i] = buffer[i + (j + 0) * size_x] > 170 ? buf[i] | 1<<1 : buf[i] & ~(1<<1);
			buf[i] = buffer[i + (j + 1) * size_x] > 85 ? buf[i] | 1<<2 : buf[i] & ~(1<<2);
			buf[i] = buffer[i + (j + 1) * size_x] > 170 ? buf[i] | 1<<3 : buf[i] & ~(1<<3);
			buf[i] = buffer[i + (j + 2) * size_x] > 85 ? buf[i] | 1<<4 : buf[i] & ~(1<<4);
			buf[i] = buffer[i + (j + 2) * size_x] > 170 ? buf[i] | 1<<5 : buf[i] & ~(1<<5);
			buf[i] = buffer[i + (j + 3) * size_x] > 85 ? buf[i] | 1<<6 : buf[i] & ~(1<<6);
			buf[i] = buffer[i + (j + 3) * size_x] > 170 ? buf[i] | 1<<7 : buf[i] & ~(1<<7); */
		}
		display_write(display_device, x, j, &buf_desc, buf);
	}

	return 0;
}

int blit_display(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	return blit_display_L8(buffer, x, y, size_x, size_y);
}

typedef struct {
	L3_Unit vx;
	L3_Unit vy;
	L3_Unit z;
	L3_Unit x;
	L3_Unit speedmul;
	L3_Unit jump;
	L3_Unit xrot;
} Controls;

static Engine_Object* player = 0;

static Controls controls = {0};

static void process() {
	if (player != 0) {
		player->visual.transform.rotation.y += controls.vy;
		player->visual.transform.rotation.x += controls.vx;
		if (player->visual.transform.rotation.y > L3_F/2) player->visual.transform.rotation.y = -L3_F/2;
		if (player->visual.transform.rotation.y < -L3_F/2) player->visual.transform.rotation.y = L3_F/2;
		if (player->visual.transform.rotation.x > L3_F/2) player->visual.transform.rotation.x = -L3_F/2;
		if (player->visual.transform.rotation.x < -L3_F/2) player->visual.transform.rotation.x = L3_F/2;

		L3_Vec4 forward = {0, 0, L3_F, L3_F};
		L3_Vec4 left = {-L3_F, 0, 0, L3_F};
		L3_Mat4 transMat;

		L3_makeRotationMatrixZXY(player->visual.transform.rotation.x,
							player->visual.transform.rotation.y,
							0,
							transMat);

		L3_vec3Xmat4(&forward, transMat);
		L3_vec3Xmat4(&left, transMat);

		player->visual.transform.translation.x += controls.z * forward.x / L3_F;
		player->visual.transform.translation.y += controls.z * forward.y / L3_F;
		player->visual.transform.translation.z += controls.z * forward.z / L3_F;
		player->visual.transform.translation.x += controls.x * left.x / L3_F;
		player->visual.transform.translation.y += controls.x * left.y / L3_F;
		player->visual.transform.translation.z += controls.x * left.z / L3_F;

		L3_Camera *camera = engine_getcamera();
		camera->transform = player->visual.transform;
	}
}

static void update_controls(struct input_event *evt, void *user_data)
{
	Controls *cont = user_data;

	if (evt->code == INPUT_KEY_UP) {
		if (evt->value)
			cont->vx = 5;
		else
			cont->vx = 0;
	} else if (evt->code == INPUT_KEY_DOWN) {
		if (evt->value)
			cont->vx = -5;
		else
			cont->vx = 0;
	}
	if (evt->code == INPUT_KEY_LEFT) {
		if (evt->value)
			cont->vy = 15;
		else
			cont->vy = 0;
	} else if (evt->code == INPUT_KEY_RIGHT) {
		if (evt->value)
			cont->vy = -15;
		else
			cont->vy = 0;
	}
	if (evt->code == INPUT_KEY_W) {
		if (evt->value)
			cont->z = E_SPEED(3*L3_F*cont->speedmul);
		else
			cont->z = 0;
	} else if (evt->code == INPUT_KEY_S) {
		if (evt->value)
			cont->z = -E_SPEED(3*L3_F*cont->speedmul);
		else
			cont->z = 0;
	}

	if (evt->code == INPUT_KEY_A) {
		if (evt->value)
			cont->x = E_SPEED(3*L3_F*cont->speedmul);
		else
			cont->x = 0;
	} else if (evt->code == INPUT_KEY_D) {
		if (evt->value)
			cont->x = -E_SPEED(3*L3_F*cont->speedmul);
		else
			cont->x = 0;
	}

	if (evt->code == INPUT_KEY_LEFTSHIFT) {
		if (evt->value)
			cont->speedmul = 8;
		else
			cont->speedmul = 1;
	}
}

INPUT_CALLBACK_DEFINE(0, update_controls, &controls);

#define CAM_ANIM_0(i, _) {.type = 2, .transform = {.translation = {.x = 0, .y = 0, .z = 0}, .rotation = {0}, .scale = {0}}}
#define CAM_ANIM_1(i, _) {.type = 2, .transform = {.translation = {.x = 0, .y = 0, .z = 128}, .rotation = {0}, .scale = {0}}}
#define CAM_ANIM_2(i, _) {.type = 2, .transform = {.translation = {.x = 0, .y = 32, .z = 0}, .rotation = {.x = -1, .y = 4, .z = 0}, .scale = {0}}}
#define CAM_ANIM_3(i, _) {.type = 2, .transform = {.translation = {.x = 0, .y = 0, .z = -128}, .rotation = {0}, .scale = {0}}}
#define CAM_ANIM_4(i, _) {.type = 2, .transform = {.translation = {.x = 0, .y = -32, .z = 0}, .rotation = {.x = 1, .y = 2, .z = 0}, .scale = {0}}}
#define CAM_ANIM_5(i, _) {.type = 2, .transform = {.translation = {.x = 16, .y = 0, .z = 0}, .rotation = {0}, .scale = {0}}}

const ObjectProcess_FrameArray_Frame camera_animation_impl_frames[] = {
	LISTIFY(50, CAM_ANIM_0, (,)),
	LISTIFY(300, CAM_ANIM_1, (,)),
	LISTIFY(64, CAM_ANIM_2, (,)),
	LISTIFY(70, CAM_ANIM_3, (,)),
	LISTIFY(64, CAM_ANIM_4, (,)),
	LISTIFY(200, CAM_ANIM_5, (,)),
};

const ObjectProcess_FrameArray camera_animation_impl = {
	.len = ARRAY_SIZE(camera_animation_impl_frames),
	.loop = false,
	.frames = camera_animation_impl_frames,
};

Engine_Object *camera_animation_objects[1];

Animation camera_animation = ANIMATION_INIT(
	camera_animation_objects,
	(void*[]) {(void*)&camera_animation_impl},
	(AnimationObjectProcess[]) {utility_animation_objectprocess_framearray},
	1, 30,
	ARRAY_SIZE(camera_animation_impl_frames),
	false
);

void scene_init(void *data)
{
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,1.5*L3_F,-20*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);
	camera_animation_objects[0] = player;

	L3_Camera *camera = engine_getcamera();
	camera->focalLength = 288;
}


static timing_t bench_start_time;
static uint32_t bench_frames;
static lv_obj_t *bench_passed;


void scene_pf(Engine_Scene *self)
{
	if (!camera_animation.started)
	{
		bench_frames = 0;
		bench_start_time = timing_counter_get();
	}
	if (camera_animation.finished) {
		timing_t bench_end_time = timing_counter_get();
		uint32_t total_time_us = timing_cycles_to_ns(timing_cycles_get(&bench_start_time, &bench_end_time)) / 1000;
		uint32_t avg_fps = (1000000 * bench_frames) / total_time_us;
		camera_animation.finished = false;
		camera_animation.started = false;
		printf("Average Frame/Second: %u\n", avg_fps);
		if (avg_fps >= CONFIG_TARGET_RENDER_FPS) {
			lv_label_set_text_fmt(bench_passed, "PASS");
			printf("\e[0;32m%u >= %u, passed\e[0m\n", avg_fps, CONFIG_TARGET_RENDER_FPS);
		} else {
			printf("\e[0;31m%u < %u, failed\e[0m\n", avg_fps, CONFIG_TARGET_RENDER_FPS);
			lv_label_set_text_fmt(bench_passed, "FAIL");
		}
		L3_transform3DSet(0 * L3_F,1.5*L3_F,-20*L3_F,0,0,0,L3_F,L3_F,L3_F, &player->visual.transform);
	} else {
		utility_animation_process(&camera_animation);
	}
}

Engine_Scene scene = {0};

static Filter_f default_scene_filters[] = {
	filter_fixgap,
};

L3_Skybox skybox = {
	.faces.front = &h2s_ft,
	.faces.back = &h2s_bk,
	.faces.left = &h2s_lf,
	.faces.right = &h2s_rt,
	.faces.top = &h2s_up,
	.faces.bottom = &h2s_dn,
};


int engine_render_hook_post(void) {
	bench_frames++;
	return 0;
}

int main()
{

	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		if (!device_is_ready(engine_display_devices[i])) {
			LOG_ERR("Display device %d is not ready", i);
			return -ENODEV;
		}
	}

	timing_init();
	timing_start();

	k_msleep(100);
	display_blanking_on(display_device);
	display_blanking_off(display_device);

	init_engine(&process);

	scene.inf = scene_init;
	scene.pf = scene_pf;
	scene.statics = map;
	scene.statics_count = sizeof(map) / sizeof(*map);
	scene.filters_count = 1,
	scene.filters = default_scene_filters,
	scene.skybox = &skybox;

	engine_switchscene(&logo_scene);
	k_msleep(2000);

	bench_passed = lv_label_create(lv_screen_active());
	lv_label_set_text_fmt(bench_passed, "");
	lv_obj_align(bench_passed, LV_ALIGN_TOP_LEFT, CONFIG_RESOLUTION_X - 4*4, 0);

	engine_switchscene(&scene);
	engine_statics_enabled(true);

	controls.speedmul = 1;

	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
