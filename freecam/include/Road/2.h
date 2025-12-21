#pragma once
static const L3_Unit Road_2_vertices[] = {
4096,-128,0,
-4096,-128,-2048,
4096,-128,-2048,
-4096,0,0,
4096,0,0,
-4096,0,-2048,
4096,0,-2048,
-4096,0,768,
4096,0,768,
4096,0,-2816,
-4096,0,-2816,
-4096,-128,0,
};
static const L3_Index Road_2_indexes[] = {
0, 1, 11,
3, 0, 11,
7, 4, 3,
6, 1, 2,
9, 5, 6,
0, 2, 1,
3, 4, 0,
7, 8, 4,
6, 5, 1,
9, 10, 5,
};
static const L3_COLORTYPE *Road_2_textures[] = {
tex_Road_1,
};
static const L3_Unit Road_2_textures_width[] = {
192,
};
static const L3_Unit Road_2_textures_height[] = {
192,
};
static const L3_Unit Road_2_UVs[] = {
46, 192,
92, 96,
46, 96,
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
46, 192,
92, 192,
92, 96,
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
static const L3_Index Road_2_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Road_2 = {
.vertices = Road_2_vertices,
.triangleCount = 10,
.vertexCount = 12,
.triangles = Road_2_indexes,
.triangleTextures = Road_2_textures,
.triangleUVs = Road_2_UVs,
.triangleTextureIndex = Road_2_indexes_texture,
.triangleTextureWidth = Road_2_textures_width,
.triangleTextureHeight = Road_2_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Road_2_object_object = {
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
.model = &Road_2,
};
static const Engine_Object Road_2_object = {
.visual = Road_2_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
