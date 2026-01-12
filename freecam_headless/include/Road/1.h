#pragma once
static const L3_Unit Road_1_vertices[] = {
-4096,-128,2048,
4096,-128,2048,
-4096,-128,-2048,
4096,-128,-2048,
-4096,0,2048,
4096,0,2048,
-4096,0,-2048,
4096,0,-2048,
-4096,0,2816,
4096,0,2816,
4096,0,-2816,
-4096,0,-2816,
};
static const L3_Index Road_1_indexes[] = {
1, 2, 0,
4, 1, 0,
8, 5, 4,
7, 2, 3,
10, 6, 7,
1, 3, 2,
4, 5, 1,
8, 9, 5,
7, 6, 2,
10, 11, 6,
};
static const L3_COLORTYPE *Road_1_textures[] = {
tex_Road_1,
};
static const L3_Unit Road_1_textures_width[] = {
192,
};
static const L3_Unit Road_1_textures_height[] = {
192,
};
static const L3_Unit Road_1_UVs[] = {
0, 191,
95, 96,
0, 96,
70, 47,
61, 0,
61, 47,
83, 47,
70, 0,
70, 47,
70, 47,
61, 0,
61, 47,
83, 47,
70, 0,
70, 47,
0, 191,
95, 191,
95, 96,
70, 47,
70, 0,
61, 0,
83, 47,
83, 0,
70, 0,
70, 47,
70, 0,
61, 0,
83, 47,
83, 0,
70, 0,
};
static const L3_Index Road_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Road_1 = {
.vertices = Road_1_vertices,
.triangleCount = 10,
.vertexCount = 12,
.triangles = Road_1_indexes,
.triangleTextures = Road_1_textures,
.triangleUVs = Road_1_UVs,
.triangleTextureIndex = Road_1_indexes_texture,
.triangleTextureWidth = Road_1_textures_width,
.triangleTextureHeight = Road_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Road_1_object_object = {
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
.model = &Road_1,
};
static const Engine_Object Road_1_object = {
.visual = Road_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
