#pragma once

extern Engine_Scene default_scene;

typedef struct {
	L3_Unit vx;
	L3_Unit vy;
	L3_Unit z;
	L3_Unit x;
	L3_Unit jump;
} Controls;

struct Default_scene_data {
	Engine_Object *player;
	L3_Unit		player_xrot;
	Controls	controls;
};
