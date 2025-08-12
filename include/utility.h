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
	int						framerate;
	uint32_t				frame_counter;
	int						error;
	uint32_t				len;
	bool					loop;
} Animation;

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
	size_t							len;
	bool							loop;
	ObjectProcess_FrameArray_Frame	*frames;
} ObjectProcess_FrameArray;

void utility_animation_objectprocess_framearray(Engine_Object *object, void* data, uint64_t frame_id);
