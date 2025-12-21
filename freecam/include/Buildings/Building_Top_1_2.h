#pragma once
static const L3_Unit Buildings_Building_Top_1_2_vertices[] = {
3916,15667,-3916,
3916,0,-3916,
3916,15667,3916,
3916,0,3916,
-3916,15667,-3916,
-3916,0,-3916,
-3916,15667,3916,
-3916,0,3916,
};
static const L3_Index Buildings_Building_Top_1_2_indexes[] = {
2, 7, 3,
6, 5, 7,
0, 3, 1,
4, 1, 5,
2, 6, 7,
6, 4, 5,
0, 2, 3,
4, 0, 1,
};
static const L3_COLORTYPE *Buildings_Building_Top_1_2_textures[] = {
tex_facade_2,
};
static const L3_Unit Buildings_Building_Top_1_2_textures_width[] = {
64,
};
static const L3_Unit Buildings_Building_Top_1_2_textures_height[] = {
64,
};
static const L3_Unit Buildings_Building_Top_1_2_UVs[] = {
64, -64,
0, 64,
64, 64,
0, -64,
64, 64,
0, 64,
64, -64,
0, 64,
64, 64,
0, -64,
64, 64,
0, 64,
64, -64,
0, -64,
0, 64,
0, -64,
64, -64,
64, 64,
64, -64,
0, -64,
0, 64,
0, -64,
64, -64,
64, 64,
};
static const L3_Index Buildings_Building_Top_1_2_indexes_texture[] = {
0,0,0,0,0,0,0,0,};
static const L3_Model3D Buildings_Building_Top_1_2 = {
.vertices = Buildings_Building_Top_1_2_vertices,
.triangleCount = 8,
.vertexCount = 8,
.triangles = Buildings_Building_Top_1_2_indexes,
.triangleTextures = Buildings_Building_Top_1_2_textures,
.triangleUVs = Buildings_Building_Top_1_2_UVs,
.triangleTextureIndex = Buildings_Building_Top_1_2_indexes_texture,
.triangleTextureWidth = Buildings_Building_Top_1_2_textures_width,
.triangleTextureHeight = Buildings_Building_Top_1_2_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Buildings_Building_Top_1_2_object_object = {
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
.model = &Buildings_Building_Top_1_2,
};
static const Engine_Object Buildings_Building_Top_1_2_object = {
.visual = Buildings_Building_Top_1_2_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
