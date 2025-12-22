#include <zephyr/kernel.h>
#include "engine.h"

void filter_apply(uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y, Filter_f filter, void *data)
{
	Filterable_Pixel pix = {
		.vbuf = &(L3_video_buffer[x + y * L3_RESOLUTION_X]),
		#if L3_Z_BUFFER
			.zbuf = &(L3_zBuffer[x + y * L3_RESOLUTION_X]),
		#else
			.zbuf = NULL;
			.zpx = NULL;
		#endif
		.size_x = size_x,
		.size_y = size_y,
	};

	for (size_t x_ = 0; x_ < size_x; x_++) {
		for (size_t y_ = 0; y_ < size_y; y_++) {
			pix.x = x_;
			pix.y = y_;
			pix.vpx = &(pix.vbuf[x_ + y_ * size_x]);
			#if L3_Z_BUFFER
				pix.zpx = &(pix.zbuf[x_ + y_ * size_x]);
			#endif
			filter(&pix, data);
		}
	}
}

void filter_apply_all(uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y, const Filter_f *filters, size_t len, void *data)
{
	Filterable_Pixel pix = {
		.vbuf = &(L3_video_buffer[x + y * L3_RESOLUTION_X]),
		#if L3_Z_BUFFER
			.zbuf = &(L3_zBuffer[x + y * L3_RESOLUTION_X]),
		#else
			.zbuf = NULL;
			.zpx = NULL;
		#endif
		.size_x = size_x,
		.size_y = size_y,
	};

	if (len <= 0) {
		return;
	}

	for (size_t x_ = 0; x_ < size_x; x_++) {
		for (size_t y_ = 0; y_ < size_y; y_++) {
			pix.x = x_;
			pix.y = y_;
			pix.vpx = &(pix.vbuf[x_ + y_ * size_x]);
			#if L3_Z_BUFFER
				pix.zpx = &(pix.zbuf[x_ + y_ * size_x]);
			#endif
			for (size_t i = 0; i < len; i++) {
				filters[i](&pix, data);
			}
		}
	}

}

void filter_blur(Filterable_Pixel *p, void *data)
{
	if (p->y > 1 && p->y < p->size_y - 1 && p->x > 1 && p->x < p->size_x - 1) {
		*p->vpx = (*p->vpx + p->vpx[1] + p->vpx[-1] + p->vpx[p->size_x] + p->vpx[-p->size_x]) / 5;
	}
}

void filter_fixgap(Filterable_Pixel *p, void *data)
{
	if (*p->vpx < 0xF) {
		if (p->y > 1 && p->y < p->size_y - 2 && p->x > 1 && p->x < p->size_x - 2) {
			if (p->vpx[1] > 0xF && p->vpx[-1] > 0xF) {
				*p->vpx = (p->vpx[1] + p->vpx[-1]) / 2;
			} else if (p->vpx[p->size_x] > 0xF && p->vpx[-p->size_x] > 0xF) {
				*p->vpx = (p->vpx[p->size_x] + p->vpx[-p->size_x]) / 2;
			}
		}
	}
}
