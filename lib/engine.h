#pragma once

/* S3D section */
#define S3L_FLAT 0
#define S3L_NEAR_CROSS_STRATEGY 0
#define S3L_PERSPECTIVE_CORRECTION 0
#define S3L_SORT 0
#define S3L_STENCIL_BUFFER 0
#define S3L_Z_BUFFER 2
#define S3L_NEAR 1

#define S3L_PIXEL_FUNCTION		zephyr_putpixel
#define S3L_TRIANGLE_FUNCTION	zephyr_drawtriangle
#define S3L_BILLBOARD_FUNCTION	zephyr_drawbillboard

#define S3L_RESOLUTION_X 128
#define S3L_RESOLUTION_Y 96

#include "small3dlib.h"

/* Define video Buffer */
#if DT_HAS_CHOSEN(zephyr_dtcm)
static __attribute__((section("DTCM"))) uint8_t video_buffer[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];
#else
static uint8_t video_buffer[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];
#endif

#include "tools.h"

#define TEXTURE_WH 32

#define FPS_TIMEPOINT(fps) sys_timepoint_calc(K_MSEC(1000 / fps))

#define S3L_MAX_MODELS 0x7F
#define S3L_MAX_BILLBOARDS 0xFF

S3L_Model3D engine_global_models[S3L_MAX_MODELS];
S3L_Billboard engine_global_billboards[S3L_MAX_BILLBOARDS];
S3L_Scene engine_global_scene;

#define S3L_SCENE		engine_global_scene
#define S3L_MODELS		engine_global_models
#define S3L_BILLBOARDS	engine_global_billboards

#define S3L_VISIBLE_INVISIBLE		0
#define S3L_VISIBLE_TEXTURED		BIT(0)
#define S3L_VISIBLE_WIREFRAME		BIT(1)
#define S3L_VISIBLE_SOLID			BIT(2)
#define S3L_VISIBLE_DISTANCELIGHT	BIT(3)


static int zephyr_putpixel_current_render_mode = 0;
inline void zephyr_putpixel(S3L_PixelInfo *p)
{
	float depthmul = 1.0;
	S3L_Model3D *model = &(engine_global_scene.models[p->modelIndex]);

	if (zephyr_putpixel_current_render_mode & S3L_VISIBLE_DISTANCELIGHT) {
		depthmul = p->depth / 128;
		depthmul = 1 / (depthmul != 0 ? depthmul : 1);
		depthmul = depthmul > 1 ? 1 : depthmul;
	}

	if (zephyr_putpixel_current_render_mode & S3L_VISIBLE_TEXTURED) {
		if (model->model->triangleTextureIndex[p->triangleIndex] < 0) {
			return;
		}
		S3L_Unit uv[2];

		const S3L_Unit *uvs = &(model->model->triangleUVs[p->triangleIndex * 6]);

		uv[0] = abs(S3L_interpolateBarycentric(uvs[0], uvs[2], uvs[4], p->barycentric) % TEXTURE_WH);
		uv[1] = abs(S3L_interpolateBarycentric(uvs[1], uvs[3], uvs[5], p->barycentric) % TEXTURE_WH);
		if (likely(0 < p->x && S3L_RESOLUTION_X > p->x && 0 < p->y && S3L_RESOLUTION_Y > p->y))
			video_buffer[p->x + p->y * S3L_RESOLUTION_X] = model->model->triangleTextures[model->model->triangleTextureIndex[p->triangleIndex]][(uv[0] >> 0) + (uv[1] >> 0) * TEXTURE_WH] * depthmul;

	} else if (zephyr_putpixel_current_render_mode & S3L_VISIBLE_SOLID) {
		if (likely(0 < p->x && S3L_RESOLUTION_X > p->x && 0 < p->y && S3L_RESOLUTION_Y > p->y))
			video_buffer[p->x + p->y * S3L_RESOLUTION_X] = 255 * depthmul;
	}
}

inline int zephyr_drawtriangle(S3L_Vec4 point0, S3L_Vec4 point1, S3L_Vec4 point2,
								S3L_Index modelIndex, S3L_Index triangleIndex)
{
	zephyr_putpixel_current_render_mode = engine_global_scene.models[modelIndex].config.visible;
	if (zephyr_putpixel_current_render_mode & S3L_VISIBLE_WIREFRAME) {
		plot_line(255, point0.x, point0.y, point1.x, point1.y);
		plot_line(255, point2.x, point2.y, point1.x, point1.y);
		plot_line(255, point2.x, point2.y, point0.x, point0.y);
		return 0;
	}
	return 1;
}

inline int zephyr_drawbillboard(S3L_Vec4 point, S3L_Billboard *billboard)
{
	if (!S3L_zTest(point.x,point.y,point.z))
		return 0;

	int focal = S3L_SCENE.camera.focalLength != 0 ? S3L_SCENE.camera.focalLength : 1;
	float scale_x = ((float)(billboard->transform.scale.x * focal) / (float)point.z) * ((float)billboard->scale/16384);
	float scale_y = ((float)(billboard->transform.scale.y * focal) / (float)point.z) * ((float)billboard->scale/16384);
	int scaled_width = billboard->width * scale_x;
	int scaled_height = billboard->height * scale_y;

	/* this has been optimized */
	int startx = point.x - scaled_width / 2;
	int endx =  MIN(point.x + scaled_width / 2, S3L_RESOLUTION_X);
	int starty = point.y - scaled_height / 2;
	int endy =  MIN(point.y + scaled_height / 2, S3L_RESOLUTION_Y);
	S3L_Unit z = S3L_zBufferFormat(point.z);
	for (int i = MAX(0, startx); i < endx; i++) {
		int m = (float)(i - startx) / scale_x;
		for (int j = MAX(0, starty); j < endy; j++) {
			int n = (float)(j - starty) / scale_y;
			video_buffer[j * S3L_RESOLUTION_X + i] = billboard->texture[m + n * billboard->width];
			#if S3L_Z_BUFFER
			S3L_zBuffer[j * S3L_RESOLUTION_X + i] = z;
			#endif
		}
	}
	return 0;
}
