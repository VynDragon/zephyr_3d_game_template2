#pragma once
static const L3_Unit Road_1_Side_vertices[] = {
1024,-128,2048,
1024,0,2048,
1024,0,2816,
-1024,0,2048,
-1024,0,2816,
-1024,-128,2048,
};
static const L3_Index Road_1_Side_indexes[] = {
3, 0, 5,
4, 1, 3,
3, 1, 0,
4, 2, 1,
};
static const L3_COLORTYPE *Road_1_Side_textures[] = {
tex_Road_1,
};
static const L3_Unit Road_1_Side_textures_width[] = {
192,
};
static const L3_Unit Road_1_Side_textures_height[] = {
192,
};
static const L3_Unit Road_1_Side_UVs[] = {
70, 12,
61, 0,
61, 12,
83, 12,
70, 0,
70, 12,
70, 12,
70, 0,
61, 0,
83, 12,
83, 0,
70, 0,
};
static const L3_Index Road_1_Side_indexes_texture[] = {
0,0,0,0,};
static const L3_Model3D Road_1_Side = {
.vertices = Road_1_Side_vertices,
.triangleCount = 4,
.vertexCount = 6,
.triangles = Road_1_Side_indexes,
.triangleTextures = Road_1_Side_textures,
.triangleUVs = Road_1_Side_UVs,
.triangleTextureIndex = Road_1_Side_indexes_texture,
.triangleTextureWidth = Road_1_Side_textures_width,
.triangleTextureHeight = Road_1_Side_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Road_1_Side_object_object = {
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
.model = &Road_1_Side,
};
static const Engine_Object Road_1_Side_object = {
.visual = Road_1_Side_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
