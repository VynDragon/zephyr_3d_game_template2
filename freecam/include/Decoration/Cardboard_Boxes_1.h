#pragma once
static const L3_Unit Decoration_Cardboard_Boxes_1_vertices[] = {
-256,0,256,
-256,512,256,
-256,0,-256,
-256,512,-256,
256,0,256,
256,512,256,
256,0,-256,
256,512,-256,
};
static const L3_Index Decoration_Cardboard_Boxes_1_indexes[] = {
1, 2, 0,
3, 6, 2,
7, 4, 6,
5, 0, 4,
6, 0, 2,
3, 5, 7,
1, 3, 2,
3, 7, 6,
7, 5, 4,
5, 1, 0,
6, 4, 0,
3, 1, 5,
};
static const L3_COLORTYPE *Decoration_Cardboard_Boxes_1_textures[] = {
tex_Cardboard_Box,
};
static const L3_Unit Decoration_Cardboard_Boxes_1_textures_width[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_1_textures_height[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_1_UVs[] = {
41, 20,
28, 8,
28, 20,
40, 8,
53, -4,
40, -4,
53, 7,
65, 20,
65, 7,
53, 20,
41, 32,
53, 32,
16, 0,
0, 16,
16, 16,
40, 8,
53, 20,
53, 7,
41, 20,
40, 8,
28, 8,
40, 8,
53, 7,
53, -4,
53, 7,
53, 20,
65, 20,
53, 20,
41, 20,
41, 32,
16, 0,
0, 0,
0, 16,
40, 8,
41, 20,
53, 20,
};
static const L3_Index Decoration_Cardboard_Boxes_1_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Decoration_Cardboard_Boxes_1 = {
.vertices = Decoration_Cardboard_Boxes_1_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = Decoration_Cardboard_Boxes_1_indexes,
.triangleTextures = Decoration_Cardboard_Boxes_1_textures,
.triangleUVs = Decoration_Cardboard_Boxes_1_UVs,
.triangleTextureIndex = Decoration_Cardboard_Boxes_1_indexes_texture,
.triangleTextureWidth = Decoration_Cardboard_Boxes_1_textures_width,
.triangleTextureHeight = Decoration_Cardboard_Boxes_1_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Decoration_Cardboard_Boxes_1_object_object = {
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
.model = &Decoration_Cardboard_Boxes_1,
};
static const Engine_Object Decoration_Cardboard_Boxes_1_object = {
.visual = Decoration_Cardboard_Boxes_1_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
