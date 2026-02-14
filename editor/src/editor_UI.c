#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/input/input.h>
#include <zephyr/timing/timing.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/random/random.h>
#include <math.h>
#include <lvgl.h>
#include <lvgl_mem.h>
#include <lvgl_zephyr.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(editor_ui);

#include "editor.h"

#include "engine.h"

#include "generated_objects.h"

#include "serialize.h"

typedef struct UI_touch_point_s {
	L3_Unit x;
	L3_Unit y;
	bool touch;
} UI_touch_point;

static lv_display_t *lv_displays[DT_ZEPHYR_DISPLAYS_COUNT];

static lv_display_t *lvgl_display;

static lv_obj_t *select_object_list;
static lv_obj_t *save;
static lv_obj_t *load;

static Engine_Object *selected_object = NULL;

static lv_obj_t *object_edit;

typedef struct Object_edit_data_t {
	lv_obj_t *area_x;
	lv_obj_t *area_y;
	lv_obj_t *area_z;
	lv_obj_t *area_sx;
	lv_obj_t *area_sy;
	lv_obj_t *area_sz;
	lv_obj_t *area_rx;
	lv_obj_t *area_ry;
	lv_obj_t *area_rz;
	lv_obj_t *focused;
	lv_obj_t *visible;
	lv_obj_t *delete;
	lv_obj_t *view_range;
	lv_obj_t *backfaceCulling;
	lv_obj_t *visible_tag;
	lv_obj_t *solid_color;
	bool inhibit_override;
} Object_edit_data;

static Object_edit_data object_edit_data = {0};

lv_style_t style_transp;

lv_obj_t * background_render;
static lv_img_dsc_t background_render_img;
extern L3_COLORTYPE blit_lvgl_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];


void update_object_edit()
{
	char buffer[64] = {0};

	object_edit_data.inhibit_override = true;
	snprintf(buffer, 63, "%d", selected_object->visual.transform.translation.x);
	lv_textarea_set_text(object_edit_data.area_x, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.translation.y);
	lv_textarea_set_text(object_edit_data.area_y, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.translation.z);
	lv_textarea_set_text(object_edit_data.area_z, buffer);

	snprintf(buffer, 63, "%d", selected_object->visual.transform.rotation.x);
	lv_textarea_set_text(object_edit_data.area_rx, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.rotation.y);
	lv_textarea_set_text(object_edit_data.area_ry, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.rotation.z);
	lv_textarea_set_text(object_edit_data.area_rz, buffer);

	snprintf(buffer, 63, "%d", selected_object->visual.transform.scale.x);
	lv_textarea_set_text(object_edit_data.area_sx, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.scale.y);
	lv_textarea_set_text(object_edit_data.area_sy, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.transform.scale.z);
	lv_textarea_set_text(object_edit_data.area_sz, buffer);

	snprintf(buffer, 63, "%d", selected_object->view_range);
	lv_textarea_set_text(object_edit_data.view_range, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.config.backfaceCulling);
	lv_textarea_set_text(object_edit_data.backfaceCulling, buffer);
	snprintf(buffer, 63, "%d", selected_object->visual.solid_color);
	lv_textarea_set_text(object_edit_data.solid_color, buffer);

	int i = 0;
	for (; i < 16; i++) {
		if (selected_object->visual.config.visible & (1 << i))
			buffer[i] = '1';
		else
			buffer[i] = '0';
	}
	buffer[i] = 0;
	lv_textarea_set_text(object_edit_data.visible_tag, buffer);

	if (selected_object->visual_type > ENGINE_VISUAL_NOTHING)
	{
		lv_obj_add_state(object_edit_data.visible, LV_STATE_CHECKED);
	} else {
		lv_obj_remove_state(object_edit_data.visible, LV_STATE_CHECKED);
	}

	object_edit_data.inhibit_override = false;
}

