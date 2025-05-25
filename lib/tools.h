static void plot_line (uint8_t color, int x0, int y0, int x1, int y1)
{
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */

	for (;;){  /* loop */
		S3L_PixelInfo p = {
			.x = x0,
			.y = y0,
			.depth = (256 - color) * 16,
		};
		zephyr_putpixel(&p);
#if S3L_Z_BUFFER
		if (0 < x0 && S3L_RESOLUTION_X > x0 && 0 < y0 && S3L_RESOLUTION_Y > y0)
			S3L_zBuffer[y0 * S3L_RESOLUTION_X + x0] = 0;
#endif
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

/*static void plot_line (uint8_t color, int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int err = dy-dx;
	int j = y0;


	for (int i = x0; i < x1; i++) {
		S3L_PixelInfo p = {
			.x = i,
			.y = j,
			.depth = (256 - color) * 16,
		};
		zephyr_putpixel(&p);
		if (0 < i && S3L_RESOLUTION_X > i && 0 < j && S3L_RESOLUTION_Y > j)
			S3L_zBuffer[j * S3L_RESOLUTION_X + i] = 0;
		if (err >= 0) {
			j += 1;
			err -= dx;
		}
		err += dy;
	}
}*/
