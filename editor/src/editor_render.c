#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/input/input.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <math.h>

#include "editor.h"

#include "engine.h"

#include "cube.h"

#include "plane.h"

#include "sphere.h"

#define ENUMERATE_DISPLAY_DEVS(node_id, prop, idx) DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)),

const struct device *display_devices[DT_ZEPHYR_DISPLAYS_COUNT] = {
	DT_FOREACH_PROP_ELEM(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_displays), displays, ENUMERATE_DISPLAY_DEVS)
};

static struct k_thread render_colliders_thread;
K_THREAD_STACK_DEFINE(render_colliders_thread_stack, CONFIG_RENDER_THREAD_STACK);

L3_Object editor_colliders_objects_inst[L3_MAX_OBJECTS] = {0};

int blit_display2(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
	buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;

	display_write(display_devices[1], 0, 0, &buf_desc, buffer);
	return 0;
}

static L3_Object build_collider_representation(const Engine_Object *object, const E_Collider *collider)
{
	L3_Object out;

	out.transform = object->visual.transform;
	out.config.backfaceCulling = 0;
	out.config.visible = L3_VISIBLE_WIREFRAME;
	switch (collider->type) {
		case ENGINE_COLLIDER_CUBE:
			out.model = &cube_model;
			out.transform.translation.x += collider->cube.offset.x;
			out.transform.translation.y += collider->cube.offset.y;
			out.transform.translation.z += collider->cube.offset.z;
			out.transform.scale.x = (out.transform.scale.x * collider->cube.size.x * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.y * collider->cube.size.y * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.z * collider->cube.size.z * 1) / L3_F;
			break;
		case ENGINE_COLLIDER_SPHERE:
			out.model = &sphere_model;
			out.transform.translation.x += collider->sphere.offset.x;
			out.transform.translation.y += collider->sphere.offset.y;
			out.transform.translation.z += collider->sphere.offset.z;
			out.transform.scale.x = (out.transform.scale.x * collider->sphere.size * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.x * collider->sphere.size * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.x * collider->sphere.size * 1) / L3_F;
			break;
		case ENGINE_COLLIDER_CAPSULE:
			out.model = &sphere_model;
			out.transform.translation.x += collider->capsule.offset.x;
			out.transform.translation.y += collider->capsule.offset.y;
			out.transform.translation.z += collider->capsule.offset.z;
			out.transform.scale.x = (out.transform.scale.x * collider->capsule.size.x * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.y * collider->capsule.size.y * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.z * collider->capsule.size.z * 1) / L3_F;
			break;
		case ENGINE_COLLIDER_APLANEX:
			out.model = &plane_model;
			out.transform.translation.x += collider->axisplane.offset.x;
			out.transform.translation.y += collider->axisplane.offset.y;
			out.transform.translation.z += collider->axisplane.offset.z;
			out.transform.rotation.z += 128;
			out.transform.scale.x = (out.transform.scale.x * collider->axisplane.size.y * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.y * collider->axisplane.size.x * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.z * collider->axisplane.size.z * 1) / L3_F;
			break;
		case ENGINE_COLLIDER_APLANEY:
			out.model = &plane_model;
			out.transform.translation.x += collider->axisplane.offset.x;
			out.transform.translation.y += collider->axisplane.offset.y;
			out.transform.translation.z += collider->axisplane.offset.z;
			out.transform.scale.x = (out.transform.scale.x * collider->axisplane.size.x * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.y * collider->axisplane.size.y * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.z * collider->axisplane.size.z * 1) / L3_F;
			break;
		case ENGINE_COLLIDER_APLANEZ:
			out.model = &plane_model;
			out.transform.translation.x += collider->axisplane.offset.x;
			out.transform.translation.y += collider->axisplane.offset.y;
			out.transform.translation.z += collider->axisplane.offset.z;
			out.transform.rotation.x += 128;
			out.transform.scale.x = (out.transform.scale.x * collider->axisplane.size.x * 1) / L3_F;
			out.transform.scale.y = (out.transform.scale.y * collider->axisplane.size.z * 1) / L3_F;
			out.transform.scale.z = (out.transform.scale.z * collider->axisplane.size.y * 1) / L3_F;
			break;
	}
	return out;
}

static void build_render_list(void)
{
	const L3_Object **render_o = L3_OBJECTS;
	int o_cnt = 0;
	L3_Vec4 forward = {0, 0, L3_F, L3_F};
	L3_Mat4 transMat;

	L3_makeRotationMatrixZXY(L3_SCENE.camera.transform.rotation.x,
							L3_SCENE.camera.transform.rotation.y,
							L3_SCENE.camera.transform.rotation.z,
							transMat);

	L3_vec3Xmat4(&forward, transMat);

	for (int i = 0; i < engine_object_getcnt(); i++) {
		if (engine_getobjects()[i].visual_type > ENGINE_VISUAL_UNUSED) {
			if (o_cnt >= L3_MAX_OBJECTS) break;
			if (engine_getobjects()[i].view_range + 32000 <= L3_distanceManhattan(engine_getobjects()[i].visual.transform.translation, L3_SCENE.camera.transform.translation)) continue;
			if (engine_getobjects()[i].collisions != 0) {
				for (int j = 0; j < engine_getobjects()[i].collisions->colliderCount; j++) {
					editor_colliders_objects_inst[o_cnt] = build_collider_representation(&(engine_getobjects()[i]), &(engine_getobjects()[i].collisions->colliders[j]));
					*render_o = &(editor_colliders_objects_inst[o_cnt]);
					render_o++;
					o_cnt++;
				}
			}
		}
	}
	L3_SCENE.objectCount = o_cnt;
}

static void render_colliders_function(void *, void *, void *)
{
	k_timepoint_t timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_RENDER_FPS);

	while (1) {
		timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_RENDER_FPS);
		/* clear viewport to black */
		L3_newFrame();
		L3_clearScreen(0);

		build_render_list();

		/*uint32_t drawnTriangles = */L3_drawScene(L3_SCENE);


		blit_display2(L3_video_buffer, L3_RESOLUTION_X, L3_RESOLUTION_Y);
		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
			k_yield();
		}
		/* force unready thread to avoid monopolization of CPU time */
		k_sleep(K_NSEC(10));
		k_yield();
	}
}

int init_editor_render(void)
{
	k_thread_create(&render_colliders_thread, render_colliders_thread_stack, CONFIG_RENDER_THREAD_STACK,
					render_colliders_function, NULL, NULL, NULL,
					15, 0, K_NO_WAIT);
	return 0;
}
