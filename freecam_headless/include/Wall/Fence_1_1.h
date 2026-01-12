#pragma once
static const L3_Unit Wall_Fence_1_1_vertices[] = {
-512,1536,204,
512,1536,204,
-512,0,76,
512,0,76,
-512,1382,76,
512,1382,76,
-512,1382,204,
512,1382,204,
512,1536,-204,
-512,1536,-204,
512,0,-76,
-512,0,-76,
512,1382,-76,
-512,1382,-76,
512,1382,-204,
-512,1382,-204,
};
static const L3_Index Wall_Fence_1_1_indexes[] = {
2, 5, 4,
4, 7, 6,
6, 1, 0,
10, 13, 12,
12, 15, 14,
14, 9, 8,
1, 9, 0,
2, 3, 5,
4, 5, 7,
6, 7, 1,
10, 11, 13,
12, 13, 15,
14, 15, 9,
1, 8, 9,
};
static const L3_COLORTYPE *Wall_Fence_1_1_textures[] = {
tex_Brick_Wall_2,
};
static const L3_Unit Wall_Fence_1_1_textures_width[] = {
96,
};
static const L3_Unit Wall_Fence_1_1_textures_height[] = {
96,
};
static const L3_Unit Wall_Fence_1_1_UVs[] = {
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
0, 0,
72, 0,
93, 0,
87, 96,
87, 0,
87, 0,
79, 96,
79, 0,
75, 0,
95, 96,
75, 96,
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
0, 96,
0, 0,
93, 0,
93, 96,
87, 96,
87, 0,
87, 96,
79, 96,
75, 0,
95, 0,
95, 96,
};
static const L3_Index Wall_Fence_1_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Wall_Fence_1_1 = {
.vertices = Wall_Fence_1_1_vertices,
.triangleCount = 14,
.vertexCount = 16,
.triangles = Wall_Fence_1_1_indexes,
.triangleTextures = Wall_Fence_1_1_textures,
.triangleUVs = Wall_Fence_1_1_UVs,
.triangleTextureIndex = Wall_Fence_1_1_indexes_texture,
.triangleTextureWidth = Wall_Fence_1_1_textures_width,
.triangleTextureHeight = Wall_Fence_1_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_Fence_1_1_object_object = {
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
.model = &Wall_Fence_1_1,
};
static const Engine_Object Wall_Fence_1_1_object = {
.visual = Wall_Fence_1_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
