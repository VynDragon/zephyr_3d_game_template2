#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/input/input.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <math.h>

#include <lvgl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define ENGINE_BLIT_FUNCTION blit_display

#include "engine.h"
#include "logo_scene.h"

#include "map.h"

#include "filters.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int blit_display(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
	//uint8_t buf[128] = {0};
	//buf_desc.buf_size = size_x * size_y / 8 / 8;
	buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;

	/*for (int j = 0; j < size_y; j+= 8) {
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
		display_write(display_device, 0, j, &buf_desc, buf);
	}*/

	display_write(display_device, x, y, &buf_desc, buffer);

	return 0;
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

void scene_init(void *data)
{
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,0,-3*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);

	L3_Camera *camera = engine_getcamera();
	camera->transform.translation.y = 1 * L3_F;
	camera->transform.translation.z = -1.5 * L3_F;
	camera->focalLength = 196;
}
void scene_pf(Engine_Scene *self)
{
}

Engine_Scene scene = {0};

static Filter_f default_scene_filters[] = {
	filter_fixgap,
};

int main()
{

	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		if (!device_is_ready(engine_display_devices[i])) {
			LOG_ERR("Display device %d is not ready", i);
			return -ENODEV;
		}
	}

	display_set_contrast(engine_display_devices[0], 255);

	timing_init();
	timing_start();

	k_msleep(100);
	display_blanking_on(display_device);
	//display_set_pixel_format(display_device, PIXEL_FORMAT_L_8);
	display_blanking_off(display_device);
	//display_set_contrast(display_device, 32);

	init_engine(&process);

	scene.inf = scene_init;
	scene.pf = scene_pf;
	scene.statics = map;
	scene.statics_count = sizeof(map) / sizeof(*map);
	scene.filters_count = 1,
	scene.filters = default_scene_filters,

	engine_switchscene(&logo_scene);
	k_msleep(2000);

	engine_switchscene(&scene);
	engine_statics_enabled(true);

	controls.speedmul = 1;

	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
