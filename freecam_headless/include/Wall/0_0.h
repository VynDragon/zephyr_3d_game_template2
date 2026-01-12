#pragma once
static const L3_Unit Wall_0_0_vertices[] = {
-512,0,0,
512,0,0,
-512,1536,0,
512,1536,0,
};
static const L3_Index Wall_0_0_indexes[] = {
0, 3, 2,
0, 1, 3,
};
static const L3_COLORTYPE *Wall_0_0_textures[] = {
tex_Brick_Wall_1,
};
static const L3_Unit Wall_0_0_textures_width[] = {
96,
};
static const L3_Unit Wall_0_0_textures_height[] = {
96,
};
static const L3_Unit Wall_0_0_UVs[] = {
72, 96,
0, -10,
72, -10,
72, 96,
0, 96,
0, -10,
};
static const L3_Index Wall_0_0_indexes_texture[] = {
0,0,};
static const L3_Model3D Wall_0_0 = {
.vertices = Wall_0_0_vertices,
.triangleCount = 2,
.vertexCount = 4,
.triangles = Wall_0_0_indexes,
.triangleTextures = Wall_0_0_textures,
.triangleUVs = Wall_0_0_UVs,
.triangleTextureIndex = Wall_0_0_indexes_texture,
.triangleTextureWidth = Wall_0_0_textures_width,
.triangleTextureHeight = Wall_0_0_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_0_0_object_object = {
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
.model = &Wall_0_0,
};
static const Engine_Object Wall_0_0_object = {
.visual = Wall_0_0_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
