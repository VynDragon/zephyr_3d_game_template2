#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);


/* S3D section */
#define S3L_FLAT 0
#define S3L_NEAR_CROSS_STRATEGY 0
#define S3L_PERSPECTIVE_CORRECTION 0
#define S3L_SORT 0
#define S3L_STENCIL_BUFFER 0
#define S3L_Z_BUFFER 2
#define S3L_NEAR 1

#define S3L_PIXEL_FUNCTION zephyr_putpixel
#define S3L_TRIANGLE_FUNCTION  zephyr_drawtriangle

#define S3L_RESOLUTION_X 128
#define S3L_RESOLUTION_Y 96

#include "small3dlib.h"

#include "tools.h"

#include "building.h"

#include "plane.h"

#define TEXTURE_WH 32

// intended for 48k dtcm
#if DT_HAS_CHOSEN(zephyr_dtcm)
static __attribute__((section("DTCM"))) uint8_t video_buffer[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];
#else
static uint8_t video_buffer[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];
#endif

void clearScreen()
{
	uint32_t index = 0;

	for (; index < S3L_RESOLUTION_Y * S3L_RESOLUTION_X; index++)
	{
		video_buffer[index] = 0;
	}
}


S3L_Model3D models[1];

inline void zephyr_putpixel(S3L_PixelInfo *p)
{
	float depthmul = p->depth / 128;
	depthmul = 1 / (depthmul != 0 ? depthmul : 1);
	depthmul = depthmul > 1 ? 1 : depthmul;
	if (models[p->modelIndex].triangleTextureIndex != 0) {
		if (models[p->modelIndex].triangleTextureIndex[p->triangleIndex] < 0) {
			if (0 < p->x && S3L_RESOLUTION_X > p->x && 0 < p->y && S3L_RESOLUTION_Y > p->y)
				video_buffer[p->x + p->y * S3L_RESOLUTION_X] = 0;
			return;
		}
		S3L_Unit uv[2];

		const S3L_Unit *uvs = &(models[p->modelIndex].triangleUVs[p->triangleIndex * 6]);

		uv[0] = abs(S3L_interpolateBarycentric(uvs[0], uvs[2], uvs[4], p->barycentric) % TEXTURE_WH);
		uv[1] = abs(S3L_interpolateBarycentric(uvs[1], uvs[3], uvs[5], p->barycentric) % TEXTURE_WH);
		if (0 < p->x && S3L_RESOLUTION_X > p->x && 0 < p->y && S3L_RESOLUTION_Y > p->y)
			video_buffer[p->x + p->y * S3L_RESOLUTION_X] = models[p->modelIndex].triangleTextures[models[p->modelIndex].triangleTextureIndex[p->triangleIndex]][(uv[0] >> 0) + (uv[1] >> 0) * TEXTURE_WH] * depthmul;
	} else {
		if (0 < p->x && S3L_RESOLUTION_X > p->x && 0 < p->y && S3L_RESOLUTION_Y > p->y)
			video_buffer[p->x + p->y * S3L_RESOLUTION_X] = 255 * depthmul;
	}
}

inline int zephyr_drawtriangle(S3L_Vec4 point0, S3L_Vec4 point1, S3L_Vec4 point2,
								S3L_Index modelIndex, S3L_Index triangleIndex)
{
	/*plot_line(200, point0.x, point0.y, point1.x, point1.y);
	plot_line(200, point2.x, point2.y, point1.x, point1.y);
	plot_line(200, point2.x, point2.y, point0.x, point0.y);*/
	return 1;
}

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int main()
{
	struct display_buffer_descriptor buf_desc;
	int sinvar = 0;
	timing_t start_time, end_time, dstart_time, rend_time, rstart_time;
	uint32_t total_time_us, render_time_us, draw_time_us;
	int close = 800;
	int scroll = 0;
	int close_add = 1;
	S3L_Scene scene;

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

	buf_desc.buf_size = S3L_RESOLUTION_X * S3L_RESOLUTION_Y;
	buf_desc.width = S3L_RESOLUTION_X;
	buf_desc.height = S3L_RESOLUTION_Y;
	buf_desc.pitch = S3L_RESOLUTION_X;

	models[0] = building_01;

	S3L_sceneInit(models,1,&scene);

	S3L_transform3DSet(0,0,512,0,0,0,S3L_F*10,S3L_F*10,S3L_F*10,&(models[0].transform));

	scene.camera.transform.translation.y = 0 * S3L_F;
	scene.camera.transform.translation.z = -1 * S3L_F;
	//scene.camera.focalLength = 0;

	while (1) {
		models[0].transform.rotation.y = sinvar;
		start_time = timing_counter_get();
		/* clear viewport to black */
		S3L_newFrame();
		clearScreen();

		rstart_time = timing_counter_get();
		uint32_t drawnTriangles = S3L_drawScene(scene);

		rend_time = timing_counter_get();
		dstart_time = timing_counter_get();
		display_write(display_device, 0, 0, &buf_desc, video_buffer);
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		render_time_us = timing_cycles_to_ns(timing_cycles_get(&rstart_time, &rend_time)) / 1000;
		draw_time_us = timing_cycles_to_ns(timing_cycles_get(&dstart_time, &end_time)) / 1000;
		printf("total us: %u ms:%u fps:%u\n", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
		printf("display us:%u render us:%u render fps: %u\n", draw_time_us, render_time_us, 1000000 / (render_time_us != 0 ? render_time_us : 1));
		printf("rendered %u Polygons, %u polygons per second\n", drawnTriangles, drawnTriangles * 1000000 / (render_time_us != 0 ? render_time_us : 1));
		sinvar+=1;
		close+=close_add*5;
		if (close > 1000)
		{
			close_add = -1;
		}
		else if (close < -1000)
		{
			close_add = 1;
		}
		scroll -= 10;
		k_msleep(1);
		//return 0;
		//msleep(100);
	}

	return 0;
}
