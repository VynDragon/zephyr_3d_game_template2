#pragma once
static const L3_Unit Wall_0_Window_3_1_vertices[] = {
-512,0,0,
512,0,0,
-512,1536,0,
512,1536,0,
512,545,0,
-512,545,0,
-512,1116,0,
512,1116,0,
};
static const L3_Index Wall_0_Window_3_1_indexes[] = {
6, 3, 2,
0, 4, 5,
6, 7, 3,
0, 1, 4,
};
static const L3_COLORTYPE *Wall_0_Window_3_1_textures[] = {
tex_Brick_Wall_2,
};
static const L3_Unit Wall_0_Window_3_1_textures_width[] = {
96,
};
static const L3_Unit Wall_0_Window_3_1_textures_height[] = {
96,
};
static const L3_Unit Wall_0_Window_3_1_UVs[] = {
72, 18,
0, -10,
72, -10,
72, 96,
0, 58,
72, 58,
72, 18,
0, 18,
0, -10,
72, 96,
0, 96,
0, 58,
};
static const L3_Index Wall_0_Window_3_1_indexes_texture[] = {
0,0,0,0,};
static const L3_Model3D Wall_0_Window_3_1 = {
.vertices = Wall_0_Window_3_1_vertices,
.triangleCount = 4,
.vertexCount = 8,
.triangles = Wall_0_Window_3_1_indexes,
.triangleTextures = Wall_0_Window_3_1_textures,
.triangleUVs = Wall_0_Window_3_1_UVs,
.triangleTextureIndex = Wall_0_Window_3_1_indexes_texture,
.triangleTextureWidth = Wall_0_Window_3_1_textures_width,
.triangleTextureHeight = Wall_0_Window_3_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_0_Window_3_1_object_object = {
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
.model = &Wall_0_Window_3_1,
};
static const Engine_Object Wall_0_Window_3_1_object = {
.visual = Wall_0_Window_3_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
