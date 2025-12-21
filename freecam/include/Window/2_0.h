#pragma once
static const L3_Unit Window_2_0_vertices[] = {
449,1054,0,
449,608,0,
-449,608,0,
-449,1054,0,
428,1032,-25,
428,629,-25,
-428,629,-25,
-428,1032,-25,
};
static const L3_Index Window_2_0_indexes[] = {
6, 4, 7,
1, 4, 5,
3, 6, 7,
2, 5, 6,
0, 7, 4,
6, 5, 4,
1, 0, 4,
3, 2, 6,
2, 1, 5,
0, 3, 7,
};
static const L3_COLORTYPE *Window_2_0_textures[] = {
tex_Window_1,
};
static const L3_Unit Window_2_0_textures_width[] = {
64,
};
static const L3_Unit Window_2_0_textures_height[] = {
64,
};
static const L3_Unit Window_2_0_UVs[] = {
0, 15,
32, 31,
0, 31,
33, 14,
32, 31,
32, 15,
-1, 32,
0, 15,
0, 31,
0, 14,
32, 15,
0, 15,
32, 33,
0, 31,
32, 31,
0, 15,
32, 15,
32, 31,
33, 14,
33, 32,
32, 31,
-1, 32,
-1, 14,
0, 15,
0, 14,
32, 14,
32, 15,
32, 33,
0, 33,
0, 31,
};
static const L3_Index Window_2_0_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Window_2_0 = {
.vertices = Window_2_0_vertices,
.triangleCount = 10,
.vertexCount = 8,
.triangles = Window_2_0_indexes,
.triangleTextures = Window_2_0_textures,
.triangleUVs = Window_2_0_UVs,
.triangleTextureIndex = Window_2_0_indexes_texture,
.triangleTextureWidth = Window_2_0_textures_width,
.triangleTextureHeight = Window_2_0_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Window_2_0_object_object = {
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
.model = &Window_2_0,
};
static const Engine_Object Window_2_0_object = {
.visual = Window_2_0_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
