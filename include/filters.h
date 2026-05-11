#pragma once

void filter_blur(Filterable_Pixel *p, void *data);
void filter_fixgap(Filterable_Pixel *p, void *data);
void filter_pixelize_2(Filterable_Pixel *p, void *data);
void filter_pixelize_4(Filterable_Pixel *p, void *data);
void filter_pixelize_6(Filterable_Pixel *p, void *data);
void filter_pixelize_8(Filterable_Pixel *p, void *data);
void filter_bloom(Filterable_Pixel *p, void *data);
void filter_star_bloom_6(Filterable_Pixel *p, void *data);
void filter_star_bloom_3(Filterable_Pixel *p, void *data);
