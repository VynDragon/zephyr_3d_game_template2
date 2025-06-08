

#include "engine.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(engine);

/* ------------------------------------------------------------------------------------------- */

static Engine_object engine_objects[CONFIG_MAX_OBJECTS] = {0};
static uint32_t		engine_objects_count = 0;
K_MUTEX_DEFINE(engine_objects_lock);
K_MUTEX_DEFINE(engine_render_lock);
static Engine_pf engine_pf;

/* ------------------------------------------------------------------------------------------- */

static void render_function(void *, void *, void *)
{
#if	CONFIG_LOG_PERFORMANCE
	timing_t start_time, end_time, dstart_time, rend_time, rstart_time;
	uint32_t total_time_us, render_time_us, draw_time_us;
#endif
	k_timepoint_t timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_RENDER_FPS);

	while (1) {
		timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_RENDER_FPS);
#if	CONFIG_LOG_PERFORMANCE
		start_time = timing_counter_get();
#endif
		/* clear viewport to black */
		L3_newFrame();
		L3_clearScreen(0);
#if	CONFIG_LOG_PERFORMANCE
		rstart_time = timing_counter_get();
#endif

		k_mutex_lock(&engine_render_lock, K_FOREVER);
		uint32_t drawnTriangles = L3_drawScene(L3_SCENE);
		k_mutex_unlock(&engine_render_lock);

#if	CONFIG_LOG_PERFORMANCE
		rend_time = timing_counter_get();
		dstart_time = timing_counter_get();
#endif
		ENGINE_BLIT_FUNCTION(L3_video_buffer, L3_RESOLUTION_X, L3_RESOLUTION_Y);
#if	CONFIG_LOG_PERFORMANCE
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		render_time_us = timing_cycles_to_ns(timing_cycles_get(&rstart_time, &rend_time)) / 1000;
		draw_time_us = timing_cycles_to_ns(timing_cycles_get(&dstart_time, &end_time)) / 1000;
		printf("total us: %u ms:%u fps:%u\n", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
		printf("display us:%u render us:%u render fps: %u\n", draw_time_us, render_time_us, 1000000 / (render_time_us != 0 ? render_time_us : 1));
		printf("rendered %u Polygons, %u polygons per second\n", drawnTriangles, drawnTriangles * 1000000 / (render_time_us != 0 ? render_time_us : 1));
#endif
		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
		}
		k_yield();
	}
}

Engine_object *engine_add_object(Engine_object object)
{
	if (object.visual_type == ENGINE_VISUAL_UNUSED)
		return 0;

	int i =0;
	for (; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type == ENGINE_VISUAL_UNUSED) {
			break;
		}
	}
	if (i >= engine_objects_count && i >= CONFIG_MAX_OBJECTS) {
		return 0;
	}
	if (i >= engine_objects_count) {
		engine_objects_count++;
	}
	engine_objects[i] = object;
	return &(engine_objects[i]);
}

int engine_remove_object(Engine_object *object)
{
	object->visual_type = ENGINE_VISUAL_UNUSED;
	return 0;
}

int engine_optimize_object_table(void)
{
	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type == ENGINE_VISUAL_UNUSED) {
			engine_objects[i] = engine_objects[engine_objects_count - 1];
			engine_objects_count--;
		}
	}
	return 0;
}

size_t engine_object_getcnt(void)
{
	return engine_objects_count;
}

L3_Camera *engine_getcamera(void)
{
	return &(L3_SCENE.camera);
}

static void build_render_list(void)
{
	L3_Model3D *render_m = L3_MODELS;
	int m_cnt = 0;
	L3_Billboard *render_b = L3_BILLBOARDS;
	int b_cnt = 0;
	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type == ENGINE_VISUAL_MODEL) {
			if (m_cnt < L3_MAX_MODELS) {
				if (engine_objects[i].view_range > L3_distanceManhattan(engine_objects[i].visual.model.transform.translation, L3_SCENE.camera.transform.translation)) {
					*render_m = engine_objects[i].visual.model;
					render_m++;
					m_cnt++;
				}
			}
		} else if (engine_objects[i].visual_type == ENGINE_VISUAL_BILLBOARD) {
			if (b_cnt < L3_MAX_BILLBOARDS) {
				if (engine_objects[i].view_range > L3_distanceManhattan(engine_objects[i].visual.billboard.transform.translation, L3_SCENE.camera.transform.translation)) {
					*render_b = engine_objects[i].visual.billboard;
					render_b++;
					b_cnt++;
				}
			}
		}
	}
	L3_SCENE.modelCount = m_cnt;
	L3_SCENE.billboardCount = b_cnt;
}

static void run_all_object_process(void)
{
	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type >= ENGINE_VISUAL_MODEL) {
			if (engine_objects[i].process != NULL) {
				engine_objects[i].process(&(engine_objects[i]), engine_objects[i].data);
			}
		}
	}
}

static void process_function(void *, void *, void *)
{
	k_timepoint_t timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_PROCESS_FPS);

	while (1) {
#if	CONFIG_LOG_PERFORMANCE
	timing_t start_time, end_time;
	uint32_t total_time_us;
#endif
		timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_PROCESS_FPS);
#if	CONFIG_LOG_PERFORMANCE
		start_time = timing_counter_get();
#endif
		k_mutex_lock(&engine_objects_lock, K_FOREVER);
		k_mutex_lock(&engine_render_lock, K_FOREVER);
		build_render_list();
		k_mutex_unlock(&engine_render_lock);
		engine_pf();
		run_all_object_process();
		k_mutex_unlock(&engine_objects_lock);

#if	CONFIG_LOG_PERFORMANCE
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		printf("total process us: %u ms:%u fps:%u\n", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
#endif

		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
		}
		k_yield();
	}
}

static struct k_thread render_thread;
K_THREAD_STACK_DEFINE(render_thread_stack, CONFIG_RENDER_THREAD_STACK);

static struct k_thread process_thread;
K_THREAD_STACK_DEFINE(process_thread_stack, CONFIG_PROCESS_THREAD_STACK);

int init_engine(Engine_pf pf)
{
	L3_sceneInit(L3_MODELS, 0, L3_BILLBOARDS, 0, &L3_SCENE);
	L3_SCENE.camera.transform.translation.y = 0 * L3_F;
	L3_SCENE.camera.transform.translation.z = 0 * L3_F;
	//L3_SCENE.camera.focalLength = 0;

	engine_pf = pf;


	k_thread_create(&render_thread, render_thread_stack, CONFIG_RENDER_THREAD_STACK,
                render_function, NULL, NULL, NULL,
                6, 0, K_NO_WAIT);

	k_thread_create(&process_thread, process_thread_stack, CONFIG_PROCESS_THREAD_STACK,
                process_function, NULL, NULL, NULL,
                5, 0, K_NO_WAIT);

	return 0;
}
