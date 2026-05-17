#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include "engine.h"

static const struct device *display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

int blit_display_L8(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	buf_desc.buf_size = size_x * size_y;
	buf_desc.width = size_x;
	buf_desc.height = size_y;
	buf_desc.pitch = size_x;

	display_write(display_device, x, y, &buf_desc, buffer);

	return 0;
}

int blit_display_L8_2to1_1(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X * 2] = {0};
	buf_desc.buf_size = size_x * 2;
	buf_desc.width = size_x * 2;
	buf_desc.height = 1;
	buf_desc.pitch = size_x * 2;

	for (int j = 0; j < size_y; j++) {
		for (int i = 0; i < size_x; i++) {
			buf[i * 2] = buffer[i + j * size_x];
			buf[i * 2 + 1] = buffer[i + j * size_x];
		}
		display_write(display_device, x/2, y+j, &buf_desc, buf);
	}

	return 0;
}

int blit_display_MONO_vtiled(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X];

	if (size_y < 8 || (y & 0x7) != 0 || (size_y & 0x7) != 0) {
		return -EINVAL;
	}

	buf_desc.buf_size = size_x;
	buf_desc.width = size_x;
	buf_desc.height = 8;
	buf_desc.pitch = size_x;

	for (int j = 0; j < size_y; j+= 8) {
		for (int i = 0; i < size_x; i++) {
			buf[i] = buffer[i + (j + 0) * size_x] > 128 ? buf[i] | 1<<0 : buf[i] & ~(1<<0);
			buf[i] = buffer[i + (j + 1) * size_x] > 128 ? buf[i] | 1<<1 : buf[i] & ~(1<<1);
			buf[i] = buffer[i + (j + 2) * size_x] > 128 ? buf[i] | 1<<2 : buf[i] & ~(1<<2);
			buf[i] = buffer[i + (j + 3) * size_x] > 128 ? buf[i] | 1<<3 : buf[i] & ~(1<<3);
			buf[i] = buffer[i + (j + 4) * size_x] > 128 ? buf[i] | 1<<4 : buf[i] & ~(1<<4);
			buf[i] = buffer[i + (j + 5) * size_x] > 128 ? buf[i] | 1<<5 : buf[i] & ~(1<<5);
			buf[i] = buffer[i + (j + 6) * size_x] > 128 ? buf[i] | 1<<6 : buf[i] & ~(1<<6);
			buf[i] = buffer[i + (j + 7) * size_x] > 128 ? buf[i] | 1<<7 : buf[i] & ~(1<<7);
		}
		display_write(display_device, x, y+j, &buf_desc, buf);
	}

	return 0;
}

int blit_display_MONO_vtiled_dither(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X];
	int error = 0;

	if (size_y < 8 || (y & 0x7) != 0 || (size_y & 0x7) != 0) {
		return -EINVAL;
	}

	buf_desc.buf_size = size_x;
	buf_desc.width = size_x;
	buf_desc.height = 8;
	buf_desc.pitch = size_x;

#define err_buf(_xoff, _yoff) \
	error += buffer[i + (j + _yoff) * size_x + _xoff];		\
	if (error > 0xff) {											\
		buf[i + (_yoff/8) * size_x + _xoff] |= 1 << _yoff;		\
		error -= 0xff;										\
	} else {													\
		buf[i + (_yoff/8) * size_x + _xoff] &= ~(1 << _yoff);		\
	}

	for (int j = 0; j < size_y; j+= 8) {
		for (int i = 0; i < size_x; i++) {
			err_buf(0,0)
			err_buf(0,1)
			err_buf(0,2)
			err_buf(0,3)
			err_buf(0,4)
			err_buf(0,5)
			err_buf(0,6)
			err_buf(0,7)
		}
		display_write(display_device, x, y+j, &buf_desc, buf);
	}

#undef err_buf

	return 0;
}

int blit_display_MONO_vtiled_dither_2(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X*2];
	int error = 0;

	if (size_y*2 < 8 || (y*2 & 0x7) != 0 || (size_y*2 & 0x7) != 0) {
		return -EINVAL;
	}

	buf_desc.buf_size = size_x * 2;
	buf_desc.width = size_x*2;
	buf_desc.height = 8;
	buf_desc.pitch = size_x*2;