static void add_obj(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target_obj(e);
	size_t id = (size_t)lv_event_get_user_data(e);
	if(code == LV_EVENT_CLICKED) {
		L3_Camera *camera = engine_getcamera();
		L3_Vec4 forward = {0, 0, L3_F, L3_F};
		Engine_Object *engine_obj = engine_add_object(generated_object_list[id]);
		L3_Mat4 transMat;

		L3_makeRotationMatrixZXY(camera->transform.rotation.x,
						camera->transform.rotation.y,
						camera->transform.rotation.z,
						transMat);
		L3_vec3Xmat4(&forward, transMat);
		forward.x *= 10;
		forward.y *= 10;
		forward.z *= 10;

		forward.x += camera->transform.translation.x;
		forward.y += camera->transform.translation.y;
		forward.z += camera->transform.translation.z;

		forward.x = (forward.x >> 9) << 9;
		forward.y = (forward.y >> 10) << 10;
		forward.z = (forward.z >> 9) << 9;

		L3_transform3DSet(forward.x, forward.y, forward.z,0,0,0,L3_F,L3_F,L3_F,&(engine_obj->visual.transform));
		if (engine_obj->collisions != NULL) {
		}
		selected_object = engine_obj;
		update_object_edit();
		printf("Adding: %s, id: %d\n", lv_list_get_button_text(select_object_list, obj), id);
		printf("objcount: %d\n", engine_object_getcnt() + engine_statics_getcnt());
	}
}

static char saveloadbuffer[1048575];

/* need share same compilation unit */
#include "serialize.c"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


