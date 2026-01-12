#pragma once
static const L3_Unit Prefabs_Prefab_2_Top_1_vertices[] = {
4542,10408,-2090,
4542,2574,-2090,
4542,10408,1818,
4542,2574,1818,
-4549,10408,-2090,
-4549,2574,-2090,
-4549,10408,1818,
-4549,2574,1818,
};
static const L3_Index Prefabs_Prefab_2_Top_1_indexes[] = {
2, 7, 3,
6, 5, 7,
0, 3, 1,
4, 1, 5,
2, 6, 7,
6, 4, 5,
0, 2, 3,
4, 0, 1,
};
static const L3_COLORTYPE *Prefabs_Prefab_2_Top_1_textures[] = {
tex_facade_1,
};
static const L3_Unit Prefabs_Prefab_2_Top_1_textures_width[] = {
256,
};
static const L3_Unit Prefabs_Prefab_2_Top_1_textures_height[] = {
256,
};
static const L3_Unit Prefabs_Prefab_2_Top_1_UVs[] = {
256, 0,
0, 256,
256, 256,
0, 0,
256, 256,
0, 256,
256, 0,
0, 256,
256, 256,
0, 0,
256, 256,
0, 256,
256, 0,
0, 0,
0, 256,
0, 0,
256, 0,
256, 256,
256, 0,
0, 0,
0, 256,
0, 0,
256, 0,
256, 256,
};
static const L3_Index Prefabs_Prefab_2_Top_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,};
static const L3_Model3D Prefabs_Prefab_2_Top_1 = {
.vertices = Prefabs_Prefab_2_Top_1_vertices,
.triangleCount = 8,
.vertexCount = 8,
.triangles = Prefabs_Prefab_2_Top_1_indexes,
.triangleTextures = Prefabs_Prefab_2_Top_1_textures,
.triangleUVs = Prefabs_Prefab_2_Top_1_UVs,
.triangleTextureIndex = Prefabs_Prefab_2_Top_1_indexes_texture,
.triangleTextureWidth = Prefabs_Prefab_2_Top_1_textures_width,
.triangleTextureHeight = Prefabs_Prefab_2_Top_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Prefabs_Prefab_2_Top_1_object_object = {
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
.model = &Prefabs_Prefab_2_Top_1,
};
static const Engine_Object Prefabs_Prefab_2_Top_1_object = {
.visual = Prefabs_Prefab_2_Top_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