#define err_buf(_xoff, _yoff) \
	error += buffer[i + (j + _yoff)/2 * size_x + (_xoff/2)];		\
	if (error > 0xff) {											\
		buf[i * 2 + (_yoff/8) * size_x*2 + (_xoff/2)] |= 1 << (_yoff);		\
		error -= 0xff;										\
	} else {													\
		buf[i * 2 + (_yoff/8) * size_x*2 + (_xoff/2)] &= ~(1 << (_yoff));		\
	}

#define err_buf_2(_xoff)		\
	err_buf(_xoff,0)	\
	err_buf(_xoff,1)	\
	err_buf(_xoff,2)	\
	err_buf(_xoff,3)	\
	err_buf(_xoff,4)	\
	err_buf(_xoff,5)	\
	err_buf(_xoff,6)	\
	err_buf(_xoff,7)	\
	err_buf(_xoff,8)

	for (int j = 0; j < size_y * 2; j+= 8) {
		for (int i = 0; i < size_x; i++) {
			err_buf_2(0)
			err_buf_2(1)
		}
		display_write(display_device, x*2, y*2+j, &buf_desc, buf);
	}

#undef err_buf
#undef err_buf_2

	return 0;
}

int blit_display_MONO_dither_2(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X];
	int error = 0;

	buf_desc.buf_size = size_x*2;
	buf_desc.width = size_x*2;
	/* for compat with st730x, do 2 lines at once */
	buf_desc.height = 4;
	buf_desc.pitch = size_x*2;

#define err_buf(_xoff, _yoff) \
	error += buffer[i + (j + _yoff / 2) * size_x + (_xoff/2)] * 1;		\
	if (error > 0xff) {											\
		buf[i / 4 + _yoff * (size_x / 4)] |= 1 << _xoff;		\
		error -= 0xff;										\
	} else {													\
		buf[i / 4 + _yoff * (size_x / 4)] &= ~(1 << _xoff);		\
	}

	for (int j = 0; j < size_y; j+= 2) {
		for (int i = 0; i < size_x; i++) {
			err_buf(0,0)
			err_buf(0,1)
			err_buf(0,2)
			err_buf(0,3)
			err_buf(1,0)
			err_buf(1,1)
			err_buf(1,2)
			err_buf(1,3)
			err_buf(2,0)
			err_buf(2,1)
			err_buf(2,2)
			err_buf(2,3)
			err_buf(3,0)
			err_buf(3,1)
			err_buf(3,2)
			err_buf(3,3)
			err_buf(4,0)
			err_buf(4,1)
			err_buf(4,2)
			err_buf(4,3)
			err_buf(5,0)
			err_buf(5,1)
			err_buf(5,2)
			err_buf(5,3)
			err_buf(6,0)
			err_buf(6,1)
			err_buf(6,2)
			err_buf(6,3)
			err_buf(7,0)
			err_buf(7,1)
			err_buf(7,2)
			err_buf(7,3)
			err_buf(8,0)
			err_buf(8,1)
			err_buf(8,2)
			err_buf(8,3)
			err_buf(9,0)
			err_buf(9,1)
			err_buf(9,2)
			err_buf(9,3)
			err_buf(10,0)
			err_buf(10,1)
			err_buf(10,2)
			err_buf(10,3)
			err_buf(11,0)
			err_buf(11,1)
			err_buf(11,2)
			err_buf(11,3)
			err_buf(12,0)
			err_buf(12,1)
			err_buf(12,2)
			err_buf(12,3)
			err_buf(13,0)
			err_buf(13,1)
			err_buf(13,2)
			err_buf(13,3)
			err_buf(14,0)
			err_buf(14,1)
			err_buf(14,2)
			err_buf(14,3)
			err_buf(15,0)
			err_buf(15,1)
			err_buf(15,2)
			err_buf(15,3)
		}
		display_write(display_device, x*2, y*2 + j * 2, &buf_desc, buf);
	}
	return 0;
}

#undef err_buf

