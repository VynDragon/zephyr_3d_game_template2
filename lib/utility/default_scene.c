#include <zephyr/kernel.h>
#include "engine.h"
#include "default_scene.h"
#include "filters.h"

static void default_scene_init(void *data);
static void default_scene_pf(Engine_Scene *self);

static struct Default_scene_data default_scene_data = {0};

static Filter_f default_scene_filters[] = {
	filter_fixgap,
};

Engine_Scene default_scene = {
	.pf = default_scene_pf,
	.statics = NULL,
	.statics_count = 0,
	.inf = default_scene_init,
	.data = &default_scene_data,
	.filters_count = 1,
	.filters = default_scene_filters,
};

static L3_Vec4 player_colpoint[2] = {{.x = 0, .y = 0, .z = 0, .w = L3_F }, {.x = 0, .y = L3_F, .z = 0, .w = L3_F }};

static void default_scene_init(void *data)
{
	struct Default_scene_data *scene_data = (struct Default_scene_data*)data;
	/* 'player' object */
	Engine_Object tmp = {0};
	L3_transform3DSet(0 * L3_F,0,0*L3_F,0,0,0,L3_F,L3_F,L3_F,&(tmp.visual.transform));
	tmp.view_range = 16 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_NOTHING;
	scene_data->player = engine_add_object(tmp);
	engine_dynamic_objects[0].object = scene_data->player;
	engine_dynamic_objects[0].physics.transform = &(scene_data->player->visual.transform);
	engine_dynamic_objects[0].physics.last_transform = scene_data->player->visual.transform;
	engine_dynamic_objects_count = 1;
	engine_dynamic_objects[0].physics.pointOffsetsCount = 2;
	engine_dynamic_objects[0].physics.pointOffsets = player_colpoint;

	L3_Camera *camera = engine_getcamera();
	camera->transform.translation.y = 0 * L3_F;
	camera->transform.translation.z = 0 * L3_F;
	camera->focalLength = 256;
}

static void default_scene_pf(Engine_Scene *self)
{
	struct Default_scene_data *scene_data = (struct Default_scene_data*)self->data;
	if (scene_data->player != 0) {
		scene_data->player->visual.transform.rotation.y += scene_data->controls.vy;
		if (scene_data->player->visual.transform.rotation.y > L3_F/2) scene_data->player->visual.transform.rotation.y = -L3_F/2;
		if (scene_data->player->visual.transform.rotation.y < -L3_F/2) scene_data->player->visual.transform.rotation.y = L3_F/2;
		scene_data->player_xrot += scene_data->controls.vx;
		L3_Unit x = (scene_data->controls.z * L3_cos(scene_data->player->visual.transform.rotation.y + L3_F/4)) / L3_F;
		L3_Unit z = (scene_data->controls.z * L3_sin(scene_data->player->visual.transform.rotation.y + L3_F/4)) / L3_F;
		x -= (scene_data->controls.x * L3_cos(scene_data->player->visual.transform.rotation.y)) / L3_F;
		z -= (scene_data->controls.x * L3_sin(scene_data->player->visual.transform.rotation.y)) / L3_F;


		engine_dynamic_objects[0].physics.speeds.translation.z += z;
		engine_dynamic_objects[0].physics.speeds.translation.x += x;
		engine_dynamic_objects[0].physics.speeds.translation.x *= 0.85;
		engine_dynamic_objects[0].physics.speeds.translation.z *= 0.85;
		engine_dynamic_objects[0].physics.speeds.translation.y *= 0.95;
		//engine_dynamic_objects[0].physics.speeds.translation.x -= 1;
		//player->visual.transform.translation.z += z;
		//player->visual.transform.translation.x += x;
		engine_dynamic_objects[0].physics.speeds.translation.y -= E_SPEED(5*L3_F);

		engine_dynamic_objects[0].physics.speeds.translation.y += scene_data->controls.jump;

		L3_Camera *camera = engine_getcamera();
		camera->transform = scene_data->player->visual.transform;
		camera->transform.translation.y += L3_F;
		if (scene_data->player_xrot > 100) scene_data->player_xrot = 100;
		if (scene_data->player_xrot < -100) scene_data->player_xrot = -100;
		camera->transform.rotation.x = scene_data->player_xrot;
		//printf("ppos: %d, %d, %d\n", player->visual.transform.translation.x, player->visual.transform.translation.y, player->visual.transform.translation.z);
	}
}
