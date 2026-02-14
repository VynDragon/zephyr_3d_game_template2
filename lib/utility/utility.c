#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "utility.h"

#define FRAME_TO_MS(n) 1000 / n

void utility_animation_process(Animation *animation)
{
	if (!animation->started) {
		animation->last_tick = k_uptime_get();
		animation->frame_counter = 0;
		animation->started = true;
		animation->error = 0;
	} else {
		int64_t difference = k_uptime_get() - animation->last_tick;
		animation->last_tick = k_uptime_get();
		animation->error += difference;
		if (animation->error > FRAME_TO_MS(animation->framerate)) {
			animation->error -= FRAME_TO_MS(animation->framerate);
		} else
			return;
		animation->frame_counter += 1;
		if (animation->frame_counter >= animation->len)
		{
			if (!animation->loop) {
				animation->finished = true;
				return;
			} else {
				animation->frame_counter = 0;
			}
			return;
		}
	}
	for (int i = 0; i < animation->animated; i++) {
		animation->pf[i](animation->objects[i], animation->objects_data[i], animation->frame_counter);
	}
}

void utility_animation_objectprocess_framearray(Engine_Object *object, void* data, uint64_t frame_id)
{
	ObjectProcess_FrameArray *array = (ObjectProcess_FrameArray*)data;

	if (frame_id > array->len - 1) {
		if (array->loop) {
			frame_id = frame_id % (array->len);
		} else {
			frame_id = array->len - 1;
		}
	}
	if (array->frames[frame_id].type == 1) {
		object->visual.transform = array->frames[frame_id].transform;
	} else if (array->frames[frame_id].type == 2) {
		object->visual.transform.translation.x += array->frames[frame_id].transform.translation.x;
		object->visual.transform.translation.y += array->frames[frame_id].transform.translation.y;
		object->visual.transform.translation.z += array->frames[frame_id].transform.translation.z;
		object->visual.transform.rotation.x += array->frames[frame_id].transform.rotation.x;
		object->visual.transform.rotation.y += array->frames[frame_id].transform.rotation.y;
		object->visual.transform.rotation.z += array->frames[frame_id].transform.rotation.z;
		object->visual.transform.scale.x += array->frames[frame_id].transform.scale.x;
		object->visual.transform.scale.y += array->frames[frame_id].transform.scale.y;
		object->visual.transform.scale.z += array->frames[frame_id].transform.scale.z;
	} else if (array->frames[frame_id].type == 4) {
		object->visual.billboard = array->frames[frame_id].billboard;
	}
}

void utility_objectprocess_3DBillboard(Engine_Object *self, void *data)
{
	Object_3DBillboard *dat = data;
	L3_Mat4 matCamera, matFinal;
	L3_Vec4 forward = { 0, 0, L3_F, L3_F };
	L3_Vec4 up = { 0, L3_F, 0, L3_F };
	L3_Vec4 center = { 0, 0, 0, L3_F };
	self->visual.transform = dat->transform;

	L3_Camera *camera = engine_getcamera();
	L3_makeCameraMatrix(camera->transform, matCamera);

	L3_makeWorldMatrix(self->visual.transform, matFinal);
	L3_mat4Xmat4(matFinal,matCamera);

	L3_vec3Xmat4(&forward, matFinal);
	L3_vec3Xmat4(&up, matFinal);
	L3_vec3Xmat4(&center, matFinal);

	forward.x = forward.x - center.x;
	forward.z = forward.z - center.z;
	up.x = up.x - center.x;
	up.y = up.y - center.y;
	up.z = up.z - center.z;

	if (up.y < L3_F - 50) {
		if (abs(forward.z) > abs(forward.x)) {
			if (forward.z > 0) {
				dat->billboard.texture = dat->back_35;
			} else {
				dat->billboard.texture = dat->front_35;
			}
		} else {
			if (forward.x > 0) {
				dat->billboard.texture = dat->right_35;
			} else {
				dat->billboard.texture = dat->left_35;
			}
		}
	} else {
		if (abs(forward.z) > abs(forward.x)) {
			if (forward.z > 0) {
				dat->billboard.texture = dat->back;
			} else {
				dat->billboard.texture = dat->front;
			}
		} else {
			if (forward.x > 0) {
				dat->billboard.texture = dat->right;
			} else {
				dat->billboard.texture = dat->left;
			}
		}
	}
}
