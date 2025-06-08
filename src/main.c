#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define ENGINE_BLIT_FUNCTION blit_display

#include "engine.h"

#include "building.h"

#include "cat.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int blit_display(L3_COLORTYPE *buffer, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
	buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;

	display_write(display_device, 0, 0, &buf_desc, buffer);
	return 0;
}

static int flip = 1;
void do_move(Engine_object *self, void* data) {
	self->visual.billboard.transform.translation.z += *(int*)data;
	if (self->visual.billboard.transform.translation.z > 200)
		*(int*)data = -*(int*)data;
	if (self->visual.billboard.transform.translation.z < 64)
		*(int*)data = -*(int*)data;
}

void do_rotate_1(Engine_object *self, void *data) {
	self->visual.model.transform.rotation.y += 1;
}

void do_rotate_2(Engine_object *self, void *data) {
	self->visual.model.transform.rotation.x += 1;
}

void do_rotate_3(Engine_object *self, void *data) {
	self->visual.model.transform.rotation.z += 1;
}

int main()
{
	if (!device_is_ready(display_device)) {
		printf("Display device not ready");
		return 0;
	}

	timing_init();
	timing_start();

	k_msleep(100);
	display_blanking_on(display_device);
	display_set_pixel_format(display_device, PIXEL_FORMAT_L_8);
	display_blanking_off(display_device);

	init_engine();

	Engine_object tmp = {0};
	tmp.visual.model = building_01;
	L3_transform3DSet(0,0,400,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(tmp.visual.model.transform));
	tmp.view_range = 8192;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	void *rm = engine_add_object(tmp);

	tmp.visual.model = building_01;
	L3_transform3DSet(0,0,400,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(tmp.visual.model.transform));
	tmp.view_range = 8192;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = &do_rotate_1;
	engine_add_object(tmp);

	tmp.visual.model = building_01;
	L3_transform3DSet(-256,0,400,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(tmp.visual.model.transform));
	tmp.visual.model.config.visible = L3_VISIBLE_WIREFRAME;
	tmp.view_range = 8192;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = &do_rotate_2;
	engine_add_object(tmp);

	tmp.visual.model = building_01;
	L3_transform3DSet(256,0,400,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(tmp.visual.model.transform));
	tmp.visual.model.config.visible = L3_VISIBLE_SOLID | L3_VISIBLE_DISTANCELIGHT;
	tmp.view_range = 8192;
	tmp.visual_type = ENGINE_VISUAL_MODEL;
	tmp.process = &do_rotate_3;
	engine_add_object(tmp);

	tmp.visual.billboard = cat;
	L3_transform3DSet(0,0,128,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(tmp.visual.billboard.transform));
	tmp.view_range = 192;
	tmp.visual_type = ENGINE_VISUAL_BILLBOARD;
	tmp.data = &flip;
	tmp.process = &do_move;
	engine_add_object(tmp);

	printf("obj cnt: %d\n", engine_object_getcnt());
	engine_remove_object(rm);
	printf("obj cnt: %d\n", engine_object_getcnt());
	engine_optimize_object_table();
	printf("obj cnt: %d\n", engine_object_getcnt());

	/*L3_BILLBOARDS[0] = cat;
	L3_transform3DSet(origbillboard[0][0],0,origbillboard[0][1],0,0,0,L3_F*4*2,L3_F*2,L3_F*2,&(L3_BILLBOARDS[0].transform));*/

	return 0;
}