static void save_it(lv_event_t * e)
{
	serialize_objects(saveloadbuffer, 1048575);
	int fd = open("map.json", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	write(fd, saveloadbuffer, strlen(saveloadbuffer));
	close(fd);
}

static void load_it(lv_event_t * e)
{
	engine_remove_all_objects_past(1);
	int fd = open("map.json", O_RDONLY, S_IRUSR | S_IWUSR);
	read(fd, saveloadbuffer, 1048575);
	deserialize_objects(saveloadbuffer, strlen(saveloadbuffer));
	close(fd);
}

static void remove_current_obj()
{
	if (selected_object != NULL)
	{
		engine_remove_object(selected_object);
		engine_optimize_object_table();
		selected_object = NULL;
	}
}

static void object_edit_changed(lv_event_t * e)
{
	if (object_edit_data.inhibit_override) return;

	selected_object->visual.transform.translation.x = atoi(lv_textarea_get_text(object_edit_data.area_x));
	selected_object->visual.transform.translation.y = atoi(lv_textarea_get_text(object_edit_data.area_y));
	selected_object->visual.transform.translation.z = atoi(lv_textarea_get_text(object_edit_data.area_z));
	selected_object->visual.transform.rotation.x = atoi(lv_textarea_get_text(object_edit_data.area_rx));
	selected_object->visual.transform.rotation.y = atoi(lv_textarea_get_text(object_edit_data.area_ry));
	selected_object->visual.transform.rotation.z = atoi(lv_textarea_get_text(object_edit_data.area_rz));
	selected_object->visual.transform.scale.x = atoi(lv_textarea_get_text(object_edit_data.area_sx));
	selected_object->visual.transform.scale.y = atoi(lv_textarea_get_text(object_edit_data.area_sy));
	selected_object->visual.transform.scale.z = atoi(lv_textarea_get_text(object_edit_data.area_sz));

	selected_object->view_range = atoi(lv_textarea_get_text(object_edit_data.view_range));
	selected_object->visual.config.backfaceCulling = atoi(lv_textarea_get_text(object_edit_data.backfaceCulling));
	selected_object->visual.solid_color = atoi(lv_textarea_get_text(object_edit_data.solid_color));

	const char * visible_tag = lv_textarea_get_text(object_edit_data.visible_tag);
	for (int i = 0; i < strlen(visible_tag); i++) {
		if (visible_tag[i] == '1')
			selected_object->visual.config.visible |= (1 << i);
		else
			selected_object->visual.config.visible &= ~(1 << i);
	}

	if (lv_obj_has_state(object_edit_data.visible, LV_STATE_CHECKED))
	{
		if ((selected_object->visual.config.visible & L3_VISIBLE_BILLBOARD) != 0) {
			selected_object->visual_type = ENGINE_VISUAL_BILLBOARD;
		} else {
			selected_object->visual_type = ENGINE_VISUAL_MODEL;
		}
	} else {
		selected_object->visual_type = ENGINE_VISUAL_NOTHING;
	}
	if (lv_event_get_current_target(e) == object_edit_data.delete && lv_event_get_code(e) == LV_EVENT_CLICKED)
	{
		remove_current_obj();
	}
}

static void object_edit_focused(lv_event_t * e)
{
	lv_obj_t *focused = lv_event_get_user_data(e);
	object_edit_data.focused = focused;
}

static void object_edit_defocused(lv_event_t * e)
{
	lv_obj_t *focused = lv_event_get_user_data(e);
	if (object_edit_data.focused == focused)
		object_edit_data.focused = NULL;
}

void object_edit_initialize()
{
	object_edit = lv_obj_create(lv_screen_active());
	lv_obj_set_size(object_edit, 132, 128);
	lv_obj_align(object_edit, LV_ALIGN_BOTTOM_LEFT, 0,0);
	lv_obj_add_style(object_edit, &style_transp, 0);

	lv_obj_t * label = lv_label_create(object_edit);
	lv_label_set_text(label, "POS");
	lv_obj_set_pos(label, 0, 2);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "x");
	lv_obj_set_pos(label, 14, 2);
	object_edit_data.area_x = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_x, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_x, "0123456789-");
	lv_obj_set_size(object_edit_data.area_x, 32, 8);
	lv_obj_add_style(object_edit_data.area_x, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_x, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_x, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_x);
	lv_obj_add_event_cb(object_edit_data.area_x, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_x);
	lv_obj_set_pos(object_edit_data.area_x, 18, 0);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "y");
	lv_obj_set_pos(label, 52, 2);
	object_edit_data.area_y = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_y, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_y, "0123456789-");
	lv_obj_set_size(object_edit_data.area_y, 32, 8);
	lv_obj_add_style(object_edit_data.area_y, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_y, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_y, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_y);
	lv_obj_add_event_cb(object_edit_data.area_y, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_y);
	lv_obj_set_pos(object_edit_data.area_y, 56, 0);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "z");
	lv_obj_set_pos(label, 90, 2);
	object_edit_data.area_z = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_z, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_z, "0123456789-");
	lv_obj_set_size(object_edit_data.area_z, 32, 8);
	lv_obj_add_style(object_edit_data.area_z, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_z, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_z, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_z);
	lv_obj_add_event_cb(object_edit_data.area_z, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_z);
	lv_obj_set_pos(object_edit_data.area_z, 94, 0);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "ROT");
	lv_obj_set_pos(label, 0, 14);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "x");
	lv_obj_set_pos(label, 14, 14);
	object_edit_data.area_rx = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_rx, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_rx, "0123456789-");
	lv_obj_set_size(object_edit_data.area_rx, 32, 8);
	lv_obj_add_style(object_edit_data.area_rx, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_rx, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_rx, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_rx);
	lv_obj_add_event_cb(object_edit_data.area_rx, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_rx);
	lv_obj_set_pos(object_edit_data.area_rx, 18, 12);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "y");
	lv_obj_set_pos(label, 52, 14);
	object_edit_data.area_ry = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_ry, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_ry, "0123456789-");
	lv_obj_set_size(object_edit_data.area_ry, 32, 8);
	lv_obj_add_style(object_edit_data.area_ry, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_ry, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_ry, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_ry);
	lv_obj_add_event_cb(object_edit_data.area_ry, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_ry);
	lv_obj_set_pos(object_edit_data.area_ry, 56, 12);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "z");
	lv_obj_set_pos(label, 90, 14);
	object_edit_data.area_rz = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_rz, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_rz, "0123456789-");
	lv_obj_set_size(object_edit_data.area_rz, 32, 8);
	lv_obj_add_style(object_edit_data.area_rz, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_rz, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_rz, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_rz);
	lv_obj_add_event_cb(object_edit_data.area_rz, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_rz);
	lv_obj_set_pos(object_edit_data.area_rz, 94, 12);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "SCAL");
	lv_obj_set_pos(label, 0, 26);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "x");
	lv_obj_set_pos(label, 14, 26);
	object_edit_data.area_sx = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_sx, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_sx, "0123456789-");
	lv_obj_set_size(object_edit_data.area_sx, 32, 8);
	lv_obj_add_style(object_edit_data.area_sx, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_sx, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_sx, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_sx);
	lv_obj_add_event_cb(object_edit_data.area_sx, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_sx);
	lv_obj_set_pos(object_edit_data.area_sx, 18, 24);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "y");
	lv_obj_set_pos(label, 52, 26);
	object_edit_data.area_sy = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_sy, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_sy, "0123456789-");
	lv_obj_set_size(object_edit_data.area_sy, 32, 8);
	lv_obj_add_style(object_edit_data.area_sy, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_sy, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_sy, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_sy);
	lv_obj_add_event_cb(object_edit_data.area_sy, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_sy);
	lv_obj_set_pos(object_edit_data.area_sy, 56, 24);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "z");
	lv_obj_set_pos(label, 90, 26);
	object_edit_data.area_sz = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.area_sz, true);
	lv_textarea_set_accepted_chars(object_edit_data.area_sz, "0123456789-");
	lv_obj_set_size(object_edit_data.area_sz, 32, 8);
	lv_obj_add_style(object_edit_data.area_sz, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.area_sz, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.area_sz, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.area_sz);
	lv_obj_add_event_cb(object_edit_data.area_sz, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.area_sz);
	lv_obj_set_pos(object_edit_data.area_sz, 94, 24);


	label = lv_label_create(object_edit);
	lv_label_set_text(label, "View Range");
	lv_obj_set_pos(label, 0, 38);
	object_edit_data.view_range = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.view_range, true);
	lv_textarea_set_accepted_chars(object_edit_data.view_range, "0123456789-");
	lv_obj_set_size(object_edit_data.view_range, 32, 8);
	lv_obj_add_style(object_edit_data.view_range, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.view_range, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.view_range, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.view_range);
	lv_obj_add_event_cb(object_edit_data.view_range, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.view_range);
	lv_obj_set_pos(object_edit_data.view_range, 42, 36);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "bfc");
	lv_obj_set_pos(label, 76, 38);
	object_edit_data.backfaceCulling = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.backfaceCulling, true);
	lv_textarea_set_accepted_chars(object_edit_data.backfaceCulling, "012");
	lv_textarea_set_max_length(object_edit_data.backfaceCulling, 1);
	lv_obj_set_size(object_edit_data.backfaceCulling, 32, 8);
	lv_obj_add_style(object_edit_data.backfaceCulling, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.backfaceCulling, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.backfaceCulling, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.backfaceCulling);
	lv_obj_add_event_cb(object_edit_data.backfaceCulling, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.backfaceCulling);
	lv_obj_set_pos(object_edit_data.backfaceCulling, 88, 36);


	object_edit_data.visible = lv_checkbox_create(object_edit);
	lv_obj_add_style(object_edit_data.visible, &style_transp, 0);
	lv_checkbox_set_text(object_edit_data.visible, "Visible");
	lv_obj_set_pos(object_edit_data.visible, 0, 50);
	lv_obj_set_size(object_edit_data.visible, 48, 16);
	lv_obj_set_style_pad_ver(object_edit_data.visible, 0, 0);
	lv_obj_set_style_radius(object_edit_data.visible, 0, 0);
	lv_obj_set_style_pad_all(object_edit_data.visible, 0, LV_PART_INDICATOR);
	lv_obj_set_style_radius(object_edit_data.visible, 0, LV_PART_INDICATOR);
	lv_obj_add_event_cb(object_edit_data.visible, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "rndr bits");
	lv_obj_set_pos(label, 0, 64);
	object_edit_data.visible_tag = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.visible_tag, true);
	lv_textarea_set_accepted_chars(object_edit_data.visible_tag, "01");
	lv_textarea_set_max_length(object_edit_data.visible_tag, 16);
	lv_obj_set_size(object_edit_data.visible_tag, 72, 8);
	lv_obj_add_style(object_edit_data.visible_tag, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.visible_tag, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.visible_tag, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.visible_tag);
	lv_obj_add_event_cb(object_edit_data.visible_tag, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.visible_tag);
	lv_obj_set_pos(object_edit_data.visible_tag, 34, 62);

	label = lv_label_create(object_edit);
	lv_label_set_text(label, "Color");
	lv_obj_set_pos(label, 0, 76);
	object_edit_data.solid_color = lv_textarea_create(object_edit);
	lv_textarea_set_one_line(object_edit_data.solid_color, true);
	lv_textarea_set_accepted_chars(object_edit_data.solid_color, "0123456789");
	lv_obj_set_size(object_edit_data.solid_color, 32, 8);
	lv_obj_add_style(object_edit_data.solid_color, &style_transp, 0);
	lv_obj_add_event_cb(object_edit_data.solid_color, object_edit_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(object_edit_data.solid_color, object_edit_focused, LV_EVENT_FOCUSED , object_edit_data.solid_color);
	lv_obj_add_event_cb(object_edit_data.solid_color, object_edit_defocused, LV_EVENT_DEFOCUSED , object_edit_data.solid_color);
	lv_obj_set_pos(object_edit_data.solid_color, 24, 74);


	object_edit_data.delete = lv_button_create(object_edit);
	lv_obj_add_style(object_edit_data.delete, &style_transp, 0);
	lv_obj_set_pos(object_edit_data.delete, 90, 96);
	lv_obj_set_size(object_edit_data.delete, 32, 16);
	lv_obj_add_event_cb(object_edit_data.delete, object_edit_changed, LV_EVENT_CLICKED, NULL);
	label = lv_label_create(object_edit_data.delete);
	lv_obj_add_style(label, &style_transp, 0);
    lv_label_set_text(label, "Remove");
    lv_obj_center(label);

	lv_obj_add_flag(object_edit, LV_OBJ_FLAG_HIDDEN);

}

