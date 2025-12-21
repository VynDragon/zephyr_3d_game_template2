#pragma once
static const L3_Unit Road_Sidewalk_vertices[] = {
-4096,0,1792,
4096,0,1792,
-4096,0,-1792,
4096,0,-1792,
};
static const L3_Index Road_Sidewalk_indexes[] = {
1, 2, 0,
1, 3, 2,
};
static const L3_COLORTYPE *Road_Sidewalk_textures[] = {
tex_Road_1,
};
static const L3_Unit Road_Sidewalk_textures_width[] = {
192,
};
static const L3_Unit Road_Sidewalk_textures_height[] = {
192,
};
static const L3_Unit Road_Sidewalk_UVs[] = {
47, 0,
0, 47,
47, 47,
47, 0,
0, 0,
0, 47,
};
static const L3_Index Road_Sidewalk_indexes_texture[] = {
0,0,};
static const L3_Model3D Road_Sidewalk = {
.vertices = Road_Sidewalk_vertices,
.triangleCount = 2,
.vertexCount = 4,
.triangles = Road_Sidewalk_indexes,
.triangleTextures = Road_Sidewalk_textures,
.triangleUVs = Road_Sidewalk_UVs,
.triangleTextureIndex = Road_Sidewalk_indexes_texture,
.triangleTextureWidth = Road_Sidewalk_textures_width,
.triangleTextureHeight = Road_Sidewalk_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Road_Sidewalk_object_object = {
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
.model = &Road_Sidewalk,
};
static const Engine_Object Road_Sidewalk_object = {
.visual = Road_Sidewalk_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
