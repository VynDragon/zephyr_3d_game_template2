#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/data/json.h>
#include "engine.h"

typedef struct Engine_Object_Serialize_s {
	int			visual_type;
	int			view_range;
	int			backfaceCulling;
	int			visible;
	int 		solid_color;
	L3_Transform3D	transform;
	const char		*visual_i;
	const char		*collisions;
} Engine_Object_Serialize;


static const struct json_obj_descr L3_Vec4_descr[] = {
	JSON_OBJ_DESCR_PRIM(L3_Vec4, x, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(L3_Vec4, y, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(L3_Vec4, z, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(L3_Vec4, w, JSON_TOK_INT),
};

static const struct json_obj_descr L3_Transform3D_descr[] = {
	JSON_OBJ_DESCR_OBJECT(L3_Transform3D, translation, L3_Vec4_descr),
	JSON_OBJ_DESCR_OBJECT(L3_Transform3D, rotation, L3_Vec4_descr),
	JSON_OBJ_DESCR_OBJECT(L3_Transform3D, scale, L3_Vec4_descr),
};

static const struct json_obj_descr Engine_Object_Serialize_descr[] = {
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, visual_type, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, view_range, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, backfaceCulling, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, visible, JSON_TOK_INT),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, solid_color, JSON_TOK_INT),
	JSON_OBJ_DESCR_OBJECT(Engine_Object_Serialize, transform, L3_Transform3D_descr),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, visual_i, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(Engine_Object_Serialize, collisions, JSON_TOK_STRING),
};

static Engine_Object_Serialize	engine_objects_serialize[CONFIG_MAX_OBJECTS] = {0};

static void objects_to_serialize()
{
	Engine_Object *objects = engine_getobjects();

	for (int i = 0; i < engine_object_getcnt(); i++) {
		engine_objects_serialize[i].visual_type = objects[i].visual_type;
		engine_objects_serialize[i].view_range = objects[i].view_range;
		engine_objects_serialize[i].backfaceCulling = objects[i].visual.config.backfaceCulling;
		engine_objects_serialize[i].visible = objects[i].visual.config.visible;
		engine_objects_serialize[i].transform = objects[i].visual.transform;
		engine_objects_serialize[i].solid_color = objects[i].visual.solid_color;
		int j = 0;
		bool found = false;
		for (; j < ARRAY_SIZE(generated_object_list); j++) {
			if (objects[i].visual.model == generated_object_list[j].visual.model ||
				objects[i].visual.billboard == generated_object_list[j].visual.billboard) {
				found = true;
				break;
			}
		}
		if (found) {
			engine_objects_serialize[i].visual_i = generated_object_visual_names[j];
			engine_objects_serialize[i].collisions = generated_object_collisions_names[j];
		} else {
			engine_objects_serialize[i].visual_i = 0;
			engine_objects_serialize[i].collisions = 0;
		}
	}
}

static void serialize_to_objects(int cnt)
{
	Engine_Object obj;
	obj.process = 0;
	obj.data = 0;

	for (int i = 0; i < cnt; i++) {
		obj.visual_type = engine_objects_serialize[i].visual_type;
		obj.view_range = engine_objects_serialize[i].view_range;
		obj.visual.config.backfaceCulling = engine_objects_serialize[i].backfaceCulling;
		obj.visual.config.visible = engine_objects_serialize[i].visible;
		obj.visual.transform = engine_objects_serialize[i].transform;
		obj.visual.solid_color = engine_objects_serialize[i].solid_color;
		int j = 0;
		bool found = false;
		for (; j < ARRAY_SIZE(generated_object_list); j++) {
			if (strcmp(generated_object_visual_names[j], engine_objects_serialize[i].visual_i) == 0) {
				found = true;
				break;
			}
		}
		if (obj.visual.config.visible & L3_VISIBLE_BILLBOARD) {
			obj.visual.billboard = generated_object_list[j].visual.billboard;
		} else {
			obj.visual.model = generated_object_list[j].visual.model;
		}
		obj.collisions = generated_object_list[j].collisions;
		engine_add_object(obj);
	}
}

int serialize_objects(char *buffer, size_t buflen)
{
	int ret = 0;
	size_t offset = 2;

	objects_to_serialize();
	buffer[0] = '[';
	buffer[1] = '\n';
	for (int i = 1; i < engine_object_getcnt(); i++) {
		ret = json_obj_encode_buf(Engine_Object_Serialize_descr, ARRAY_SIZE(Engine_Object_Serialize_descr), &(engine_objects_serialize[i]), buffer + offset, buflen - offset);
		offset += json_calc_encoded_len(Engine_Object_Serialize_descr, ARRAY_SIZE(Engine_Object_Serialize_descr), &(engine_objects_serialize[i])) + 2;
		if (i+1 < engine_object_getcnt()) {
			*(buffer + offset-2) = ',';
			*(buffer + offset-1) = '\n';
		} else {
			*(buffer + offset-2) = '\n';
			*(buffer + offset-1) = ']';
			*(buffer + offset) = 0;
		}
		if (ret < 0) return ret;
	}
	return ret;
}

int deserialize_objects(char *buffer, size_t buflen)
{
	int ret;
	struct json_obj json;
	size_t cnt = 0;

	ret = json_arr_separate_object_parse_init(&json, buffer, buflen);
	if (ret < 0) return ret;
	ret = json_arr_separate_parse_object(&json, Engine_Object_Serialize_descr, ARRAY_SIZE(Engine_Object_Serialize_descr), &(engine_objects_serialize[cnt]));
	while (ret > 0) {
		cnt++;
		ret = json_arr_separate_parse_object(&json, Engine_Object_Serialize_descr, ARRAY_SIZE(Engine_Object_Serialize_descr), &(engine_objects_serialize[cnt]));
	}

	serialize_to_objects(cnt);
	return ret;
}