int init_editor_UI(void)
{
	lv_display_t *d = NULL;
	lv_obj_t *label;

	/* inverted order for some reason */
	for (int i = 0; i < DT_ZEPHYR_DISPLAYS_COUNT; i++) {
		d = lv_display_get_next(d);
		if (d == NULL) {
			printf("Invalid LV display %d object", i);
			return 0;
		}
		lv_displays[i] = d;
	}
	lvgl_display = lv_displays[1];

	lv_display_set_default(lvgl_display);
	engine_UI_set_area(0, 0, L3_RESOLUTION_X, L3_RESOLUTION_Y);
	extern int set_lvgl_rendering_cb(lv_display_t *display);
	set_lvgl_rendering_cb(lvgl_display);

	background_render = lv_img_create(lv_screen_active());
	background_render_img.header.magic = LV_IMAGE_HEADER_MAGIC;
	background_render_img.header.w = L3_RESOLUTION_X;
	background_render_img.header.h = L3_RESOLUTION_Y;
	background_render_img.data_size = L3_RESOLUTION_X * L3_RESOLUTION_Y;
	background_render_img.header.cf = LV_COLOR_FORMAT_L8;
	background_render_img.data = (uint8_t *)blit_lvgl_buffer;
	lv_img_set_src(background_render, &background_render_img);
	lv_obj_set_size(background_render, 1024, 512);
	lv_obj_center(background_render);

	select_object_list = lv_list_create(lv_screen_active());
	lv_obj_set_size(select_object_list, 256, 512);
	lv_obj_align(select_object_list, LV_ALIGN_TOP_RIGHT, 0,0);



	lv_style_init(&style_transp);
	lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
	//lv_style_set_border_width(&style_transp, 0);
	lv_style_set_text_color(&style_transp, lv_color_hex3(0xFFF));
	lv_style_set_radius(&style_transp, 0);
	lv_style_set_border_width(&style_transp, 1);
	lv_style_set_pad_all(&style_transp, 1);
	lv_obj_add_style(select_object_list, &style_transp, 0);

	load = lv_button_create(lv_screen_active());
	lv_obj_add_style(load, &style_transp, 0);
	//lv_obj_set_pos(load, 48, 50);
	lv_obj_set_size(load, 32, 16);
	lv_obj_add_event_cb(load, load_it, LV_EVENT_CLICKED, NULL);
	label = lv_label_create(load);
	lv_obj_add_style(label, &style_transp, 0);
    lv_label_set_text(label, "LOAD");
    lv_obj_center(label);
	lv_obj_align(load, LV_ALIGN_TOP_LEFT, 0,0);

	save = lv_button_create(lv_screen_active());
	lv_obj_add_style(save, &style_transp, 0);
	//lv_obj_set_pos(load, 48, 50);
	lv_obj_set_size(save, 32, 16);
	lv_obj_add_event_cb(save, save_it, LV_EVENT_CLICKED, NULL);
	label = lv_label_create(save);
	lv_obj_add_style(label, &style_transp, 0);
    lv_label_set_text(label, "SAVE");
    lv_obj_center(label);
	lv_obj_align(save, LV_ALIGN_TOP_LEFT, 32,0);

	object_edit_initialize();

	for (int i = 0; i < ARRAY_SIZE(generated_object_list); i++) {
		lv_obj_t * btn;
		btn = lv_list_add_button(select_object_list, NULL, generated_object_list_names[i]);
		lv_obj_add_style(btn, &style_transp, 0);
		lv_obj_add_event_cb(btn, add_obj, LV_EVENT_CLICKED, (void*)i);
	}

	return 0;
}


