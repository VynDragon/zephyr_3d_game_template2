#pragma once
static const L3_Unit Wall_1_2_vertices[] = {
-512,1536,128,
512,1536,128,
-512,0,0,
512,0,0,
-512,1382,0,
512,1382,0,
-512,1382,128,
512,1382,128,
-512,1536,0,
512,1536,0,
};
static const L3_Index Wall_1_2_indexes[] = {
2, 5, 4,
4, 7, 6,
6, 1, 0,
9, 0, 1,
2, 3, 5,
4, 5, 7,
6, 7, 1,
9, 8, 0,
};
static const L3_COLORTYPE *Wall_1_2_textures[] = {
tex_Concrete_Wall_1,
};
static const L3_Unit Wall_1_2_textures_width[] = {
96,
};
static const L3_Unit Wall_1_2_textures_height[] = {
96,
};
static const L3_Unit Wall_1_2_UVs[] = {
72, 96,
0, 0,
72, 0,
93, 0,
87, 96,
87, 0,
87, 0,
79, 96,
79, 0,
72, 96,
79, 0,
79, 96,
72, 96,
0, 96,
0, 0,
93, 0,
93, 96,
87, 96,
87, 0,
87, 96,
79, 96,
72, 96,
72, 0,
79, 0,
};
static const L3_Index Wall_1_2_indexes_texture[] = {
0,0,0,0,0,0,0,0,};
static const L3_Model3D Wall_1_2 = {
.vertices = Wall_1_2_vertices,
.triangleCount = 8,
.vertexCount = 10,
.triangles = Wall_1_2_indexes,
.triangleTextures = Wall_1_2_textures,
.triangleUVs = Wall_1_2_UVs,
.triangleTextureIndex = Wall_1_2_indexes_texture,
.triangleTextureWidth = Wall_1_2_textures_width,
.triangleTextureHeight = Wall_1_2_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_1_2_object_object = {
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
.model = &Wall_1_2,
};
static const Engine_Object Wall_1_2_object = {
.visual = Wall_1_2_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
