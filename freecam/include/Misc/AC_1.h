#pragma once
static const L3_Unit Misc_AC_1_vertices[] = {
-253,810,346,
-253,1179,346,
-253,810,42,
-253,1179,42,
253,810,346,
253,1179,346,
253,810,42,
253,1179,42,
};
static const L3_Index Misc_AC_1_indexes[] = {
1, 2, 0,
3, 6, 2,
7, 4, 6,
5, 0, 4,
6, 0, 2,
3, 5, 7,
1, 3, 2,
3, 7, 6,
7, 5, 4,
5, 1, 0,
6, 4, 0,
3, 1, 5,
};
static const L3_COLORTYPE *Misc_AC_1_textures[] = {
tex_AC,
};
static const L3_Unit Misc_AC_1_textures_width[] = {
96,
};
static const L3_Unit Misc_AC_1_textures_height[] = {
96,
};
static const L3_Unit Misc_AC_1_UVs[] = {
21, 74,
48, 96,
48, 74,
21, 38,
47, 74,
47, 38,
21, 16,
48, 38,
48, 16,
21, 38,
48, 74,
48, 38,
69, 38,
48, 74,
69, 74,
0, 74,
21, 38,
0, 38,
21, 74,
21, 96,
48, 96,
21, 38,
21, 74,
47, 74,
21, 16,
21, 38,
48, 38,
21, 38,
21, 74,
48, 74,
69, 38,
48, 38,
48, 74,
0, 74,
21, 74,
21, 38,
};
static const L3_Index Misc_AC_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Misc_AC_1 = {
.vertices = Misc_AC_1_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = Misc_AC_1_indexes,
.triangleTextures = Misc_AC_1_textures,
.triangleUVs = Misc_AC_1_UVs,
.triangleTextureIndex = Misc_AC_1_indexes_texture,
.triangleTextureWidth = Misc_AC_1_textures_width,
.triangleTextureHeight = Misc_AC_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Misc_AC_1_object_object = {
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
.model = &Misc_AC_1,
};
static const Engine_Object Misc_AC_1_object = {
.visual = Misc_AC_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