static UI_touch_point main_touch_point;

typedef struct {
	int dummy;
} Main_keys;

static Main_keys main_keys = {0};

static void update_selection_pseudoray(L3_Vec4 start_point, L3_Vec4 direction)
{
	Engine_Object *closest = NULL;
	int closest_distance_camera = INT_MAX;
	int closest_distance_point = INT_MAX;

	for (int i = 1; i < 65536; i+=16)
	{
		/* obj 1 is camera */
		for (size_t ob = 1; ob < engine_object_getcnt(); ob++) {
			Engine_Object *obj = &(engine_getobjects()[ob]);
			L3_Vec4 point = start_point;
			point.x += direction.x * i / L3_F;
			point.y += direction.y * i / L3_F;
			point.z += direction.z * i / L3_F;
			int distc = L3_distanceManhattan(obj->visual.transform.translation, start_point);
			int distp = L3_distanceManhattan(obj->visual.transform.translation, point);
			if (distc < closest_distance_camera && distp < closest_distance_point && distp < distc / 2) {
				closest = obj;
				closest_distance_camera = distc;
				closest_distance_point = distp;
			}
		}
	}
	selected_object = closest;
	if (selected_object != NULL) {
		update_object_edit();
	}
}

static void update_selection()
{
	L3_Camera *camera = engine_getcamera();
	L3_Vec4 forward = {0, 0, L3_F, L3_F};
	L3_Mat4 transMat;

	L3_makeRotationMatrixZXY(camera->transform.rotation.x,
					camera->transform.rotation.y,
					camera->transform.rotation.z,
					transMat);
	L3_vec3Xmat4(&forward, transMat);

	update_selection_pseudoray(camera->transform.translation, forward);
}