int blit_display_MONO_dither_4(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint8_t buf[L3_RESOLUTION_X*4];
	int error = 0;

	buf_desc.buf_size = size_x*4;
	buf_desc.width = size_x*4;
	/* for compat with st730x, do 2 lines at once */
	buf_desc.height = 8;
	buf_desc.pitch = size_x*4;

#define err_buf(_xoff, _yoff) \
	error += (buffer[i + (j + _yoff / 4) * size_x + (_xoff/4)]			\
			+ buffer[i + (j + (_yoff) / 4) * size_x + ((_xoff+1)/4)]	\
			+ buffer[i + (j + (_yoff) / 4) * size_x + ((_xoff-1)/4)]	\
			+ buffer[i + (j + (_yoff-1) / 4) * size_x + ((_xoff+1)/4)]	\
			+ buffer[i + (j + (_yoff+1) / 4) * size_x + ((_xoff-1)/4)]	\
			  ) / 5;	\
	if (error > 0xff) {											\
		buf[i / 2 + _yoff * (size_x / 2)] |= 1 << _xoff;		\
		error -= 0xff;										\
	} else {													\
		buf[i / 2 + _yoff * (size_x / 2)] &= ~(1 << _xoff);		\
	}

#define err_buf_2(_xoff)		\
			err_buf(_xoff,0)	\
			err_buf(_xoff,1)	\
			err_buf(_xoff,2)	\
			err_buf(_xoff,3)	\
			err_buf(_xoff,4)	\
			err_buf(_xoff,5)	\
			err_buf(_xoff,6)	\
			err_buf(_xoff,7)

#define err_buf_4(_xoff)		\
			err_buf_2((_xoff))	\
			err_buf_2((_xoff+1))	\
			err_buf_2((_xoff+2))	\
			err_buf_2((_xoff+3))	\
			err_buf_2((_xoff+4))	\
			err_buf_2((_xoff+5))	\
			err_buf_2((_xoff+6))	\
			err_buf_2((_xoff+7))

	for (int j = 0; j < size_y; j+= 2) {
		for (int i = 0; i < size_x; i++) {
			err_buf_4(0)
			err_buf_4(8)
			err_buf_4(16)
			err_buf_4(24)
		}
		display_write(display_device, x*4, y*4 + j * 4, &buf_desc, buf);
	}
	return 0;
}

#undef err_buf
#undef err_buf_2
#undef err_buf_4

int blit_display_RGB565(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_buffer_descriptor buf_desc;
	uint16_t buf[L3_RESOLUTION_X];
	buf_desc.buf_size = size_x * 2;
	buf_desc.width = size_x;
	buf_desc.height = 1;
	buf_desc.pitch = size_x;

	for (int j = y; j < size_y; j+= 1) {
		for (int i = 0; i < size_x; i++) {
			buf[i] = (uint16_t)(buffer[i + j * size_x] & 0xF8);
		}
		display_write(display_device, x, y+j, &buf_desc, buf);
	}
	return 0;
}

int blit_select_for_me(L3_COLORTYPE *buffer, uint16_t x, uint16_t y, uint16_t size_x, uint16_t size_y)
{
	struct display_capabilities caps = {0};

	display_get_capabilities(display_device, &caps);

	if (caps.current_pixel_format == PIXEL_FORMAT_L_8) {
		return blit_display_L8(buffer, x, y, size_x, size_y);
	}
	if (caps.current_pixel_format == PIXEL_FORMAT_RGB_565 || caps.current_pixel_format == PIXEL_FORMAT_RGB_565X) {
		return blit_display_RGB565(buffer, x, y, size_x, size_y);
	}
	if (caps.current_pixel_format == PIXEL_FORMAT_MONO10 || caps.current_pixel_format == PIXEL_FORMAT_MONO01) {
		if (caps.screen_info & SCREEN_INFO_MONO_VTILED) {
			if (caps.x_resolution >= L3_RESOLUTION_X * 2 && caps.y_resolution >= L3_RESOLUTION_Y * 2) {
				return blit_display_MONO_vtiled_dither_2(buffer, x, y, size_x, size_y);
			} else {
				return blit_display_MONO_vtiled_dither(buffer, x, y, size_x, size_y);
			}
			return -ENOTSUP;
		}
		if (caps.x_resolution >= L3_RESOLUTION_X * 4 && caps.y_resolution >= L3_RESOLUTION_Y * 4) {
			return blit_display_MONO_dither_4(buffer, x, y, size_x, size_y);
		} else if (caps.x_resolution >= L3_RESOLUTION_X * 2 && caps.y_resolution >= L3_RESOLUTION_Y * 2) {
			return blit_display_MONO_dither_2(buffer, x, y, size_x, size_y);
		}
		return -ENOTSUP;
	}
	return -ENOTSUP;
}
