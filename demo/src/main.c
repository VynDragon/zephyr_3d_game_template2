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
#include "default_scene.h"
#include "logo_scene.h"

#include "demo.h"

#include "particles/flame.h"

#include "rain.h"
#include "particles/smoke.h"

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

void particle_fire(E_Particle *self)
{
	self->transform.translation.y += 24;
	self->transform.scale.x *= 0.96;
	self->transform.scale.y *= 0.96;
}

void particle_rain(E_Particle *self)
{
	self->transform.translation.y -= 256;
	if (self->transform.translation.y < 0) self->life = 1;
}
void particle_smoke(E_Particle *self)
{
	self->transform.translation.y += 24;
	self->transform.scale.x *= 0.98;
	self->transform.scale.y *= 0.98;
}

static Controls controls = {0};

static void process() {
	if (engine_getscene() == &default_scene) {
		struct Default_scene_data *scene_data = (struct Default_scene_data*)default_scene.data;

		/* in a real game this would be in the scene's pf */
		scene_data->controls = controls;
		L3_Transform3D transform;
		transform.scale.x = 192;
		transform.scale.y = 192;
		transform.translation.x = sys_rand8_get() - 128;
		transform.translation.y = 128;
		transform.translation.z = sys_rand8_get() - 128;
		engine_create_particle(transform, &particle_fire, flame.billboard, E_LIFESPAN(0.4));
		transform.translation.x = sys_rand8_get() - 128;
		transform.translation.y = 128;
		transform.translation.z = sys_rand8_get() - 128;
		engine_create_particle(transform, &particle_fire, flame.billboard, E_LIFESPAN(0.4));
		transform.scale.x = 128;
		transform.scale.y = 128;
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.y = scene_data->player->visual.transform.translation.y + 3 * L3_F;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.translation.x = scene_data->player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
		transform.translation.z = scene_data->player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
		engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
		transform.scale.x = 128;
		transform.scale.y = 128;
		transform.translation.x = sys_rand8_get() - 128;
		transform.translation.y = 512;
		transform.translation.z = sys_rand8_get() - 128;
		engine_create_particle(transform, &particle_smoke, smoke.billboard, E_LIFESPAN(2.0));
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
			cont->z = E_SPEED(3*L3_F);
		else
			cont->z = 0;
	} else if (evt->code == INPUT_KEY_S) {
		if (evt->value)
			cont->z = -E_SPEED(3*L3_F);
		else
			cont->z = 0;
	}

	if (evt->code == INPUT_KEY_A) {
		if (evt->value)
			cont->x = E_SPEED(3*L3_F);
		else
			cont->x = 0;
	} else if (evt->code == INPUT_KEY_D) {
		if (evt->value)
			cont->x = -E_SPEED(3*L3_F);
		else
			cont->x = 0;
	}

	if (evt->code == INPUT_KEY_SPACE) {
		if (evt->value)
			cont->jump = E_SPEED(15*L3_F);
		else
			cont->jump = 0;
	}
}

INPUT_CALLBACK_DEFINE(0, update_controls, &controls);

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
	//display_set_pixel_format(display_device, PIXEL_FORMAT_L_8);
	display_blanking_off(display_device);

	init_engine(&process);

	default_scene.statics = demo_scene;
	default_scene.statics_count = sizeof(demo_scene) / sizeof(*demo_scene);

	engine_switchscene(&logo_scene);
	struct Default_scene_data *scene_data = (struct Default_scene_data*)default_scene.data;
	k_msleep(2000);

	engine_switchscene(&default_scene);
	scene_data->player->visual.transform.translation.z -= L3_F*4;

	/*engine_set_statics(demo_scene, sizeof(demo_scene) / sizeof(*demo_scene));
	engine_statics_enabled(true);*/


	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
