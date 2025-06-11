/*
* APACHE 2
*/


#pragma once

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "L3.h"

#define ENGINE_BLIT_FUNCTION blit_display
int ENGINE_BLIT_FUNCTION(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y);

#define ENGINE_VISUAL_UNUSED	0
#define ENGINE_VISUAL_MODEL		1
#define ENGINE_VISUAL_BILLBOARD	2
#define ENGINE_VISUAL_NOTHING	3

#define ENGINE_COLLISION_UNUSED		0
#define ENGINE_COLLISION_FLATPLANE	1
#define ENGINE_COLLISION_CUBOID		2

typedef struct Engine_object_s Engine_object;

/* functions ran each tick for associated object */
typedef void (*Engine_object_pf)(Engine_object *self, void * data);
/* function ran each tick engine */
typedef void (*Engine_pf)(void);

typedef struct E_C_Flatplane_s {
	bool	negative;
	L3_Vec4	a, b, c , d;
} E_C_Flatplane;

typedef struct E_C_Cuboid_s {
	L3_Vec4	a, b, c, d, e, f, g, h;
} E_C_Cuboid;

typedef struct Engine_object_s {
	L3_Object			visual;
	uint8_t				visual_type;
	L3_Unit				view_range;
	Engine_object_pf	process;
	void 				*data;
} Engine_object;

int init_engine(Engine_pf pf);

/* add object to object list, returns pointer to the instance in the table
 */
Engine_object *engine_add_object(Engine_object object);

/* remove the object (simply mark object unused)
 */
int engine_remove_object(Engine_object *object);

/* moves object around to clean up table and improve performance
 * INVALIDATES ALL add_object REFERENCES!
 */
int engine_optimize_object_table(void);

size_t engine_object_getcnt(void);

L3_Camera *engine_getcamera(void);
