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

void filter_pixelize_2(Filterable_Pixel *p, void *data)
{
	if (p->x % 2 == 0 && p->y % 2 == 0 && p->y < p->size_y - 2 && p->x < p->size_x - 2) {
		L3_COLORTYPE c = (p->vpx[0] + p->vpx[1] + p->vpx[p->size_x] + p->vpx[p->size_x + 1]) / 4;
		p->vpx[0] = c;
		p->vpx[1] = c;
		p->vpx[p->size_x] = c;
		p->vpx[p->size_x + 1] = c;
	}
}

void filter_pixelize_4(Filterable_Pixel *p, void *data)
{
	if (p->x % 4 == 0 && p->y % 4 == 0 && p->y < p->size_y - 4 && p->x < p->size_x - 4) {
		L3_COLORTYPE c = (p->vpx[0] + p->vpx[1] + p->vpx[2] + p->vpx[3]
			+ p->vpx[p->size_x] + p->vpx[p->size_x + 1] + p->vpx[p->size_x + 2] + p->vpx[p->size_x + 3]
			+ p->vpx[p->size_x*2] + p->vpx[p->size_x*2 + 1] + p->vpx[p->size_x*2 + 2] + p->vpx[p->size_x*2 + 3]
			+ p->vpx[p->size_x*3] + p->vpx[p->size_x*3 + 1] + p->vpx[p->size_x*3 + 2] + p->vpx[p->size_x*3 + 3]
		) / 16;
		p->vpx[0] = c;
		p->vpx[1] = c;
		p->vpx[2] = c;
		p->vpx[3] = c;
		p->vpx[p->size_x] = c;
		p->vpx[p->size_x + 1] = c;
		p->vpx[p->size_x + 2] = c;
		p->vpx[p->size_x + 3] = c;
		p->vpx[p->size_x*2] = c;
		p->vpx[p->size_x*2 + 1] = c;
		p->vpx[p->size_x*2 + 2] = c;
		p->vpx[p->size_x*2 + 3] = c;
		p->vpx[p->size_x*3] = c;
		p->vpx[p->size_x*3 + 1] = c;
		p->vpx[p->size_x*3 + 2] = c;
		p->vpx[p->size_x*3 + 3] = c;
	}
}

void filter_pixelize_6(Filterable_Pixel *p, void *data)
{
	if (p->x % 6 == 0 && p->y % 6 == 0 && p->y < p->size_y - 6 && p->x < p->size_x - 6) {
		L3_COLORTYPE c = (p->vpx[0] + p->vpx[1] + p->vpx[2] + p->vpx[3] + p->vpx[4] + p->vpx[5]
			+ p->vpx[p->size_x] + p->vpx[p->size_x + 1] + p->vpx[p->size_x + 2] + p->vpx[p->size_x + 3] + p->vpx[p->size_x + 4] + p->vpx[p->size_x + 5]
			+ p->vpx[p->size_x*2] + p->vpx[p->size_x*2 + 1] + p->vpx[p->size_x*2 + 2] + p->vpx[p->size_x*2 + 3] + p->vpx[p->size_x*2 + 4] + p->vpx[p->size_x*2 + 5]
			+ p->vpx[p->size_x*3] + p->vpx[p->size_x*3 + 1] + p->vpx[p->size_x*3 + 2] + p->vpx[p->size_x*3 + 3] + p->vpx[p->size_x*3 + 4] + p->vpx[p->size_x*3 + 5]
			+ p->vpx[p->size_x*4] + p->vpx[p->size_x*4 + 1] + p->vpx[p->size_x*4 + 2] + p->vpx[p->size_x*4 + 3] + p->vpx[p->size_x*4 + 4] + p->vpx[p->size_x*4 + 5]
			+ p->vpx[p->size_x*5] + p->vpx[p->size_x*5 + 1] + p->vpx[p->size_x*5 + 2] + p->vpx[p->size_x*5 + 3] + p->vpx[p->size_x*5 + 4] + p->vpx[p->size_x*5 + 5]
		) / 36;
		memset(p->vpx, c, 6);
		memset(&p->vpx[p->size_x], c, 6);
		memset(&p->vpx[p->size_x*2], c, 6);
		memset(&p->vpx[p->size_x*3], c, 6);
		memset(&p->vpx[p->size_x*4], c, 6);
		memset(&p->vpx[p->size_x*5], c, 6);
	}
}

