#pragma once
static const L3_Unit Prefabs_Prefab_2_Garage_Doors_1_vertices[] = {
-3120,0,1847,
-1031,0,1847,
-3120,1462,1847,
-1031,1462,1847,
13,0,1847,
2102,0,1847,
13,1462,1847,
2102,1462,1847,
};
static const L3_Index Prefabs_Prefab_2_Garage_Doors_1_indexes[] = {
1, 2, 0,
5, 6, 4,
1, 3, 2,
5, 7, 6,
};
static const L3_COLORTYPE *Prefabs_Prefab_2_Garage_Doors_1_textures[] = {
tex_Garage_Door_0,
};
static const L3_Unit Prefabs_Prefab_2_Garage_Doors_1_textures_width[] = {
64,
};
static const L3_Unit Prefabs_Prefab_2_Garage_Doors_1_textures_height[] = {
64,
};
static const L3_Unit Prefabs_Prefab_2_Garage_Doors_1_UVs[] = {
64, 64,
0, 0,
0, 64,
64, 64,
0, 0,
0, 64,
64, 64,
64, 0,
0, 0,
64, 64,
64, 0,
0, 0,
};
static const L3_Index Prefabs_Prefab_2_Garage_Doors_1_indexes_texture[] = {
0,0,0,0,};
static const L3_Model3D Prefabs_Prefab_2_Garage_Doors_1 = {
.vertices = Prefabs_Prefab_2_Garage_Doors_1_vertices,
.triangleCount = 4,
.vertexCount = 8,
.triangles = Prefabs_Prefab_2_Garage_Doors_1_indexes,
.triangleTextures = Prefabs_Prefab_2_Garage_Doors_1_textures,
.triangleUVs = Prefabs_Prefab_2_Garage_Doors_1_UVs,
.triangleTextureIndex = Prefabs_Prefab_2_Garage_Doors_1_indexes_texture,
.triangleTextureWidth = Prefabs_Prefab_2_Garage_Doors_1_textures_width,
.triangleTextureHeight = Prefabs_Prefab_2_Garage_Doors_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Prefabs_Prefab_2_Garage_Doors_1_object_object = {
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
.model = &Prefabs_Prefab_2_Garage_Doors_1,
};
static const Engine_Object Prefabs_Prefab_2_Garage_Doors_1_object = {
.visual = Prefabs_Prefab_2_Garage_Doors_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
