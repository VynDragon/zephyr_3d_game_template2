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

static uint32_t bench_frames;
float engine_rFPS_avg;
int last_t = -1;

#define BAR_LEN 20

void print_progress_bar(size_t value, size_t min, size_t max, size_t len)
{
	size_t t = ((value - min) * len) / max;

	printf("\e[1A[");
	for (int i = 0; i < len; i++) {
		if (t >= i) {
			printf("=");
		} else {
			printf(" ");
		}
	}
	printf("]\n");
}

void scene_pf(Engine_Scene *self)
{
	int t = (camera_animation.frame_counter * BAR_LEN) / ARRAY_SIZE(camera_animation_impl_frames);

	if (!camera_animation.started)
	{
		bench_frames = 0;
	}
	if (camera_animation.finished) {
		camera_animation.finished = false;
		camera_animation.started = false;
#if defined(CONFIG_FPU)
		printf("Average Render Frame/Second (RFPS): %f\n\n", (double)engine_rFPS_avg);
#else
		printf("Average Render Frame/Second (RFPS): %d\n\n", (int)engine_rFPS_avg);
#endif
		L3_transform3DSet(0 * L3_F,1.5*L3_F,-20*L3_F,0,0,0,L3_F,L3_F,L3_F, &player->visual.transform);
	} else {
		if (bench_frames > 0) {
			engine_rFPS_avg = (engine_rFPS_avg * (bench_frames - 1) + engine_rFPS) / bench_frames;
		}
		if (t != last_t) {
			print_progress_bar(camera_animation.frame_counter, 0, ARRAY_SIZE(camera_animation_impl_frames), BAR_LEN);
			last_t = t;
		}
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
	scene.skybox = &skybox;

	engine_switchscene(&logo_scene);
	k_msleep(100);

	engine_switchscene(&scene);
	engine_statics_enabled(true);

	printf("Benching...\n\n");

	return 0;
}
