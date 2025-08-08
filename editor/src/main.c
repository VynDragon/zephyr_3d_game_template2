#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/input/input.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include "editor.h"

#include "engine.h"

#include "building.h"

L3_COLORTYPE blit_lvgl_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];

int blit_display(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y)
{	/*struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
		buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;
		display_write(display_devices[0], 0, 0, &buf_desc, buffer);*/
	memcpy(blit_lvgl_buffer, buffer, size_x*size_y);
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
		if (player->visual.transform.rotation.y > 256) player->visual.transform.rotation.y = -256;
		if (player->visual.transform.rotation.y < -256) player->visual.transform.rotation.y = 256;
		if (player->visual.transform.rotation.x > 256) player->visual.transform.rotation.x = -256;
		if (player->visual.transform.rotation.x < -256) player->visual.transform.rotation.x = 256;

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

	do_editor_UI();
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
			cont->z = E_SPEED(5*L3_F*cont->speedmul);
		else
			cont->z = 0;
	} else if (evt->code == INPUT_KEY_S) {
		if (evt->value)
			cont->z = -E_SPEED(5*L3_F*cont->speedmul);
		else
			cont->z = 0;
	}

	if (evt->code == INPUT_KEY_A) {
		if (evt->value)
			cont->x = E_SPEED(5*L3_F*cont->speedmul);
		else
			cont->x = 0;
	} else if (evt->code == INPUT_KEY_D) {
		if (evt->value)
			cont->x = -E_SPEED(5*L3_F*cont->speedmul);
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

int main()
{
	if (!device_is_ready(display_devices[0])) {
		printf("Display device not ready");
		return 0;
	}

	timing_init();
	timing_start();

	k_msleep(100);
	display_blanking_on(display_devices[0]);
	display_blanking_on(display_devices[1]);
	//display_blanking_on(display_devices[2]);
	//display_set_pixel_format(display_device, PIXEL_FORMAT_L_8);
	display_blanking_off(display_devices[0]);
	display_blanking_off(display_devices[1]);
	//display_blanking_off(display_devices[2]);
	//display_set_contrast(display_device, 32);

	init_engine(&process);
	engine_UI_set_area(0, 0, L3_RESOLUTION_X, L3_RESOLUTION_Y);
	init_editor_render();
	init_editor_UI();

	/* 'player' object */
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,0,-3*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);

	//engine_set_statics(demo_scene, sizeof(demo_scene) / sizeof(*demo_scene));
	//engine_statics_enabled(true);

	controls.speedmul = 1;

	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
