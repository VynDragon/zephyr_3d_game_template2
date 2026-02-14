#pragma once
static const L3_Unit Prefabs_Prefab_2_Top_1_vertices[] = {
4452,10204,-2049,
4452,2524,-2049,
4452,10204,1783,
4452,2524,1783,
-4460,10204,-2049,
-4460,2524,-2049,
-4460,10204,1783,
-4460,2524,1783,
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
static const L3_Texture *Prefabs_Prefab_2_Top_1_textures[] = {
&tex_facade_1,
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
static const L3_Unit Prefabs_Prefab_2_Top_1_Normals[] = {
-0.0 * L3_F,-0.0 * L3_F,1.0 * L3_F,
-1.0 * L3_F,-0.0 * L3_F,-0.0 * L3_F,
1.0 * L3_F,-0.0 * L3_F,-0.0 * L3_F,
-0.0 * L3_F,-0.0 * L3_F,-1.0 * L3_F,
-0.0 * L3_F,-0.0 * L3_F,1.0 * L3_F,
-1.0 * L3_F,-0.0 * L3_F,-0.0 * L3_F,
1.0 * L3_F,-0.0 * L3_F,-0.0 * L3_F,
-0.0 * L3_F,-0.0 * L3_F,-1.0 * L3_F,
};
static const L3_Model3D Prefabs_Prefab_2_Top_1 = {
.vertices = Prefabs_Prefab_2_Top_1_vertices,
.triangleCount = 8,
.vertexCount = 8,
.triangles = Prefabs_Prefab_2_Top_1_indexes,
.triangleTextures = Prefabs_Prefab_2_Top_1_textures,
.triangleUVs = Prefabs_Prefab_2_Top_1_UVs,
.triangleTextureIndex = Prefabs_Prefab_2_Top_1_indexes_texture,
.triangleNormals = Prefabs_Prefab_2_Top_1_Normals,
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
.config.visible = L3_VISIBLE_MODEL_TEXTURED,
.solid_color = 0xFF,
.model = &Prefabs_Prefab_2_Top_1,
};
static const Engine_Object Prefabs_Prefab_2_Top_1_object = {
.visual = Prefabs_Prefab_2_Top_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
