#pragma once
static const L3_Unit terrain_vertices[] = {
2559,1536,-5120,
-2560,1536,-5120,
2560,-511,5120,
-2559,-511,5120,
2559,1536,-15360,
-2560,1536,-15360,
};
static const L3_Index terrain_indexes[] = {
1, 2, 0,
0, 5, 1,
1, 3, 2,
0, 4, 5,
};
static const L3_Model3D terrain_model = {
.vertices = terrain_vertices,
.triangleCount = 4,
.vertexCount = 6,
.triangles = terrain_indexes,
};
static const L3_Object terrain = {
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
.config.visible = 2,
.solid_color = 0xFF,
.model = &terrain_model,
};
