#pragma once
static const L3_Unit cube_vertices[] = {
256,256,-256,
256,-256,-256,
256,256,256,
256,-256,256,
-256,256,-256,
-256,-256,-256,
-256,256,256,
-256,-256,256,
};
static const L3_Index cube_indexes[] = {
4, 2, 0,
2, 7, 3,
6, 5, 7,
1, 7, 5,
0, 3, 1,
4, 1, 5,
4, 6, 2,
2, 6, 7,
6, 4, 5,
1, 3, 7,
0, 2, 3,
4, 0, 1,
};
static const L3_Model3D cube_model = {
.vertices = cube_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = cube_indexes,
};
static const L3_Object cube = {
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
.config.backfaceCulling = 0,
.config.visible = L3_VISIBLE_DISTANCELIGHT | L3_VISIBLE_SOLID | L3_VISIBLE_WIREFRAME,
.model = &cube_model,
};
