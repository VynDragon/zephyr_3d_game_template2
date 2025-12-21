#pragma once
static const L3_Unit Wall_Garage_Door_0_vertices[] = {
-1024,0,-102,
1024,0,-102,
-1024,1433,-102,
1024,1433,-102,
};
static const L3_Index Wall_Garage_Door_0_indexes[] = {
1, 2, 0,
1, 3, 2,
};
static const L3_COLORTYPE *Wall_Garage_Door_0_textures[] = {
tex_Garage_Door_0,
};
static const L3_Unit Wall_Garage_Door_0_textures_width[] = {
64,
};
static const L3_Unit Wall_Garage_Door_0_textures_height[] = {
64,
};
static const L3_Unit Wall_Garage_Door_0_UVs[] = {
64, 64,
0, 0,
0, 64,
64, 64,
64, 0,
0, 0,
};
static const L3_Index Wall_Garage_Door_0_indexes_texture[] = {
0,0,};
static const L3_Model3D Wall_Garage_Door_0 = {
.vertices = Wall_Garage_Door_0_vertices,
.triangleCount = 2,
.vertexCount = 4,
.triangles = Wall_Garage_Door_0_indexes,
.triangleTextures = Wall_Garage_Door_0_textures,
.triangleUVs = Wall_Garage_Door_0_UVs,
.triangleTextureIndex = Wall_Garage_Door_0_indexes_texture,
.triangleTextureWidth = Wall_Garage_Door_0_textures_width,
.triangleTextureHeight = Wall_Garage_Door_0_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Wall_Garage_Door_0_object_object = {
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
.model = &Wall_Garage_Door_0,
};
static const Engine_Object Wall_Garage_Door_0_object = {
.visual = Wall_Garage_Door_0_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
