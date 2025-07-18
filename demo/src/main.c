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

#include "demo.h"

#include "flame.h"

#include "rain.h"
#include "smoke.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int blit_display(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y)
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



	display_write(display_device, 0, 0, &buf_desc, buffer);
	return 0;
}

typedef struct {
	L3_Unit vx;
	L3_Unit vy;
	L3_Unit z;
	L3_Unit x;
	L3_Unit jump;
	L3_Unit xrot;
} Controls;

static Engine_Object* player = 0;
static L3_Vec4 player_colpoint[2] = {{.x = 0, .y = 0, .z = 0, .w = L3_F }, {.x = 0, .y = L3_F, .z = 0, .w = L3_F }};
static Controls controls = {0};

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

static void process() {
	if (player != 0) {
		player->visual.transform.rotation.y += controls.vy;
		if (player->visual.transform.rotation.y > 256) player->visual.transform.rotation.y = -256;
		if (player->visual.transform.rotation.y < -256) player->visual.transform.rotation.y = 256;
		controls.xrot += controls.vx;
		L3_Unit x = (controls.z * L3_cos(player->visual.transform.rotation.y + L3_F/4)) / L3_F;
		L3_Unit z = (controls.z * L3_sin(player->visual.transform.rotation.y + L3_F/4)) / L3_F;
		x -= (controls.x * L3_cos(player->visual.transform.rotation.y)) / L3_F;
		z -= (controls.x * L3_sin(player->visual.transform.rotation.y)) / L3_F;


		engine_dynamic_objects[0].physics.speeds.translation.z += z;
		engine_dynamic_objects[0].physics.speeds.translation.x += x;
		engine_dynamic_objects[0].physics.speeds.translation.x *= 0.85;
		engine_dynamic_objects[0].physics.speeds.translation.z *= 0.85;
		engine_dynamic_objects[0].physics.speeds.translation.y *= 0.95;
		//engine_dynamic_objects[0].physics.speeds.translation.x -= 1;
		//player->visual.transform.translation.z += z;
		//player->visual.transform.translation.x += x;
		engine_dynamic_objects[0].physics.speeds.translation.y -= E_SPEED(5*L3_F);

		engine_dynamic_objects[0].physics.speeds.translation.y += controls.jump;

		L3_Camera *camera = engine_getcamera();
		camera->transform = player->visual.transform;
		camera->transform.translation.y += L3_F;
		if (controls.xrot > 100) controls.xrot = 100;
		if (controls.xrot < -100) controls.xrot = -100;
		camera->transform.rotation.x = controls.xrot;
		//printf("ppos: %d, %d, %d\n", player->visual.transform.translation.x, player->visual.transform.translation.y, player->visual.transform.translation.z);
	}

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
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.y = player->visual.transform.translation.y + 3 * L3_F;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.translation.x = player->visual.transform.translation.x + sys_rand16_get() / 0x10 - 0x7ff;
	transform.translation.z = player->visual.transform.translation.z + sys_rand16_get() / 0x10 - 0x7ff;
	engine_create_particle(transform, &particle_rain, rain.billboard, E_LIFESPAN(0.3));
	transform.scale.x = 128;
	transform.scale.y = 128;
	transform.translation.x = sys_rand8_get() - 128;
	transform.translation.y = 512;
	transform.translation.z = sys_rand8_get() - 128;
	engine_create_particle(transform, &particle_smoke, smoke.billboard, E_LIFESPAN(2.0));
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
	//display_set_contrast(display_device, 32);

	init_engine(&process);

	/* 'player' object */
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,0,-3*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);
	engine_dynamic_objects[0].object = player;
	engine_dynamic_objects[0].physics.transform = &(player->visual.transform);
	engine_dynamic_objects[0].physics.last_transform = player->visual.transform;
	engine_dynamic_objects_count = 1;
	engine_dynamic_objects[0].physics.pointOffsetsCount = 2;
	engine_dynamic_objects[0].physics.pointOffsets = player_colpoint;

	engine_set_statics(demo_scene, sizeof(demo_scene) / sizeof(*demo_scene));
	engine_statics_enabled(true);

	/*for (int i = 0; i < 1000; i++) {
		tmp.visual = cat;
		int x, y, z;
		x = sys_rand8_get() / 4 - 32;
		y = sys_rand8_get() / 8 - 16;
		z = sys_rand8_get() / 4 - 32;
		L3_transform3DSet(x*L3_F,y*L3_F,z*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
		tmp.view_range = 50 * L3_F;
		tmp.visual_type = ENGINE_VISUAL_BILLBOARD;
		tmp.process = 0;
		engine_add_object(tmp);
	}*/
/*
#define aG 45
	for (int i = 0; i < aG; i++) {
		for (int j = 0; j < aG; j++) {
			tmp.visual = building_01;
			tmp.visual.config.visible = L3_VISIBLE_TEXTURED;
			int x, y, z;
			x = -200 + i * 30;
			y = 180;
			z = -200 + j * 30;
			L3_transform3DSet(x*L3_F,y,z*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
			tmp.view_range = 70 * L3_F;
			tmp.visual_type = ENGINE_VISUAL_MODEL;
			tmp.process = 0;
			engine_add_object(tmp);
		}
	}*/

	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
