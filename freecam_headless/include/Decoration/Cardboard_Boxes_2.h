#pragma once
static const L3_Unit Decoration_Cardboard_Boxes_2_vertices[] = {
-170,-5,453,
-170,334,453,
-170,-5,-112,
-170,334,-112,
170,-5,453,
170,334,453,
170,-5,-112,
170,334,-112,
};
static const L3_Index Decoration_Cardboard_Boxes_2_indexes[] = {
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
static const L3_COLORTYPE *Decoration_Cardboard_Boxes_2_textures[] = {
tex_Cardboard_Box,
};
static const L3_Unit Decoration_Cardboard_Boxes_2_textures_width[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_2_textures_height[] = {
64,
};
static const L3_Unit Decoration_Cardboard_Boxes_2_UVs[] = {
41, 23,
28, 5,
28, 23,
40, 8,
53, -4,
40, -4,
53, 4,
65, 22,
65, 4,
53, 20,
41, 32,
53, 32,
16, 0,
0, 16,
16, 16,
40, 5,
53, 23,
53, 4,
41, 23,
40, 5,
28, 5,
40, 8,
53, 7,
53, -4,
53, 4,
53, 23,
65, 22,
53, 20,
41, 20,
41, 32,
16, 0,
0, 0,
0, 16,
40, 5,
41, 23,
53, 23,
};
static const L3_Index Decoration_Cardboard_Boxes_2_indexes_texture[] = {
0,0,0,0,0,0,0,0,0,0,0,0,};
static const L3_Model3D Decoration_Cardboard_Boxes_2 = {
.vertices = Decoration_Cardboard_Boxes_2_vertices,
.triangleCount = 12,
.vertexCount = 8,
.triangles = Decoration_Cardboard_Boxes_2_indexes,
.triangleTextures = Decoration_Cardboard_Boxes_2_textures,
.triangleUVs = Decoration_Cardboard_Boxes_2_UVs,
.triangleTextureIndex = Decoration_Cardboard_Boxes_2_indexes_texture,
.triangleTextureWidth = Decoration_Cardboard_Boxes_2_textures_width,
.triangleTextureHeight = Decoration_Cardboard_Boxes_2_textures_height,
};
#pragma once
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif
static const L3_Object Decoration_Cardboard_Boxes_2_object_object = {
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
.model = &Decoration_Cardboard_Boxes_2,
};
static const Engine_Object Decoration_Cardboard_Boxes_2_object = {
.visual = Decoration_Cardboard_Boxes_2_object_object,
.collisions = NULL,
.visual_type = ENGINE_VISUAL_MODEL,
.view_range = 65536,
};
