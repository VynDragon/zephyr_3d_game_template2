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

/* basic frame-based animation for when not procedural
 * it is possible to use multiple per object (multiple entries in the animation with each one type) */
typedef struct ObjectProcess_FrameArray_s {
	uint8_t	type;
	size_t	len;
	bool	loop;
	union {
		/* 3D movement animation, type 0 absolute, type 1 relative (+= on values instead of =)*/
		L3_Transform3D	*transforms;
		/* 2D billboard animation, type 4*/
		L3_Billboard	**billboards;
	};
} ObjectProcess_FrameArray;

void utility_animation_objectprocess_framearray(Engine_Object *object, void* data, uint64_t frame_id);
