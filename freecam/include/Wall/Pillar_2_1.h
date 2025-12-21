#pragma once
static const L3_Unit Wall_Pillar_2_1_vertices[] = {
-250,0,373,
-128,1536,256,
-250,0,0,
-128,1536,0,
250,0,373,
128,1536,256,
250,0,0,
128,1536,0,
-128,377,256,
-128,377,0,
128,377,0,
128,377,256,
};
static const L3_Index Wall_Pillar_2_1_indexes[] = {
1, 9, 8,
7, 11, 10,
5, 8, 11,
3, 5, 7,
4, 8, 0,
6, 11, 4,
8, 2, 0,
1, 3, 9,
7, 5, 11,
5, 1, 8,
3, 1, 5,
4, 11, 8,
6, 10, 11,
8, 9, 2,
};
static const L3_COLORTYPE *Wall_Pillar_2_1_textures[] = {
tex_Brick_Wall_2,
};
static const L3_Unit Wall_Pillar_2_1_textures_width[] = {
96,
};
static const L3_Unit Wall_Pillar_2_1_textures_height[] = {
96,
};
static const L3_Unit Wall_Pillar_2_1_UVs[] = {
16, -10,
0, 69,
16, 69,
48, -10,
31, 69,
48, 69,
31, -10,
16, 69,
31, 69,
76, 76,
92, 92,
92, 76,
31, 96,
16, 70,
16, 96,
48, 96,
31, 70,
31, 96,
16, 70,
0, 96,
16, 96,
16, -10,
0, -10,
0, 69,
48, -10,
31, -10,
31, 69,
31, -10,
16, -10,
16, 69,
76, 76,
76, 92,
92, 92,
31, 96,
31, 70,
16, 70,
48, 96,
48, 70,
31, 70,
16, 70,
0, 70,
0, 96,
};
static const L3_Index Wall_Pillar_2_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Wall_Pillar_2_1 = {
.vertices = Wall_Pillar_2_1_vertices,
.triangleCount = 14,
.vertexCount = 12,
.triangles = Wall_Pillar_2_1_indexes,
.triangleTextures = Wall_Pillar_2_1_textures,
.triangleUVs = Wall_Pillar_2_1_UVs,
.triangleTextureIndex = Wall_Pillar_2_1_indexes_texture,
.triangleTextureWidth = Wall_Pillar_2_1_textures_width,
.triangleTextureHeight = Wall_Pillar_2_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_Pillar_2_1_object_object = {
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
.model = &Wall_Pillar_2_1,
};
static const Engine_Object Wall_Pillar_2_1_object = {
.visual = Wall_Pillar_2_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
