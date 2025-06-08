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

#define ENGINE_VISUAL_UNUSED		0
#define ENGINE_VISUAL_MODEL			1
#define ENGINE_VISUAL_BILLBOARD	2

typedef struct Engine_object_s Engine_object;

typedef void (*Engine_object_pf)(Engine_object *self, void * data);

typedef struct Engine_object_s {
	union {
		L3_Model3D		model;
		L3_Billboard	billboard;
	} visual;
	uint8_t			visual_type;
	L3_Unit	view_range;
	Engine_object_pf	process;
	void *data;
} Engine_object;

int init_engine(void);

/* add object to object list, returns pointer to the instance in the table
 */
Engine_object *engine_add_object(Engine_object object);

/* remove the object (simply mark object unused)
 */
int engine_remove_object(Engine_object *object);

/* moves object around to clean up table and improve performance
 */
int engine_optimize_object_table(void);

size_t engine_object_getcnt(void);
