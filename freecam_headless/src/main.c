#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define ENGINE_BLIT_FUNCTION blit_display

#include "engine.h"
#include "logo_scene.h"

#include "map.h"

#include "filters.h"


int blit_display(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
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

static void process() {
	if (player != 0) {
		L3_Camera *camera = engine_getcamera();
		camera->transform = player->visual.transform;
	}
}

void scene_init(void *data)
{
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,1.5*L3_F,-20*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	player = engine_add_object(tmp);

	L3_Camera *camera = engine_getcamera();
	camera->focalLength = 288;
}
void scene_pf(Engine_Scene *self)
{
}

Engine_Scene scene = {0};

static Filter_f default_scene_filters[] = {
	filter_fixgap,
};

L3_COLORTYPE sky_clearpix(L3_Unit x, L3_Unit y)
{
	return ((L3_RESOLUTION_Y - y) * 0x28) / L3_RESOLUTION_Y + (abs(L3_RESOLUTION_X / 2 - x) * 0x28) / (L3_RESOLUTION_X / 2);
}

int main()
{
	timing_init();
	timing_start();

	k_msleep(100);

	init_engine(&process);

	scene.inf = scene_init;
	scene.pf = scene_pf;
	scene.statics = map;
	scene.statics_count = sizeof(map) / sizeof(*map);
	scene.filters_count = 1,
	scene.filters = default_scene_filters,
	scene.clear_pix_func = sky_clearpix;

	engine_switchscene(&logo_scene);
	k_msleep(2000);

	engine_switchscene(&scene);
	engine_statics_enabled(true);

	printf("obj cnt: %d\n", engine_object_getcnt() + engine_statics_getcnt());

	return 0;
}
