#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);


#include "engine.h"

#include "building.h"

#include "cat.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static void render_function(void *, void *, void *)
{
	timing_t start_time, end_time, dstart_time, rend_time, rstart_time;
	uint32_t total_time_us, render_time_us, draw_time_us;
	struct display_buffer_descriptor buf_desc;
	k_timepoint_t timing = L3_FPS_TIMEPOINT(3);

	buf_desc.buf_size = L3_RESOLUTION_X * L3_RESOLUTION_Y;
	buf_desc.width = L3_RESOLUTION_X;
	buf_desc.height = L3_RESOLUTION_Y;
	buf_desc.pitch = L3_RESOLUTION_X;

	while (1) {
		timing = L3_FPS_TIMEPOINT(3);
		start_time = timing_counter_get();
		/* clear viewport to black */
		L3_newFrame();
		L3_clearScreen(0);

		rstart_time = timing_counter_get();
		uint32_t drawnTriangles = L3_drawScene(L3_SCENE);

		rend_time = timing_counter_get();
		dstart_time = timing_counter_get();
		display_write(display_device, 0, 0, &buf_desc, L3_video_buffer);
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		render_time_us = timing_cycles_to_ns(timing_cycles_get(&rstart_time, &rend_time)) / 1000;
		draw_time_us = timing_cycles_to_ns(timing_cycles_get(&dstart_time, &end_time)) / 1000;
		printf("total us: %u ms:%u fps:%u\n", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
		printf("display us:%u render us:%u render fps: %u\n", draw_time_us, render_time_us, 1000000 / (render_time_us != 0 ? render_time_us : 1));
		printf("rendered %u Polygons, %u polygons per second\n", drawnTriangles, drawnTriangles * 1000000 / (render_time_us != 0 ? render_time_us : 1));
		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
		}
		k_yield();
	}
}

static int origbillboard[4][2] = {
	{0,64},
	{0,-64},
	{64,0},
	{-64,0},
};

static void process_function(void *, void *, void *)
{
	int sinvar = 0;
	k_timepoint_t timing = L3_FPS_TIMEPOINT(10);

	while (1) {
		timing = L3_FPS_TIMEPOINT(10);
		uint32_t ticks = sys_clock_cycle_get_32();
		L3_MODELS[0].config.visible = L3_VISIBLE_SOLID;
		L3_MODELS[0].transform.rotation.y = sinvar;
		L3_MODELS[0].config.visible = L3_VISIBLE_TEXTURED | L3_VISIBLE_DISTANCELIGHT;
		L3_MODELS[1].transform.rotation.y = sinvar;
		L3_MODELS[1].config.visible = L3_VISIBLE_SOLID | L3_VISIBLE_DISTANCELIGHT;
		L3_MODELS[2].transform.rotation.y = sinvar;
		L3_MODELS[2].config.visible = L3_VISIBLE_SOLID;
		L3_MODELS[3].transform.rotation.y = sinvar;
		L3_MODELS[4].transform.rotation.y = sinvar;
		L3_MODELS[5].transform.rotation.y = sinvar;
		L3_MODELS[6].transform.rotation.y = sinvar;
		L3_MODELS[7].transform.rotation.y = sinvar;
		L3_MODELS[4].config.visible = L3_VISIBLE_WIREFRAME;
		L3_MODELS[8].transform.rotation.y = sinvar;

		for (int i = 0; i < 4; i++) {
			L3_BILLBOARDS[i].transform.translation.z = origbillboard[i][1];
			L3_BILLBOARDS[i].transform.translation.x = origbillboard[i][0];
		}

		L3_rotate2DPoint(&(L3_BILLBOARDS[0].transform.translation.z), &(L3_BILLBOARDS[0].transform.translation.x), sinvar);
		L3_rotate2DPoint(&(L3_BILLBOARDS[1].transform.translation.z), &(L3_BILLBOARDS[1].transform.translation.x), sinvar);
		L3_rotate2DPoint(&(L3_BILLBOARDS[2].transform.translation.z), &(L3_BILLBOARDS[2].transform.translation.x), sinvar);
		L3_rotate2DPoint(&(L3_BILLBOARDS[3].transform.translation.z), &(L3_BILLBOARDS[3].transform.translation.x), sinvar);

		for (int i = 0; i < 4; i++) {
			L3_BILLBOARDS[i].transform.translation.z = L3_BILLBOARDS[i].transform.translation.z + 190;
		}

		sinvar+=1;
		//L3_SCENE.camera.transform.translation.z = sinvar;
		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
		}
		k_yield();
	}
}

static struct k_thread render_thread;
K_THREAD_STACK_DEFINE(render_thread_stack, 4096);

static struct k_thread process_thread;
K_THREAD_STACK_DEFINE(process_thread_stack, 4096);

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

	L3_MODELS[0] = building_01;
	L3_MODELS[1] = building_01;
	L3_MODELS[2] = building_01;
	L3_MODELS[3] = building_01;
	L3_MODELS[4] = building_01;
	L3_MODELS[5] = building_01;
	L3_MODELS[6] = building_01;
	L3_MODELS[7] = building_01;
	L3_MODELS[8] = building_01;
	L3_MODELS[9] = building_01;

	L3_BILLBOARDS[0] = cat;
	L3_BILLBOARDS[1] = cat;
	L3_BILLBOARDS[2] = cat;
	L3_BILLBOARDS[3] = cat;

	L3_sceneInit(L3_MODELS, 9, L3_BILLBOARDS, 4, &L3_SCENE);

	L3_transform3DSet(0,0,400,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[0].transform));
	L3_transform3DSet(256,0,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[1].transform));
	L3_transform3DSet(-256,0,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[2].transform));
	L3_transform3DSet(0,256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[3].transform));
	L3_transform3DSet(0,-256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[4].transform));
	L3_transform3DSet(256,256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[5].transform));
	L3_transform3DSet(256,-256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[6].transform));
	L3_transform3DSet(-256,256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[7].transform));
	L3_transform3DSet(-256,-256,512,0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_MODELS[8].transform));

	L3_transform3DSet(origbillboard[0][0],0,origbillboard[0][1],0,0,0,L3_F*4*2,L3_F*2,L3_F*2,&(L3_BILLBOARDS[0].transform));
	L3_transform3DSet(origbillboard[1][0],0,origbillboard[1][1],0,0,0,L3_F*2,L3_F*2*2,L3_F*2,&(L3_BILLBOARDS[1].transform));
	L3_transform3DSet(origbillboard[2][0],0,origbillboard[2][1],0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_BILLBOARDS[2].transform));
	L3_transform3DSet(origbillboard[3][0],0,origbillboard[3][1],0,0,0,L3_F*2,L3_F*2,L3_F*2,&(L3_BILLBOARDS[3].transform));

	L3_SCENE.camera.transform.translation.y = 0 * L3_F;
	L3_SCENE.camera.transform.translation.z = -0 * L3_F;
	//L3_SCENE.camera.focalLength = 0;

	k_thread_create(&render_thread, render_thread_stack, 4096,
                render_function, NULL, NULL, NULL,
                6, 0, K_NO_WAIT);

	k_thread_create(&process_thread, process_thread_stack, 4096,
                process_function, NULL, NULL, NULL,
                5, 0, K_NO_WAIT);

	return 0;
}
