#pragma once
static const L3_Unit Window_3_0_vertices[] = {
512,545,0,
-512,545,0,
-512,1116,0,
512,1116,0,
475,1080,-30,
475,582,-30,
-475,582,-30,
-475,1080,-30,
};
static const L3_Index Window_3_0_indexes[] = {
6, 4, 7,
0, 4, 5,
1, 5, 6,
2, 6, 7,
3, 7, 4,
6, 5, 4,
0, 3, 4,
1, 0, 5,
2, 1, 6,
3, 2, 7,
};
static const L3_COLORTYPE *Window_3_0_textures[] = {
tex_Window_1,
};
static const L3_Unit Window_3_0_textures_width[] = {
64,
};
static const L3_Unit Window_3_0_textures_height[] = {
64,
};
static const L3_Unit Window_3_0_UVs[] = {
0, 15,
31, 32,
0, 32,
33, 14,
31, 32,
31, 15,
-1, 13,
31, 15,
0, 15,
-1, 33,
0, 15,
0, 32,
33, 33,
0, 32,
31, 32,
0, 15,
31, 15,
31, 32,
33, 14,
33, 33,
31, 32,
-1, 13,
33, 13,
31, 15,
-1, 33,
-1, 14,
0, 15,
33, 33,
-1, 33,
0, 32,
};
static const L3_Index Window_3_0_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Window_3_0 = {
.vertices = Window_3_0_vertices,
.triangleCount = 10,
.vertexCount = 8,
.triangles = Window_3_0_indexes,
.triangleTextures = Window_3_0_textures,
.triangleUVs = Window_3_0_UVs,
.triangleTextureIndex = Window_3_0_indexes_texture,
.triangleTextureWidth = Window_3_0_textures_width,
.triangleTextureHeight = Window_3_0_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Window_3_0_object_object = {
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
.model = &Window_3_0,
};
static const Engine_Object Window_3_0_object = {
.visual = Window_3_0_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
