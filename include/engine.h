/*
* APACHE 2
*/


#pragma once

#include "L3.h"

#define ENGINE_BLIT_FUNCTION blit_display
int ENGINE_BLIT_FUNCTION(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y);

#define E_SPEED(speed) (speed / CONFIG_TARGET_PROCESS_FPS)
/* returns a lifespan from value in seconds */
#define E_LIFESPAN(lifespan) (lifespan * CONFIG_TARGET_PROCESS_FPS)

#define ENGINE_VISUAL_UNUSED	0
#define ENGINE_VISUAL_NOTHING	1
#define ENGINE_VISUAL_MODEL		2
#define ENGINE_VISUAL_BILLBOARD	3

#define ENGINE_MAX_COLLIDERS	 0xFF
#define ENGINE_MAX_DOBJECTS		0xF
#define ENGINE_MAX_PARTICLES	0x100

/* do product to determine if object is behind camera limit */
#define ENGINE_REAR_OBJECT_CUTOFF 1 * L3_F

typedef struct Engine_Object_s Engine_Object;
typedef struct E_Particle_s E_Particle;

/* functions ran each tick for associated object */
typedef void (*Engine_Object_pf)(Engine_Object *self, void * data);
/* functions ran each tick for associated particle */
typedef void (*Engine_Particle_pf)(E_Particle *self);
/* function ran each tick engine */
typedef void (*Engine_pf)(void);

/* Basic cuboid collider */
typedef struct E_C_Cuboid_s {
	L3_Vec4	offset;
	L3_Vec4 size;
	L3_Unit	bouncyness;
} E_C_Cuboid;

/* Basic Sphere collider */
typedef struct E_C_Sphere_s {
	L3_Vec4	offset;
	L3_Unit size;
	L3_Unit	bouncyness;
} E_C_Sphere;

/* Sphere but differnet x/y/z sizes */
typedef struct E_C_Capsule_s {
	L3_Vec4	offset;
	L3_Vec4 size;
	L3_Unit	bouncyness;
} E_C_Capsule;

/* Basic plane aligned in axis, useful for absolute walls (ground, ceiling)*/
typedef struct E_C_AxisPlane_s {
	L3_Vec4	offset;
	/* axis size define direction */
	L3_Vec4 size;
	L3_Unit	bouncyness;
	/* determine if we need to be >= than its limit for it to affect */
	bool traverseable;
} E_C_AxisPlane;

#define ENGINE_COLLIDER_NOTHING	0
#define ENGINE_COLLIDER_CUBE	1
#define ENGINE_COLLIDER_SPHERE	2
#define ENGINE_COLLIDER_CAPSULE	3
#define ENGINE_COLLIDER_APLANEX	4
#define ENGINE_COLLIDER_APLANEY	5
#define ENGINE_COLLIDER_APLANEZ	6

typedef struct E_Collider_s {
	union {
		E_C_Cuboid		cube;
		E_C_Sphere		sphere;
		E_C_Capsule		capsule;
		E_C_AxisPlane	axisplane;
	};
	uint8_t			type;
	const L3_Transform3D	*transform;
} E_Collider;

typedef struct Engine_Physics_s {
	/* pointer to visual's transform */
	L3_Transform3D	*transform;
	L3_Transform3D	speeds;
	L3_Transform3D	last_transform;
	/* points to check collisions against, if 0, use transform */
	L3_Vec4	*pointOffsets;
	uint8_t pointOffsetsCount;
} Engine_Physics;

typedef struct Engine_Collisions_s {
	/* Collider table */
	const E_Collider	*colliders;
	uint8_t				colliderCount;
} Engine_Collisions;

typedef struct Engine_Object_s {
	L3_Object				visual;
	uint8_t					visual_type;
	L3_Unit					view_range;
	Engine_Object_pf		process;
	void 					*data;
	const Engine_Collisions	*collisions;
} Engine_Object;

typedef struct Engine_DObject_s {
	Engine_Object		*object;
	Engine_Physics		physics;
} Engine_DObject;

typedef struct E_Particle_s {
	L3_Transform3D		transform;
	const L3_Billboard	*billboard;
	Engine_Particle_pf	process;
	uint32_t			life;
} E_Particle;

int init_engine(Engine_pf pf);

/* add object to object list, returns pointer to the instance in the table
 */
Engine_Object *engine_add_object(Engine_Object object);

/* remove the object (simply mark object unused)
 */
int engine_remove_object(Engine_Object *object);

/* moves object around to clean up table and improve performance
 * INVALIDATES ALL add_object REFERENCES!
 */
int engine_optimize_object_table(void);

size_t engine_object_getcnt(void);

size_t engine_statics_getcnt(void);

L3_Camera *engine_getcamera(void);

/* sets const renderlist, can be from XIP */
void engine_set_statics(const Engine_Object *objects, uint32_t count);
/* enable or disable rendering statics set */
void engine_statics_enabled(bool yes);

extern Engine_DObject engine_dynamic_objects[ENGINE_MAX_DOBJECTS];
extern uint32_t engine_dynamic_objects_count;

E_Particle *engine_create_particle(L3_Transform3D transform, Engine_Particle_pf process, const L3_Billboard *billboard, uint32_t lifespan);
