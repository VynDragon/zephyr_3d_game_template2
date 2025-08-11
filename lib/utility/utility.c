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
				animation->started = false;
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
	if (array->type == 0) {
		object->visual.transform = array->transforms[frame_id];
	} else if (array->type == 1) {
		object->visual.transform.translation.x += array->transforms[frame_id].translation.x;
		object->visual.transform.translation.y += array->transforms[frame_id].translation.y;
		object->visual.transform.translation.z += array->transforms[frame_id].translation.z;
		object->visual.transform.rotation.x += array->transforms[frame_id].rotation.x;
		object->visual.transform.rotation.y += array->transforms[frame_id].rotation.y;
		object->visual.transform.rotation.z += array->transforms[frame_id].rotation.z;
		object->visual.transform.scale.x += array->transforms[frame_id].scale.x;
		object->visual.transform.scale.y += array->transforms[frame_id].scale.y;
		object->visual.transform.scale.z += array->transforms[frame_id].scale.z;
	} else if (array->type == 4) {
		object->visual.billboard = array->billboards[frame_id];
	}
}
