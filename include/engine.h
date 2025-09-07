/*
* APACHE 2
*/


#pragma once

#include "L3.h"

#ifndef DT_ZEPHYR_DISPLAYS_COUNT
#define DT_ZEPHYR_DISPLAYS_COUNT 1
#endif

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

#if DT_ZEPHYR_DISPLAYS_COUNT > 1
#warning UI Display may end up being wrong, display > 1
#endif

typedef struct Engine_Object_s Engine_Object;
typedef struct E_Particle_s E_Particle;
typedef struct Engine_Scene_s Engine_Scene;

/* functions ran each tick for associated object */
typedef void (*Engine_Object_pf)(Engine_Object *self, void * data);
/* functions ran each tick for associated particle */
typedef void (*Engine_Particle_pf)(E_Particle *self);
/* function ran each tick for scene*/
typedef void (*Engine_Scene_pf)(Engine_Scene *self);
/* function ran each tick engine */
typedef void (*Engine_pf)(void);

/* function to initialize a scene (spawn dynamic objects etc) */
typedef void (*Engine_Scene_inf)(void *data);

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

typedef struct E_C_Terrain_s {
	const L3_Vec4	*points;
	size_t			points_cnt;
	L3_Unit			bouncyness;
	/* determine if we need to be >= than its limit for it to affect */
	bool traverseable;
} E_C_Terrain;

#define ENGINE_COLLIDER_NOTHING	0
#define ENGINE_COLLIDER_CUBE	1
#define ENGINE_COLLIDER_SPHERE	2
#define ENGINE_COLLIDER_CAPSULE	3
#define ENGINE_COLLIDER_APLANEX	4
#define ENGINE_COLLIDER_APLANEY	5
#define ENGINE_COLLIDER_APLANEZ	6
#define ENGINE_COLLIDER_TERRAIN	7

typedef struct E_Collider_s {
	union {
		E_C_Cuboid		cube;
		E_C_Sphere		sphere;
		E_C_Capsule		capsule;
		E_C_AxisPlane	axisplane;
		E_C_Terrain		terrain;
	};
	uint8_t			type;
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

typedef struct Engine_Scene_s {
	Engine_Scene_pf		pf;
	const Engine_Object	*statics;
	size_t				statics_count;
	Engine_Scene_inf	inf;
	void				*data;
} Engine_Scene;

int init_engine(Engine_pf pf);

int init_engine_UI(void);

int engine_render_UI(void);

int engine_render_hook(void);

void engine_UI_set_area(int32_t x, int32_t y, int32_t w, int32_t h);

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

Engine_Object *engine_getobjects(void);

size_t engine_statics_getcnt(void);

L3_Camera *engine_getcamera(void);

/* Scene Management */
Engine_Scene *engine_getscene(void);
/* Blank slate the engine scene */
int	engine_cleanscene(void);
/* Load scene as the engine scene */
int	engine_initscene(Engine_Scene *scene);
/* Blank engine scene then load scene as the engine scene */
int	engine_switchscene(Engine_Scene *scene);


/* sets const renderlist, can be from XIP */
void engine_set_statics(const Engine_Object *objects, uint32_t count);
/* enable or disable rendering statics set */
void engine_statics_enabled(bool yes);

extern Engine_DObject engine_dynamic_objects[ENGINE_MAX_DOBJECTS];
extern uint32_t engine_dynamic_objects_count;
extern const struct device *engine_display_devices[DT_ZEPHYR_DISPLAYS_COUNT];
extern uint32_t engine_drawnTriangles;

E_Particle *engine_create_particle(L3_Transform3D transform, Engine_Particle_pf process, const L3_Billboard *billboard, uint32_t lifespan);


