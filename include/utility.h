#pragma once

#include "engine.h"

typedef void (*AnimationObjectProcess)(Engine_Object *object, void* data, uint64_t frame_id);

typedef struct Animation_s {
	Engine_Object			**objects;
	void					**objects_data;
	AnimationObjectProcess	*pf;
	size_t					animated;
	int64_t					last_tick;
	bool					started;
	bool					finished;
	int						framerate;
	uint32_t				frame_counter;
	int						error;
	uint32_t				len;
	bool					loop;
} Animation;

#define ANIMATION_INIT(_objects, _objects_data, _pf, _animated, _framerate, _len, _loop) \
{												\
	.objects = _objects,						\
	.objects_data = _objects_data,				\
	.pf = _pf,									\
	.animated = _animated,						\
	.last_tick = 0,								\
	.started = false,							\
	.finished = false,							\
	.framerate = _framerate,					\
	.frame_counter = 0,							\
	.error = 0,									\
	.len = _len,								\
	.loop = _loop,								\
	.finished = false,							\
}

void utility_animation_process(Animation *animation);

typedef struct ObjectProcess_FrameArray_Frame_s {
	uint8_t	type;
	union {
		/* type 0 is nothing */
		/* 3D movement animation, type 1 absolute, type 2 relative (+= on values instead of =)*/
		L3_Transform3D	transform;
		/* 2D billboard animation, type 4*/
		L3_Billboard	*billboard;
	};
} ObjectProcess_FrameArray_Frame;

/* basic frame-based animation for when not procedural
 * it is possible to use multiple per object (multiple entries in the animation with each one type) */
typedef struct ObjectProcess_FrameArray_s {
	size_t									len;
	bool									loop;
	const ObjectProcess_FrameArray_Frame	*frames;
} ObjectProcess_FrameArray;

void utility_animation_objectprocess_framearray(Engine_Object *object, void* data, uint64_t frame_id);

typedef struct Object_3DBillboard_s {
	L3_Billboard		billboard;
	const L3_Texture	*front;
	const L3_Texture	*back;
	const L3_Texture	*left;
	const L3_Texture	*right;
	const L3_Texture	*front_35;
	const L3_Texture	*back_35;
	const L3_Texture	*left_35;
	const L3_Texture	*right_35;
	L3_Transform3D		transform;
} Object_3DBillboard;

void utility_objectprocess_3DBillboard(Engine_Object *self, void *data);
