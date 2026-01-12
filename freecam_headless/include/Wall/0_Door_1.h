#pragma once
static const L3_Unit Wall_0_Door_1_vertices[] = {
-512,0,0,
512,0,0,
-512,1536,0,
512,1536,0,
-231,1102,0,
231,1102,0,
-243,0,0,
243,0,0,
-231,1102,-131,
231,1102,-131,
-243,0,-131,
243,0,-131,
};
static const L3_Index Wall_0_Door_1_indexes[] = {
5, 3, 2,
8, 5, 4,
9, 7, 5,
10, 4, 6,
0, 6, 4,
5, 7, 1,
2, 0, 4,
5, 1, 3,
2, 4, 5,
8, 9, 5,
9, 11, 7,
10, 8, 4,
};
static const L3_COLORTYPE *Wall_0_Door_1_textures[] = {
tex_Brick_Wall_2,
};
static const L3_Unit Wall_0_Door_1_textures_width[] = {
96,
};
static const L3_Unit Wall_0_Door_1_textures_height[] = {
96,
};
static const L3_Unit Wall_0_Door_1_UVs[] = {
19, 19,
0, -10,
72, -10,
67, 16,
26, 4,
67, 4,
58, 18,
47, 96,
47, 18,
9, 96,
20, 19,
20, 96,
72, 96,
53, 96,
52, 19,
19, 19,
18, 96,
0, 96,
72, -10,
72, 96,
52, 19,
19, 19,
0, 96,
0, -10,
72, -10,
52, 19,
19, 19,
67, 16,
26, 16,
26, 4,
58, 18,
58, 96,
47, 96,
9, 96,
9, 19,
20, 19,
};
static const L3_Index Wall_0_Door_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Wall_0_Door_1 = {
.vertices = Wall_0_Door_1_vertices,
.triangleCount = 12,
.vertexCount = 12,
.triangles = Wall_0_Door_1_indexes,
.triangleTextures = Wall_0_Door_1_textures,
.triangleUVs = Wall_0_Door_1_UVs,
.triangleTextureIndex = Wall_0_Door_1_indexes_texture,
.triangleTextureWidth = Wall_0_Door_1_textures_width,
.triangleTextureHeight = Wall_0_Door_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_0_Door_1_object_object = {
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
.model = &Wall_0_Door_1,
};
static const Engine_Object Wall_0_Door_1_object = {
.visual = Wall_0_Door_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
