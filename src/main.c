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

#define ENGINE_BLIT_FUNCTION blit_display

#include "engine.h"

#include "building.h"

#include "cat.h"

#include "cube.h"

#include "plane.h"

#include "sphere.h"

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

static int flip = 1;
void do_move(Engine_Object *self, void* data) {
	self->visual.transform.translation.z += *(int*)data;
	if (self->visual.transform.translation.z > 200)
		*(int*)data = -*(int*)data;
	if (self->visual.transform.translation.z < 64)
		*(int*)data = -*(int*)data;
}

static void do_rotate_1(Engine_Object *self, void *data) {
	self->visual.transform.rotation.y += 1;
}

static void do_rotate_2(Engine_Object *self, void *data) {
	self->visual.transform.rotation.x += 1;
}

static void do_rotate_3(Engine_Object *self, void *data) {
	self->visual.transform.rotation.z += 1;
}

typedef struct {
	L3_Unit vx;
	L3_Unit vy;
	L3_Unit z;
	L3_Unit x;
	L3_Unit jump;
} Controls;

static Engine_Object* player = 0;
static L3_Vec4 player_colpoint[2] = {{.x = 0, .y = 0, .z = 0, .w = L3_F }, {.x = 0, .y = L3_F, .z = 0, .w = L3_F }};
static Controls controls = {0};
static E_Collider collider_test[7][1] = {0};
static Engine_Collisions collision_test[7] = {0};

