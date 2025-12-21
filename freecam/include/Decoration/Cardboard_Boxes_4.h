#pragma once
static const L3_Unit Decoration_Cardboard_Boxes_4_vertices[] = {
-182,0,931,
-182,288,931,
-182,0,-256,
-182,288,-256,
182,0,931,
182,288,931,
182,0,-256,
182,288,-256,
};
static const L3_Index Decoration_Cardboard_Boxes_4_indexes[] = {
1, 2, 0,
3, 6, 2,
7, 4, 6,
5, 0, 4,
6, 0, 2,
3, 5, 7,
1, 3, 2,
3, 7, 6,
7, 5, 4,
5, 1, 0,
6, 4, 0,
3, 1, 5,
};
static const L3_COLORTYPE *Decoration_Cardboard_Boxes_4_textures[] = {
tex_Cardboard_Box,
};
static const L3_Unit Decoration_Cardboard_Boxes_4_textures_width[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_4_textures_height[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_4_UVs[] = {
74, 26,
51, 3,
51, 26,
43, 24,
20, 2,
21, 24,
2, 1,
15, 14,
15, 1,
66, 27,
30, 64,
67, 64,
19, 4,
40, 26,
40, 4,
37, 20,
58, 63,
58, 19,
74, 26,
74, 3,
51, 3,
43, 24,
43, 2,
20, 2,
2, 1,
2, 14,
15, 14,
66, 27,
30, 28,
30, 64,
19, 4,
19, 26,
40, 26,
37, 20,
37, 63,
58, 63,
};
static const L3_Index Decoration_Cardboard_Boxes_4_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Decoration_Cardboard_Boxes_4 = {
.vertices = Decoration_Cardboard_Boxes_4_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = Decoration_Cardboard_Boxes_4_indexes,
.triangleTextures = Decoration_Cardboard_Boxes_4_textures,
.triangleUVs = Decoration_Cardboard_Boxes_4_UVs,
.triangleTextureIndex = Decoration_Cardboard_Boxes_4_indexes_texture,
.triangleTextureWidth = Decoration_Cardboard_Boxes_4_textures_width,
.triangleTextureHeight = Decoration_Cardboard_Boxes_4_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Decoration_Cardboard_Boxes_4_object_object = {
.transform.scale.x = L3_F,
.transform.scale.y = L3_F,
.transform.scale.z = L3_F,
.transform.scale.w = 0,
.transform.translation.x = 0,
.transform.translation.y = 0,
.transform.translation.z = 0,
.transform.translation.w = L3_F,
.transform.rotation.x = 0,
.transform.rotation.y = 0,
.transform.rotation.z = 0,
.transform.rotation.w = L3_F,
.config.backfaceCulling = 1,
.config.visible = 1,
.solid_color = 0xFF,
.model = &Decoration_Cardboard_Boxes_4,
};
static const Engine_Object Decoration_Cardboard_Boxes_4_object = {
.visual = Decoration_Cardboard_Boxes_4_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