void filter_pixelize_8(Filterable_Pixel *p, void *data)
{
	if (p->x % 8 == 0 && p->y % 8 == 0 && p->y < p->size_y - 8 && p->x < p->size_x - 8) {
		L3_COLORTYPE c = (p->vpx[0] + p->vpx[1] + p->vpx[2] + p->vpx[3] + p->vpx[4] + p->vpx[5] + p->vpx[6] + p->vpx[7]
			+ p->vpx[p->size_x] + p->vpx[p->size_x + 1] + p->vpx[p->size_x + 2] + p->vpx[p->size_x + 3] + p->vpx[p->size_x + 4] + p->vpx[p->size_x + 5] + p->vpx[p->size_x + 6] + p->vpx[p->size_x + 7]
			+ p->vpx[p->size_x*2] + p->vpx[p->size_x*2 + 1] + p->vpx[p->size_x*2 + 2] + p->vpx[p->size_x*2 + 3] + p->vpx[p->size_x*2 + 4] + p->vpx[p->size_x*2 + 5] + p->vpx[p->size_x*2 + 6] + p->vpx[p->size_x*2 + 7]
			+ p->vpx[p->size_x*3] + p->vpx[p->size_x*3 + 1] + p->vpx[p->size_x*3 + 2] + p->vpx[p->size_x*3 + 3] + p->vpx[p->size_x*3 + 4] + p->vpx[p->size_x*3 + 5] + p->vpx[p->size_x*3 + 6] + p->vpx[p->size_x*3 + 7]
			+ p->vpx[p->size_x*4] + p->vpx[p->size_x*4 + 1] + p->vpx[p->size_x*4 + 2] + p->vpx[p->size_x*4 + 3] + p->vpx[p->size_x*4 + 4] + p->vpx[p->size_x*4 + 5] + p->vpx[p->size_x*4 + 6] + p->vpx[p->size_x*4 + 7]
			+ p->vpx[p->size_x*5] + p->vpx[p->size_x*5 + 1] + p->vpx[p->size_x*5 + 2] + p->vpx[p->size_x*5 + 3] + p->vpx[p->size_x*5 + 4] + p->vpx[p->size_x*5 + 5] + p->vpx[p->size_x*5 + 6] + p->vpx[p->size_x*5 + 7]
			+ p->vpx[p->size_x*6] + p->vpx[p->size_x*6 + 1] + p->vpx[p->size_x*6 + 2] + p->vpx[p->size_x*6 + 3] + p->vpx[p->size_x*6 + 4] + p->vpx[p->size_x*6 + 5] + p->vpx[p->size_x*6 + 6] + p->vpx[p->size_x*6 + 7]
			+ p->vpx[p->size_x*7] + p->vpx[p->size_x*7 + 1] + p->vpx[p->size_x*7 + 2] + p->vpx[p->size_x*7 + 3] + p->vpx[p->size_x*7 + 4] + p->vpx[p->size_x*7 + 5] + p->vpx[p->size_x*7 + 6] + p->vpx[p->size_x*7 + 7]
		) / 64;
		memset(p->vpx, c, 8);
		memset(&p->vpx[p->size_x], c, 8);
		memset(&p->vpx[p->size_x*2], c, 8);
		memset(&p->vpx[p->size_x*3], c, 8);
		memset(&p->vpx[p->size_x*4], c, 8);
		memset(&p->vpx[p->size_x*5], c, 8);
		memset(&p->vpx[p->size_x*6], c, 8);
		memset(&p->vpx[p->size_x*7], c, 8);
	}
}

void filter_bloom(Filterable_Pixel *p, void *data)
{
	if (p->y > 2 && p->y < p->size_y - 2 && p->x > 2 && p->x < p->size_x - 2 && p->vpx[0] < L3_COLORTYPE_LIGHT_THRES) {
		L3_COLORTYPE c = (p->vpx[1] + p->vpx[-1] + p->vpx[p->size_x] + p->vpx[-p->size_x]
			+ p->vpx[2] + p->vpx[-2] + p->vpx[p->size_x*2] + p->vpx[-p->size_x*2]
			 + p->vpx[p->size_x - 1] + p->vpx[-p->size_x + 1] + p->vpx[p->size_x*2 + 1] + p->vpx[-p->size_x*2 -1]
			 + p->vpx[p->size_x - 2] + p->vpx[-p->size_x + 2] + p->vpx[p->size_x*2 + 2] + p->vpx[-p->size_x*2 -2]
		) / 16;
		if (c > L3_COLORTYPE_LIGHT_THRES / 2) {
			p->vpx[0] = min((p->vpx[0] + (c * 10) / 8) / 2, 0xff);
		}
	}
}

void filter_star_bloom_6(Filterable_Pixel *p, void *data)
{
	if (p->y > 6 && p->y < p->size_y - 6 && p->x > 6 && p->x < p->size_x - 6 && p->vpx[0] < L3_COLORTYPE_LIGHT_THRES) {
		L3_COLORTYPE c = 0;
		if (p->vpx[3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[4] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-4] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*4] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*4] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[5] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-5] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*5] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*5] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[6] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-6] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*6] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*6] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		p->vpx[0] = min(p->vpx[0] + c, 0xff);
	}
}

void filter_star_bloom_3(Filterable_Pixel *p, void *data)
{
	if (p->y > 3 && p->y < p->size_y - 3 && p->x > 3 && p->x < p->size_x - 3) {
		L3_COLORTYPE c = 0;
		if (p->vpx[1] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-1] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*1] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*1] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[2] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-2] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*2] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*2] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[-p->size_x*3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[p->size_x*3] > L3_COLORTYPE_LIGHT_THRES) {
			c += 0x10;
		}
		if (p->vpx[0] < L3_COLORTYPE_LIGHT_THRES) {
			p->vpx[0] = min(p->vpx[0] + c, 0xff);
		} else {
			p->vpx[0] = min(p->vpx[0] + c / 6, 0xff);
		}
	}
}