static void process() {
	if (player != 0) {
		player->visual.transform.rotation.y += controls.vy;
		player->visual.transform.rotation.x += controls.vx;
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
		//printf("ppos: %d, %d, %d\n", player->visual.transform.translation.x, player->visual.transform.translation.y, player->visual.transform.translation.z);
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
	if (!device_is_ready(display_device)) {
		printf("Display device not ready");
		return 0;
	}

	timing_init();
	timing_start();

	k_msleep(100);
	display_blanking_on(display_device);
	//display_set_pixel_format(display_device, PIXEL_FORMAT_L_8);
	display_blanking_off(display_device);

	init_engine(&process);

	/* 'player' object */
	Engine_Object tmp = {0};
	L3_transform3DSet(0,L3_F*10,0,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);
	engine_dynamic_objects[0].object = player;
	engine_dynamic_objects[0].physics.transform = &(player->visual.transform);
	engine_dynamic_objects[0].physics.last_transform = player->visual.transform;
	engine_dynamic_objects_count = 1;
	engine_dynamic_objects[0].physics.pointOffsetsCount = 2;
	engine_dynamic_objects[0].physics.pointOffsets = player_colpoint;


	INSTANCIATE_OBJECT(tmpm, building_01);
	tmp.visual = tmpm;
	L3_transform3DSet(0,0,10*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	void *rm = engine_add_object(tmp);

	tmp.visual = building_01;
	L3_transform3DSet(0,180,10*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	tmp.collisions = &(collision_test[1]);
	collision_test[1].colliderCount = 1;
	collision_test[1].colliders = collider_test[1];
	collider_test[1][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[1][0].cube.size.y = 2.5*L3_F;
	collider_test[1][0].cube.bouncyness = 128;
	collider_test[1][0].cube.size.x = 2.5*L3_F;
	collider_test[1][0].cube.size.z = 4.5*L3_F;
	collider_test[1][0].type = ENGINE_COLLIDER_CUBE;

	tmp.collisions = 0;

	tmp.visual = building_01;
	L3_transform3DSet(-10*L3_F,180,10*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.visual.config.visible = L3_VISIBLE_WIREFRAME;
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	engine_add_object(tmp);

	tmp.visual = building_01;
	L3_transform3DSet(10*L3_F,180,10*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.visual.config.visible = L3_VISIBLE_SOLID | L3_VISIBLE_DISTANCELIGHT;
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	engine_add_object(tmp);

	tmp.visual = cat;
	L3_transform3DSet(0,0,5*L3_F,0,0,0,L3_F*4,L3_F*4,L3_F*4,&(tmp.visual.transform));
	tmp.view_range = 12 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_BILLBOARD;
	tmp.data = &flip;
	tmp.process = 0;
	engine_add_object(tmp);

	tmp.visual = cube;
	L3_transform3DSet(L3_F*2,L3_F,2*L3_F,0,64,128,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 64 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.data = 0;
	tmp.process = 0;
	tmp.collisions = &(collision_test[0]);
	collision_test[0].colliderCount = 1;
	collision_test[0].colliders = collider_test[0];
	collider_test[0][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[0][0].cube.size.y = 0.5*L3_F;
	collider_test[0][0].cube.bouncyness = 128;
	collider_test[0][0].cube.size.x = 0.5*L3_F;
	collider_test[0][0].cube.size.z = 0.5*L3_F;
	collider_test[0][0].type = ENGINE_COLLIDER_CUBE;

	tmp.visual = sphere;
	L3_transform3DSet(-L3_F*2,0,2*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 64 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.data = 0;
	tmp.process = 0;
	tmp.collisions = &(collision_test[2]);
	collision_test[2].colliderCount = 1;
	collision_test[2].colliders = collider_test[2];
	collider_test[2][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[2][0].sphere.size = L3_F;
	collider_test[2][0].sphere.bouncyness = 0xFF;
	collider_test[2][0].type = ENGINE_COLLIDER_SPHERE;

	tmp.visual = plane;
	L3_transform3DSet(0,0*L3_F,0*L3_F,0,0,0,L3_F*100,L3_F,L3_F*100,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	tmp.process = 0;
	tmp.collisions = &(collision_test[3]);
	collision_test[3].colliderCount = 1;
	collision_test[3].colliders = collider_test[3];
	collider_test[3][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[3][0].axisplane.size.y = 1;
	collider_test[3][0].axisplane.bouncyness = 0xFF;
	collider_test[3][0].axisplane.size.x = 1*L3_F;
	collider_test[3][0].axisplane.size.z = 1*L3_F;
	collider_test[3][0].type = ENGINE_COLLIDER_APLANEY;

	tmp.visual = plane;
	L3_transform3DSet(0,128,-2*L3_F,0,0,0,L3_F*2,L3_F,L3_F*2,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	tmp.collisions = &(collision_test[4]);
	collision_test[4].colliderCount = 1;
	collision_test[4].colliders = collider_test[4];
	collider_test[4][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[4][0].axisplane.size.y = 1;
	collider_test[4][0].axisplane.bouncyness = 128;
	collider_test[4][0].axisplane.size.x = 1*L3_F;
	collider_test[4][0].axisplane.size.z = 1*L3_F;
	collider_test[4][0].type = ENGINE_COLLIDER_APLANEY;

	tmp.visual = plane;
	L3_transform3DSet(L3_F*2,L3_F*1.5,-2*L3_F,0,0,0,L3_F*2,L3_F,L3_F*2,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	tmp.collisions = &(collision_test[6]);
	collision_test[6].colliderCount = 1;
	collision_test[6].colliders = collider_test[6];
	collider_test[6][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[6][0].axisplane.size.y = 1;
	collider_test[6][0].axisplane.bouncyness = 128;
	collider_test[6][0].axisplane.size.x = 1*L3_F;
	collider_test[6][0].axisplane.size.z = 1*L3_F;
	collider_test[6][0].axisplane.traverseable = true;
	collider_test[6][0].type = ENGINE_COLLIDER_APLANEY;

	/*tmp.visual = plane;
	L3_transform3DSet(0,2*L3_F,0*L3_F,0,0,0,L3_F*10,L3_F,L3_F*10,&(tmp.visual.transform));
	tmp.view_range = 8192 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = 0;
	tmp.collisions = &(collision_test[5]);
	collision_test[5].colliderCount = 1;
	collision_test[5].colliders = collider_test[5];
	collider_test[5][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[5][0].axisplane.size.y = -1;
	collider_test[5][0].axisplane.bouncyness = 0x0;
	collider_test[5][0].axisplane.size.x = 1*L3_F;
	collider_test[5][0].axisplane.size.z = 1*L3_F;
	collider_test[5][0].type = ENGINE_COLLIDER_APLANEY;*/

	tmp.visual = sphere;
	L3_transform3DSet(-L3_F*4,0,2*L3_F,0,0,-128,L3_F,L3_F*3,L3_F*4,&(tmp.visual.transform));
	tmp.view_range = 64 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.data = 0;
	tmp.process = 0;
	tmp.collisions = &(collision_test[5]);
	collision_test[5].colliderCount = 1;
	collision_test[5].colliders = collider_test[5];
	collider_test[5][0].transform = &(engine_add_object(tmp)->visual.transform);
	collider_test[5][0].capsule.size.x = L3_F;
	collider_test[5][0].capsule.size.y = L3_F;
	collider_test[5][0].capsule.size.z = L3_F;
	collider_test[5][0].capsule.bouncyness = 0xFFF;
	collider_test[5][0].type = ENGINE_COLLIDER_CAPSULE;


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

	printf("obj cnt: %d\n", engine_object_getcnt());
	engine_remove_object(rm);
	printf("obj cnt: %d\n", engine_object_getcnt());
	engine_optimize_object_table();
	printf("obj cnt: %d\n", engine_object_getcnt());

	return 0;
}
