#pragma once
static const L3_Unit Road_1_Low_vertices[] = {
-4096,-128,2048,
4096,-128,2048,
4096,-128,-2048,
-4096,-128,-2048,
};
static const L3_Index Road_1_Low_indexes[] = {
1, 3, 0,
1, 2, 3,
};
static const L3_COLORTYPE *Road_1_Low_textures[] = {
tex_Road_1,
};
static const L3_Unit Road_1_Low_textures_width[] = {
192,
};
static const L3_Unit Road_1_Low_textures_height[] = {
192,
};
static const L3_Unit Road_1_Low_UVs[] = {
0, 191,
95, 96,
0, 96,
0, 191,
95, 191,
95, 96,
};
static const L3_Index Road_1_Low_indexes_texture[] = {
0,0,};
static const L3_Model3D Road_1_Low = {
.vertices = Road_1_Low_vertices,
.triangleCount = 2,
.vertexCount = 4,
.triangles = Road_1_Low_indexes,
.triangleTextures = Road_1_Low_textures,
.triangleUVs = Road_1_Low_UVs,
.triangleTextureIndex = Road_1_Low_indexes_texture,
.triangleTextureWidth = Road_1_Low_textures_width,
.triangleTextureHeight = Road_1_Low_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Road_1_Low_object_object = {
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
.model = &Road_1_Low,
};
static const Engine_Object Road_1_Low_object = {
.visual = Road_1_Low_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
