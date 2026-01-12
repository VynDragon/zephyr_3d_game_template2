#pragma once
static const L3_Unit Wall_Fence_1_Cap_0_vertices[] = {
512,1536,204,
512,0,76,
512,1382,76,
512,1382,204,
512,1536,-204,
512,0,-76,
512,1382,-76,
512,1382,-204,
};
static const L3_Index Wall_Fence_1_Cap_0_indexes[] = {
5, 2, 1,
2, 4, 3,
5, 6, 2,
6, 7, 4,
4, 0, 3,
2, 6, 4,
};
static const L3_COLORTYPE *Wall_Fence_1_Cap_0_textures[] = {
tex_Brick_Wall_1,
};
static const L3_Unit Wall_Fence_1_Cap_0_textures_width[] = {
96,
};
static const L3_Unit Wall_Fence_1_Cap_0_textures_height[] = {
96,
};
static const L3_Unit Wall_Fence_1_Cap_0_UVs[] = {
0, 96,
9, 0,
9, 96,
89, 36,
78, 55,
89, 27,
0, 96,
0, 0,
9, 0,
89, 47,
89, 55,
78, 55,
78, 55,
78, 27,
89, 27,
89, 36,
89, 47,
78, 55,
};
static const L3_Index Wall_Fence_1_Cap_0_indexes_texture[] = {
0,0,0,0,0,0,};
static const L3_Model3D Wall_Fence_1_Cap_0 = {
.vertices = Wall_Fence_1_Cap_0_vertices,
.triangleCount = 6,
.vertexCount = 8,
.triangles = Wall_Fence_1_Cap_0_indexes,
.triangleTextures = Wall_Fence_1_Cap_0_textures,
.triangleUVs = Wall_Fence_1_Cap_0_UVs,
.triangleTextureIndex = Wall_Fence_1_Cap_0_indexes_texture,
.triangleTextureWidth = Wall_Fence_1_Cap_0_textures_width,
.triangleTextureHeight = Wall_Fence_1_Cap_0_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_Fence_1_Cap_0_object_object = {
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
.model = &Wall_Fence_1_Cap_0,
};
static const Engine_Object Wall_Fence_1_Cap_0_object = {
.visual = Wall_Fence_1_Cap_0_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
