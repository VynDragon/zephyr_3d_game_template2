#pragma once
static const L3_Unit Vertical_Platform_vertices[] = {
-512,0,512,
512,0,512,
-512,0,-512,
512,0,-512,
-512,51,-512,
-512,51,512,
512,51,512,
512,51,-512,
};
static const L3_Index Vertical_Platform_indexes[] = {
2, 1, 0,
6, 4, 5,
3, 6, 1,
0, 4, 2,
2, 7, 3,
1, 5, 0,
2, 3, 1,
6, 7, 4,
3, 7, 6,
0, 5, 4,
2, 4, 7,
1, 6, 5,
};
static const L3_Model3D Vertical_Platform = {
.vertices = Vertical_Platform_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = Vertical_Platform_indexes,
.triangleTextures = NULL,
.triangleUVs = NULL,
.triangleTextureIndex = NULL,
.triangleTextureWidth = NULL,
.triangleTextureHeight = NULL,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Vertical_Platform_object_object = {
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
.config.visible = 4,
.solid_color = 0xFF,
.model = &Vertical_Platform,
};
static const Engine_Object Vertical_Platform_object = {
.visual = Vertical_Platform_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
