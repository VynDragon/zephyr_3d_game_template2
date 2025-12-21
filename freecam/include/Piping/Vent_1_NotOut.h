#pragma once
static const L3_Unit Piping_Vent_1_NotOut_vertices[] = {
-512,-256,256,
-512,256,256,
-512,-256,-256,
-512,256,-256,
512,-256,256,
512,256,256,
512,-256,-256,
512,256,-256,
};
static const L3_Index Piping_Vent_1_NotOut_indexes[] = {
3, 6, 2,
5, 0, 4,
6, 0, 2,
3, 5, 7,
3, 7, 6,
5, 1, 0,
6, 4, 0,
3, 1, 5,
};
static const L3_COLORTYPE *Piping_Vent_1_NotOut_textures[] = {
tex_Aluminium,
};
static const L3_Unit Piping_Vent_1_NotOut_textures_width[] = {
64,
};
static const L3_Unit Piping_Vent_1_NotOut_textures_height[] = {
64,
};
static const L3_Unit Piping_Vent_1_NotOut_UVs[] = {
3, 50,
30, 37,
3, 37,
30, 10,
3, 23,
30, 23,
30, 37,
3, 23,
3, 37,
3, 50,
30, 64,
30, 50,
3, 50,
30, 50,
30, 37,
30, 10,
3, 10,
3, 23,
30, 37,
30, 23,
3, 23,
3, 50,
3, 64,
30, 64,
};
static const L3_Index Piping_Vent_1_NotOut_indexes_texture[] = {
0,0,0,0,0,0,0,0,};
static const L3_Model3D Piping_Vent_1_NotOut = {
.vertices = Piping_Vent_1_NotOut_vertices,
.triangleCount = 8,
.vertexCount = 8,
.triangles = Piping_Vent_1_NotOut_indexes,
.triangleTextures = Piping_Vent_1_NotOut_textures,
.triangleUVs = Piping_Vent_1_NotOut_UVs,
.triangleTextureIndex = Piping_Vent_1_NotOut_indexes_texture,
.triangleTextureWidth = Piping_Vent_1_NotOut_textures_width,
.triangleTextureHeight = Piping_Vent_1_NotOut_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Piping_Vent_1_NotOut_object_object = {
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
.model = &Piping_Vent_1_NotOut,
};
static const Engine_Object Piping_Vent_1_NotOut_object = {
.visual = Piping_Vent_1_NotOut_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
