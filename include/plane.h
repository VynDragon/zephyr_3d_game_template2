#pragma once
static const L3_Unit plane_vertices[] = {
-256,0,256,
256,0,256,
-256,0,-256,
256,0,-256,
};
static const L3_Index plane_indexes[] = {
1, 2, 0,
1, 3, 2,
};
static const L3_Model3D plane_model = {
.vertices = plane_vertices,
.triangleCount = 2,
.vertexCount = 4,
.triangles = plane_indexes,
};
static const L3_Object plane = {
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
.config.visible = L3_VISIBLE_WIREFRAME,
.model = &plane_model,
};
