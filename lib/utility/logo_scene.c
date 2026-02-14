#include <zephyr/kernel.h>
#include "engine.h"
#include "logo_scene.h"
#include "models/Zephyr_Logo_model_1.h"
#include "models/Zephyr_Logo_model_2.h"

static void logo_scene_init(void *data);
static void logo_scene_pf(Engine_Scene *self);

Engine_Scene logo_scene = {
	.pf = logo_scene_pf,
	.statics = NULL,
	.statics_count = 0,
	.inf = logo_scene_init,
	.data = NULL,
};

static Engine_Object *logo_1;
static Engine_Object *logo_2;

static void logo_scene_init(void *data)
{
	/* 'player' object */
	Engine_Object tmp = {0};
	tmp.visual = Zephyr_Logo_model_1;
	tmp.view_range = 512 * L3_F;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	L3_transform3DSet(0 * L3_F,0,3*L3_F,0,0,0,4*L3_F,4*L3_F,4*L3_F,&(tmp.visual.transform));
	tmp.visual.config.visible = L3_VISIBLE_MODEL_SOLID;
	logo_1 = engine_add_object(tmp);
	tmp.visual = Zephyr_Logo_model_2;
	L3_transform3DSet(0 * L3_F,0,3*L3_F,0,0,0,4*L3_F,4*L3_F,4*L3_F,&(tmp.visual.transform));
	tmp.visual.config.visible |= L3_VISIBLE_MODEL_SOLID;
	tmp.visual.solid_color = 0x80;
	logo_2 = engine_add_object(tmp);

	L3_Camera *camera = engine_getcamera();
	camera->focalLength = 256;
}

static void logo_scene_pf(Engine_Scene *self)
{

}