static void update_selection_mouse()
{
	L3_Camera *camera = engine_getcamera();
	L3_Vec4 forward = {0, 0, L3_F, L3_F};
	forward.x = (main_touch_point.x - CONFIG_RESOLUTION_X / 2) * 2;
	forward.y = (-main_touch_point.y + CONFIG_RESOLUTION_Y / 2) * 2;
	L3_Mat4 transMat;

	L3_makeRotationMatrixZXY(camera->transform.rotation.x,
					camera->transform.rotation.y,
					camera->transform.rotation.z,
					transMat);
	L3_vec3Xmat4(&forward, transMat);
	L3_vec3Normalize(&forward);

	update_selection_pseudoray(camera->transform.translation, forward);
}

static void main_touch(struct input_event *evt, void *user_data)
{
	UI_touch_point *touch = (UI_touch_point*)user_data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_X) {
			touch->x = evt->value;
		}
		if (evt->code == INPUT_ABS_Y) {
			touch->y = evt->value;
		}
	} else if (evt->type == INPUT_EV_KEY) {
		if (evt->code == INPUT_BTN_TOUCH) {
			if (touch->touch && !evt->value)
			{
				if (touch->x > 0 && touch->x < CONFIG_RESOLUTION_X && touch->y > 0 && touch->y < CONFIG_RESOLUTION_Y)
				{
					/* dont pick when in other UI zones */
					if (!(touch->x < 132 && touch->y > CONFIG_RESOLUTION_Y - 128) && !(touch->x > CONFIG_RESOLUTION_X - 256)) {
						update_selection_mouse();
					}
				}
			}
			touch->touch = evt->value ? true : false;
		}
	}
}

static void main_key(struct input_event *evt, void *user_data)
{
	//Main_keys *cont = user_data;

	if (evt->code == INPUT_KEY_RIGHTCTRL) {
		if (!evt->value)
			update_selection();
	}
	if (evt->type == INPUT_EV_KEY && selected_object!= NULL) {
		if (!evt->value)
			switch(evt->code) {
				case INPUT_KEY_J:
					selected_object->visual.transform.translation.x += L3_F;
					update_object_edit();
				break;
				case INPUT_KEY_L:
					selected_object->visual.transform.translation.x -= L3_F;
					update_object_edit();
				break;
				case INPUT_KEY_I:
					selected_object->visual.transform.translation.z += L3_F;
					update_object_edit();
				break;
				case INPUT_KEY_K:
					selected_object->visual.transform.translation.z -= L3_F;
					update_object_edit();
				break;
			}
	}

	if (evt->type == INPUT_EV_KEY && object_edit_data.focused != NULL) {
		if (!evt->value)
			switch(evt->code) {
				case INPUT_KEY_KP0:
				case INPUT_KEY_0:
					lv_textarea_add_char(object_edit_data.focused, '0');
				break;

				case INPUT_KEY_KP1:
				case INPUT_KEY_1:
					lv_textarea_add_char(object_edit_data.focused, '1');
				break;

				case INPUT_KEY_KP2:
				case INPUT_KEY_2:
					lv_textarea_add_char(object_edit_data.focused, '2');
				break;

				case INPUT_KEY_KP3:
				case INPUT_KEY_3:
					lv_textarea_add_char(object_edit_data.focused, '3');
				break;

				case INPUT_KEY_KP4:
				case INPUT_KEY_4:
					lv_textarea_add_char(object_edit_data.focused, '4');
				break;

				case INPUT_KEY_KP5:
				case INPUT_KEY_5:
					lv_textarea_add_char(object_edit_data.focused, '5');
				break;

				case INPUT_KEY_KP6:
				case INPUT_KEY_6:
					lv_textarea_add_char(object_edit_data.focused, '6');
				break;

				case INPUT_KEY_KP7:
				case INPUT_KEY_7:
					lv_textarea_add_char(object_edit_data.focused, '7');
				break;

				case INPUT_KEY_KP8:
				case INPUT_KEY_8:
					lv_textarea_add_char(object_edit_data.focused, '8');
				break;

				case INPUT_KEY_KP9:
				case INPUT_KEY_9:
					lv_textarea_add_char(object_edit_data.focused, '9');
				break;

				case INPUT_KEY_LEFT:
					lv_textarea_cursor_left(object_edit_data.focused);
				break;

				case INPUT_KEY_RIGHT:
					lv_textarea_cursor_right(object_edit_data.focused);
				break;

				case INPUT_KEY_KPMINUS:
					lv_textarea_add_char(object_edit_data.focused, '-');
				break;

				case INPUT_KEY_BACKSPACE:
					lv_textarea_delete_char(object_edit_data.focused);
				break;
			}
	}
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(input_sdl_touch)), main_touch, &main_touch_point);
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(evdev)), main_key, &main_keys);

static UI_touch_point collider_touch_point;

static void collider_touch(struct input_event *evt, void *user_data)
{
	UI_touch_point *touch = (UI_touch_point*)user_data;

	if (evt->type == INPUT_EV_ABS) {
		if (evt->code == INPUT_ABS_X) {
			touch->x = evt->value;
		}
		if (evt->code == INPUT_ABS_Y) {
			touch->y = evt->value;
		}
	} else if (evt->type == INPUT_EV_KEY) {
		if (evt->code == INPUT_BTN_TOUCH) {
			touch->touch = evt->value ? true : false;
		}
	}
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(input_sdl_touch2)), collider_touch, &collider_touch_point);

void draw_selector()
{
	L3_Vec4 out;
	L3_project3DPointToScreen(selected_object->visual.transform.translation, *engine_getcamera(), &out);
	L3_plot_line(0xFF, out.x, out.y, out.x +32 , out.y + 32);
	L3_plot_line(0xFF, out.x, out.y, out.x +32 , out.y - 32);
	L3_plot_line(0xFF, out.x, out.y, out.x -32 , out.y - 32);
	L3_plot_line(0xFF, out.x, out.y, out.x -32 , out.y + 32);
}

int engine_render_hook_post(void) {
	if (selected_object != NULL) {
		draw_selector();
	}
	return 0;
}

int do_editor_UI(void)
{
	lv_display_set_default(lvgl_display);

	if (selected_object != NULL)
	{
		lv_obj_clear_flag(object_edit, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(object_edit, LV_OBJ_FLAG_HIDDEN);
	}

	lv_img_set_src(background_render, &background_render_img);
	lv_timer_handler();

	return 0;
}
