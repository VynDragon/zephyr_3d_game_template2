/*
* APACHE 2
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/timing/timing.h>
#include <stdlib.h>
#include <math.h>

#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(engine);
#include "engine.h"
#include "filters.h"

/* ------------------------------------------------------------------------------------------- */

static Engine_Object		engine_objects[CONFIG_MAX_OBJECTS] = {0};
static uint32_t				engine_objects_count = 0;

static const Engine_Object	*static_engine_objects;
static uint32_t				static_engine_objects_count;
static bool					static_engine_objects_enabled = false;
static const Filter_f		*static_engine_filters;
size_t						static_engine_filters_count;

typedef struct E_Collider_pair_t {
	const E_Collider	*collider;
	const Engine_Object	*parent;
} E_Collider_pair;

static E_Collider_pair		engine_colliders[ENGINE_MAX_COLLIDERS];
static uint32_t				engine_colliders_count = 0;

static E_Particle			engine_particles[ENGINE_MAX_PARTICLES];

Engine_DObject				engine_dynamic_objects[ENGINE_MAX_DOBJECTS];
uint32_t					engine_dynamic_objects_count = 0;

K_MUTEX_DEFINE(engine_objects_lock);
K_MUTEX_DEFINE(engine_render_lock);
#ifdef CONFIG_BLIT_THREAD
K_SEM_DEFINE(engine_blit_cnt, 0, ENGINE_MAX_BLIT_QUEUED * 2);
#endif

static Engine_pf	engine_pf;
static Engine_Scene	*engine_current_scene = NULL;

uint32_t engine_drawnTriangles = 0;
float engine_rFPS = 0;

/* ------------------------------------------------------------------------------------------- */



#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_displays)
#define ENUMERATE_DISPLAY_DEVS(node_id, prop, idx) DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)),

const struct device *engine_display_devices[DT_ZEPHYR_DISPLAYS_COUNT] = {
	DT_FOREACH_PROP_ELEM(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_displays), displays, ENUMERATE_DISPLAY_DEVS)
};
#else
const struct device *engine_display_devices[] = {
	DEVICE_DT_GET(DT_CHOSEN(zephyr_display)),
};
#endif

/* ------------------------------------------------------------------------------------------- */

L3_PERFORMANCE_FUNCTION
inline int E_drawbillboard_particle(L3_Vec4 point, const E_Particle *particle, L3_Camera camera)
{
	if (!L3_zTest(point.x,point.y,point.z))
		return 0;

	int focal = camera.focalLength != 0 ? camera.focalLength : 1;
	float scale_x = ((float)(particle->transform.scale.x * focal) / (float)point.z) * ((float)particle->billboard->scale/0x4000) * L3_RESOLUTION_X / L3_F;
	float scale_y = ((float)(particle->transform.scale.y * focal) / (float)point.z) * ((float)particle->billboard->scale/0x4000) * L3_RESOLUTION_X / L3_F;
	int scaled_width = particle->billboard->width * scale_x;
	int scaled_height = particle->billboard->height * scale_y;

	int startx = point.x - scaled_width / 2;
	int endx =  MIN(point.x + scaled_width / 2, L3_RESOLUTION_X);
	int starty = point.y - scaled_height / 2;
	int endy =  MIN(point.y + scaled_height / 2, L3_RESOLUTION_Y);
	for (int i = MAX(0, startx); i < endx; i++) {
		int m = (float)(i - startx) / scale_x;
		for (int j = MAX(0, starty); j < endy; j++) {
			int n = (float)(j - starty) / scale_y;
			if (particle->billboard->texture[m + n * particle->billboard->width] > particle->billboard->transparency_threshold) {
				L3_video_buffer[j * L3_RESOLUTION_X + i] = (L3_video_buffer[j * L3_RESOLUTION_X + i] * (0xFF - particle->billboard->transparency) +  particle->billboard->texture[m + n * particle->billboard->width] * particle->billboard->transparency) / 0xFF;
			}
		}
	}
	return 0;
}

__attribute__((flatten))
L3_PERFORMANCE_FUNCTION
static void E_drawParticles(L3_Camera camera)
{
	L3_Mat4 matCamera;
	L3_Vec4 transformed;

	L3_makeCameraMatrix(camera.transform, matCamera);

	for (int i = 0; i < ENGINE_MAX_PARTICLES; i++) {
		if (engine_particles[i].life > 0) {
			transformed = engine_particles[i].transform.translation;
			transformed.w = L3_F;
			L3_vec3Xmat4(&transformed, matCamera);

			transformed.w = transformed.z;

			_L3_mapProjectedVertexToScreen(&transformed, camera.focalLength);

			if (transformed.x < 0 || transformed.x >= L3_RESOLUTION_X || transformed.y < 0 || transformed.y >= L3_RESOLUTION_Y || transformed.z <= L3_NEAR)
			{
				continue;
			}
			E_drawbillboard_particle(transformed, &(engine_particles[i]), camera);
		}
	}
}

__attribute__((weak)) int engine_render_hook_pre(void) {
	return 0;
}
__attribute__((weak)) int engine_render_hook_post(void) {
	return 0;
}

#ifdef CONFIG_BLIT_THREAD
static void blit_function(void *, void *, void *)
{
#if	CONFIG_LOG_PERFORMANCE
	timing_t start_time, end_time;
	uint32_t total_time_us;
#endif
	uint16_t offset = 0;
	uint16_t size = L3_RESOLUTION_Y;
	uint16_t size_filter_ui;
	uint16_t missed_seq = L3_RESOLUTION_Y;

	while (1) {

		k_sem_take(&engine_blit_cnt, K_FOREVER);

#if	CONFIG_LOG_PERFORMANCE
		start_time = timing_counter_get();
#endif
		if (k_sem_count_get(&engine_blit_cnt) * 4 < ENGINE_MAX_BLIT_QUEUED) {
			missed_seq = L3_RESOLUTION_Y;
		} else if (k_sem_count_get(&engine_blit_cnt) * 4 > ENGINE_MAX_BLIT_QUEUED) {
			missed_seq = L3_RESOLUTION_Y / 2;
		} else if (k_sem_count_get(&engine_blit_cnt) * 2 > ENGINE_MAX_BLIT_QUEUED) {
			missed_seq = L3_RESOLUTION_Y / 4;
		} else if (k_sem_count_get(&engine_blit_cnt) > ENGINE_MAX_BLIT_QUEUED) {
			missed_seq = L3_RESOLUTION_Y / 8;
		}

		offset += missed_seq;
		size = missed_seq;

		if (offset >= L3_RESOLUTION_Y) {
			offset = 0;
		}
		if (offset + size > L3_RESOLUTION_Y) {
			size = L3_RESOLUTION_Y - offset;
		}
		size_filter_ui = size;
		if (offset + size_filter_ui > L3_RESOLUTION_Y - 4) {
			size_filter_ui = L3_RESOLUTION_Y - offset - 4;
		}
		filter_apply_all(0, offset, L3_RESOLUTION_X, size_filter_ui, static_engine_filters, static_engine_filters_count, NULL);
		ENGINE_BLIT_FUNCTION(&(L3_video_buffer[offset * L3_RESOLUTION_X]), 0, offset, L3_RESOLUTION_X, size);
#if	CONFIG_LOG_PERFORMANCE
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		LOG_INF_RATELIMIT_RATE(1000, "blit us: %u ms:%u fps:%u", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
#endif
	}
}

#endif
static uint64_t render_delayed_count = 0;

static void render_function(void *, void *, void *)
{
#if	CONFIG_LOG_PERFORMANCE
	timing_t start_time;
	uint32_t total_time_us,;
#endif
	timing_t end_time, rstart_time;
	uint32_t render_time_us;
	k_timepoint_t timing = L3_FPS_TIMEPOINT(CONFIG_TARGET_RENDER_FPS);

	while (1) {
		if (render_delayed_count < CONFIG_TARGET_RENDER_FPS) {
			timing = L3_FPS_TIMEPOINT((uint64_t)CONFIG_TARGET_RENDER_FPS - render_delayed_count);
		} else {
			render_delayed_count = (uint64_t)CONFIG_TARGET_RENDER_FPS;
			timing = L3_FPS_TIMEPOINT((uint64_t)1);
		}
#if	CONFIG_LOG_PERFORMANCE
		start_time = timing_counter_get();
#endif
		/* clear viewport to black */
		L3_newFrame();
		if (engine_current_scene != NULL)
			if (engine_current_scene->clear_pix_func != NULL)
				L3_clearScreen_with(engine_current_scene->clear_pix_func);
			else
				L3_clearScreen(0);
		else
			L3_clearScreen(0);

		rstart_time = timing_counter_get();

		k_mutex_lock(&engine_render_lock, K_FOREVER);
		engine_render_hook_pre();
		engine_drawnTriangles = L3_draw(engine_camera, engine_global_objects, engine_objectCount);
		E_drawParticles(engine_camera);
		engine_render_UI();
		engine_render_hook_post();
		k_mutex_unlock(&engine_render_lock);
#ifdef CONFIG_BLIT_THREAD
		if (k_sem_count_get(&engine_blit_cnt) > ENGINE_MAX_BLIT_QUEUED) {
			render_delayed_count+=2;
			//LOG_ERR_RATELIMIT_RATE(2000, "Blit cant keep up!");
		}
		k_sem_give(&engine_blit_cnt);
#else
		ENGINE_BLIT_FUNCTION(L3_video_buffer, 0, 0, L3_RESOLUTION_X, L3_RESOLUTION_Y);
#endif

		end_time = timing_counter_get();
		render_time_us = timing_cycles_to_ns(timing_cycles_get(&rstart_time, &end_time)) / 1000;
		engine_rFPS = 1000000.0 / (render_time_us != 0 ? render_time_us : 1.0);

#if	CONFIG_LOG_PERFORMANCE
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		LOG_INF_RATELIMIT_RATE(1000, "total render us: %u ms:%u fps:%u", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
		LOG_INF_RATELIMIT_RATE(1000, "exclusively render us:%u render fps: %u", render_time_us, engine_rFPS);
		LOG_INF_RATELIMIT_RATE(1000, "rendered %u Polygons, %u polygons per second", engine_drawnTriangles, engine_drawnTriangles * engine_rFPS);
#endif

		if (sys_timepoint_expired(timing)) {
			render_delayed_count+=2;
		}
		else if (render_delayed_count > 0)
		{
			render_delayed_count--;
		}
		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_USEC(1));
		}
		/* force unready thread to avoid monopolization of CPU time */
		k_yield();
	}
}

Engine_Object *engine_add_object(Engine_Object object)
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

int engine_remove_object(Engine_Object *object)
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

size_t engine_statics_getcnt(void)
{
	return static_engine_objects_count;
}

Engine_Object *engine_getobjects(void)
{
	return engine_objects;
}

L3_Camera *engine_getcamera(void)
{
	return &(engine_camera);
}

E_Particle *engine_create_particle(L3_Transform3D transform, Engine_Particle_pf process, const L3_Billboard *billboard, uint32_t lifespan)
{
	int i = 0;
	for (; i < ENGINE_MAX_PARTICLES; i++) {
		if (engine_particles[i].life <= 0) {
			break;
		}
	}
	if (i >= ENGINE_MAX_PARTICLES) {
		return 0;
	}

	engine_particles[i].process = process;
	engine_particles[i].billboard = billboard;
	engine_particles[i].transform = transform;
	engine_particles[i].life = lifespan;
	return &(engine_particles[i]);
}

static void build_render_list(void)
{
	const L3_Object **render_o = engine_global_objects;
	size_t o_cnt = 0;
	L3_Vec4 forward = {0, 0, L3_F, L3_F};
	L3_Mat4 transMat;
	L3_Vec4 dir;
	L3_Unit dot;

	L3_makeRotationMatrixZXY(engine_camera.transform.rotation.x,
							engine_camera.transform.rotation.y,
							engine_camera.transform.rotation.z,
							transMat);

	L3_vec3Xmat4(&forward, transMat);

	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type >= ENGINE_VISUAL_MODEL) {
			if (o_cnt >= L3_MAX_OBJECTS) break;
			if (engine_objects[i].view_range <= L3_distanceManhattan(engine_objects[i].visual.transform.translation, engine_camera.transform.translation)) continue;
			dir.x = engine_objects[i].visual.transform.translation.x - engine_camera.transform.translation.x;
			dir.y = engine_objects[i].visual.transform.translation.y - engine_camera.transform.translation.y;
			dir.z = engine_objects[i].visual.transform.translation.z - engine_camera.transform.translation.z;
			dot = L3_vec3Dot(forward, dir);
			if (dot < -ENGINE_REAR_OBJECT_CUTOFF) continue;
			*render_o = &(engine_objects[i].visual);
			render_o++;
			o_cnt++;
		}
	}

	if (static_engine_objects_enabled) {
		for (int i = 0; i < static_engine_objects_count; i++) {
			if (static_engine_objects[i].visual_type >= ENGINE_VISUAL_MODEL) {
				if (o_cnt >= L3_MAX_OBJECTS) break;
				if (static_engine_objects[i].view_range <= L3_distanceManhattan(static_engine_objects[i].visual.transform.translation, engine_camera.transform.translation)) continue;
				dir.x = static_engine_objects[i].visual.transform.translation.x - engine_camera.transform.translation.x;
				dir.y = static_engine_objects[i].visual.transform.translation.y - engine_camera.transform.translation.y;
				dir.z = static_engine_objects[i].visual.transform.translation.z - engine_camera.transform.translation.z;
				dot = L3_vec3Dot(forward, dir);
				if (dot < -ENGINE_REAR_OBJECT_CUTOFF) continue;
				*render_o = &(static_engine_objects[i].visual);
				render_o++;
				o_cnt++;
			}
		}
	}
#if	CONFIG_LOG_PERFORMANCE
	//LOG_INF("selected %d objects", o_cnt);
#endif
	engine_objectCount = o_cnt;
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

	/*
	if (static_engine_objects_enabled) {
		for (int i = 0; i < static_engine_objects_count; i++) {
			if (static_engine_objects[i].visual_type >= ENGINE_VISUAL_MODEL) {
				if (static_engine_objects[i].process != NULL) {
					static_engine_objects[i].process(&(static_engine_objects[i]), static_engine_objects[i].data);
				}
			}
		}
	}
	*/
}

static void run_particles(void)
{
	for (int i = 0; i < ENGINE_MAX_PARTICLES; i++) {
		if (engine_particles[i].life > 0) {
			engine_particles[i].process(&(engine_particles[i]));
			engine_particles[i].life -= 1;
		}
	}
}

static void build_collider_list(void)
{
	E_Collider_pair *colliders = engine_colliders;
	engine_colliders_count = 0;

	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].collisions != 0) {
			if (engine_colliders_count >= ENGINE_MAX_COLLIDERS) break;
			for (int j = 0; j < engine_objects[i].collisions->colliderCount; j++) {
				colliders->collider = &(engine_objects[i].collisions->colliders[j]);
				colliders->parent = &(engine_objects[i]);
				colliders++;
				engine_colliders_count++;
			}
		}
	}

	if (static_engine_objects_enabled) {
		for (int i = 0; i < static_engine_objects_count; i++) {
			if (static_engine_objects[i].collisions != 0) {
				if (engine_colliders_count >= ENGINE_MAX_COLLIDERS) break;
				if (static_engine_objects[i].view_range > L3_distanceManhattan(static_engine_objects[i].visual.transform.translation, engine_camera.transform.translation)) {
					for (int j = 0; j < static_engine_objects[i].collisions->colliderCount; j++) {
						if (engine_colliders_count < ENGINE_MAX_COLLIDERS) {
							colliders->collider = &(static_engine_objects[i].collisions->colliders[j]);
							colliders->parent = &(static_engine_objects[i]);
							colliders++;
							engine_colliders_count++;
						}
					}
				}
			}
		}
	}
#if	CONFIG_LOG_PERFORMANCE
	//LOG_INF("selected %d colliders", engine_colliders_count);
#endif
}

static void do_collision_cube(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 up = {0, L3_F, 0, L3_F};
	L3_Vec4 right = {L3_F, 0, 0, L3_F};
	L3_Vec4 far = {0, 0, L3_F, L3_F};
	L3_Mat4 transMat;
	L3_Vec4 muln;
	L3_Vec4 plane_pos;

	plane_pos.x = collider_object->visual.transform.translation.x + collider->cube.offset.x;
	plane_pos.y = collider_object->visual.transform.translation.y + collider->cube.offset.y;
	plane_pos.z = collider_object->visual.transform.translation.z + collider->cube.offset.z;

	muln.x = point.x - plane_pos.x;
	muln.y = point.y - plane_pos.y;
	muln.z = point.z - plane_pos.z;

	L3_makeRotationMatrixZXY(collider_object->visual.transform.rotation.x,
							collider_object->visual.transform.rotation.y,
							collider_object->visual.transform.rotation.z,
							transMat);

	L3_vec3Xmat4(&up, transMat);
	L3_vec3Xmat4(&right, transMat);
	L3_vec3Xmat4(&far, transMat);

	L3_Unit dotx = L3_vec3Dot(right, muln);
	L3_Unit x = collider->cube.size.x * 0.5 * collider_object->visual.transform.scale.x / L3_F;
	L3_Unit y = collider->cube.size.y * 0.5 * collider_object->visual.transform.scale.y / L3_F;
	L3_Unit z = collider->cube.size.z * 0.5 * collider_object->visual.transform.scale.z / L3_F;
	if (dotx > x*1.1 || dotx < -x*1.1)
		return;
	L3_Unit doty = L3_vec3Dot(up, muln);
	if (doty > y*1.1 || doty < -y*1.1)
		return;
	L3_Unit dotz = L3_vec3Dot(far, muln);
	if (dotz > z*1.1 || dotz < -z*1.1)
		return;
	if (abs(dotz) < z && abs(doty) < y && abs(dotx) < x) {
		object->physics.transform->translation.x = object->physics.last_transform.translation.x;
		object->physics.transform->translation.y = object->physics.last_transform.translation.y;
		object->physics.transform->translation.z = object->physics.last_transform.translation.z;
	}

	L3_vec3Normalize(&up);

	L3_Unit dotv = L3_vec3Dot(up, object->physics.speeds.translation);
	object->physics.speeds.translation.x = (- 2 * dotv * up.x / L3_F + object->physics.speeds.translation.x) * collider->cube.bouncyness / L3_F;
	object->physics.speeds.translation.y = (- 2 * dotv * up.y / L3_F + object->physics.speeds.translation.y) * collider->cube.bouncyness / L3_F;
	object->physics.speeds.translation.z = (- 2 * dotv * up.z / L3_F + object->physics.speeds.translation.z) * collider->cube.bouncyness / L3_F;
}

static void do_collision_sphere(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 pos, up;

	pos.x = collider_object->visual.transform.translation.x + collider->sphere.offset.x;
	pos.y = collider_object->visual.transform.translation.y + collider->sphere.offset.y;
	pos.z = collider_object->visual.transform.translation.z + collider->sphere.offset.z;

	L3_Unit x = point.x - pos.x;
	L3_Unit y = point.y - pos.y;
	L3_Unit z = point.z - pos.z;
	L3_Unit d = abs(x) + abs(y) + abs(z);
	if (d*1.1 > collider_object->visual.transform.scale.x * collider->sphere.size / L3_F)
		return;
	if (d < collider_object->visual.transform.scale.x * collider->sphere.size / L3_F) {
		object->physics.transform->translation.x = object->physics.last_transform.translation.x;
		object->physics.transform->translation.y = object->physics.last_transform.translation.y;
		object->physics.transform->translation.z = object->physics.last_transform.translation.z;
	}


	up.x = point.x - pos.x;
	up.y = point.y - pos.z;
	up.z = point.z - pos.z;
	L3_vec3Normalize(&up);

	L3_Unit dotv = L3_vec3Dot(up, object->physics.speeds.translation);
	object->physics.speeds.translation.x = (2 * dotv * up.x / L3_F + object->physics.speeds.translation.x) * collider->sphere.bouncyness / L3_F;
	object->physics.speeds.translation.y = (2 * dotv * up.y / L3_F + object->physics.speeds.translation.y) * collider->sphere.bouncyness / L3_F;
	object->physics.speeds.translation.z = (2 * dotv * up.z / L3_F + object->physics.speeds.translation.z) * collider->sphere.bouncyness / L3_F;
}

static void do_collision_capsule(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 pos, up;
	L3_Vec4 r_size;
	L3_Mat4 transMat;

	pos.x = collider_object->visual.transform.translation.x + collider->capsule.offset.x;
	pos.y = collider_object->visual.transform.translation.y + collider->capsule.offset.y;
	pos.z = collider_object->visual.transform.translation.z + collider->capsule.offset.z;

	L3_makeRotationMatrixZXY(collider_object->visual.transform.rotation.x,
						collider_object->visual.transform.rotation.y,
						collider_object->visual.transform.rotation.z,
						transMat);

	r_size = collider->capsule.size;
	r_size.x *= collider_object->visual.transform.scale.x / L3_F;
	r_size.y *= collider_object->visual.transform.scale.y / L3_F;
	r_size.z *= collider_object->visual.transform.scale.z / L3_F;
	L3_vec3Xmat4(&r_size, transMat);

	L3_Unit x = point.x - pos.x;
	L3_Unit y = point.y - pos.y;
	L3_Unit z = point.z - pos.z;
	if (abs(x)*1.1 > abs(r_size.x/2))
		return;
	else if (abs(y)*1.1 > abs(r_size.y/2))
		return;
	else if (abs(z)*1.1 > abs(r_size.z/2))
		return;
	else {
		object->physics.transform->translation.x = object->physics.last_transform.translation.x;
		object->physics.transform->translation.y = object->physics.last_transform.translation.y;
		object->physics.transform->translation.z = object->physics.last_transform.translation.z;
	}


	up.x = point.x - pos.x;
	up.y = point.y - pos.z;
	up.z = point.z - pos.z;
	L3_vec3Normalize(&up);

	L3_Unit dotv = L3_vec3Dot(up, object->physics.speeds.translation);
	object->physics.speeds.translation.x = (-2 * dotv * up.x / L3_F + object->physics.speeds.translation.x) * collider->capsule.bouncyness / L3_F;
	object->physics.speeds.translation.y = (-2 * dotv * up.y / L3_F + object->physics.speeds.translation.y) * collider->capsule.bouncyness / L3_F;
	object->physics.speeds.translation.z = (-2 * dotv * up.z / L3_F + object->physics.speeds.translation.z) * collider->capsule.bouncyness / L3_F;
}

static void do_collision_axisplane_y(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider_object->visual.transform.translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider_object->visual.transform.translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider_object->visual.transform.translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.x * collider_object->visual.transform.scale.x / L3_F;
	L3_Unit z = collider->axisplane.size.z * collider_object->visual.transform.scale.z / L3_F;
	if (point.x > plane_pos.x + x / 2 || point.x < plane_pos.x - x / 2)
		return;
	if (point.z > plane_pos.z + z / 2 || point.z < plane_pos.z - z / 2)
		return;
	if (collider->axisplane.size.y > 0) {
		if (point.y > plane_pos.y) {
			return;
		}
		if (object->physics.last_transform.translation.y < plane_pos.y && collider->axisplane.traverseable) {
			return;
		}
	} else {
		if (point.y < plane_pos.y) {
			return;
		}
		if (object->physics.last_transform.translation.y > plane_pos.y && collider->axisplane.traverseable) {
			return;
		}
	}
	object->physics.transform->translation.y = plane_pos.y;
	object->physics.speeds.translation.y = -object->physics.speeds.translation.y * collider->axisplane.bouncyness / L3_F;
}

static void do_collision_axisplane_z(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider_object->visual.transform.translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider_object->visual.transform.translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider_object->visual.transform.translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.x * collider_object->visual.transform.scale.x / L3_F;
	L3_Unit z = collider->axisplane.size.y * collider_object->visual.transform.scale.y / L3_F;
	if (point.x > plane_pos.x + x / 2 || point.x < plane_pos.x - x / 2)
		return;
	if (point.y > plane_pos.y + z / 2 || point.y < plane_pos.y - z / 2)
		return;
	if (collider->axisplane.size.z > 0) {
		if (point.z > plane_pos.z) {
			return;
		}
		if (object->physics.last_transform.translation.z < plane_pos.z && collider->axisplane.traverseable) {
			return;
		}
	} else {
		if (point.z < plane_pos.z) {
			return;
		}
		if (object->physics.last_transform.translation.z > plane_pos.z && collider->axisplane.traverseable) {
			return;
		}
	}
	object->physics.transform->translation.z = plane_pos.z;
	object->physics.speeds.translation.z = -object->physics.speeds.translation.z * collider->axisplane.bouncyness / L3_F;
}

static void do_collision_axisplane_x(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider_object->visual.transform.translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider_object->visual.transform.translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider_object->visual.transform.translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.y * collider_object->visual.transform.scale.y / L3_F;
	L3_Unit z = collider->axisplane.size.z * collider_object->visual.transform.scale.z / L3_F;
	if (point.y > plane_pos.y + x / 2 || point.y < plane_pos.y - x / 2)
		return;
	if (point.z > plane_pos.z + z / 2 || point.z < plane_pos.z - z / 2)
		return;
	if (collider->axisplane.size.x > 0) {
		if (point.x > plane_pos.x) {
			return;
		}
		if (object->physics.last_transform.translation.x < plane_pos.x && collider->axisplane.traverseable) {
			return;
		}
	} else {
		if (point.x < plane_pos.x) {
			return;
		}
		if (object->physics.last_transform.translation.x > plane_pos.x && collider->axisplane.traverseable) {
			return;
		}
	}
	object->physics.transform->translation.x = plane_pos.x;
	object->physics.speeds.translation.x = -object->physics.speeds.translation.x * collider->axisplane.bouncyness / L3_F;
}

/* HOW MANY POINTS IN A TRIANGLE BRO? */
#define INTRP_NB 3

static size_t get_furthest(L3_Unit *distances)
{
	int furthest_saved = 0;
	size_t furthest_id;
	for (int j = 0; j < INTRP_NB; j++) {
		if (distances[j] > furthest_saved)
		{
			furthest_saved = distances[j];
			furthest_id = j;
		}
	}
	return furthest_id;
}

static L3_Vec4 to_barycentric_2d(L3_Vec4 *t_points, L3_Vec4 point)
{
	L3_Vec4 v0, v1, v2, out;
	v0.x = t_points[1].x - t_points[0].x;
	v0.z = t_points[1].z - t_points[0].z;
	v1.x = t_points[2].x - t_points[0].x;
	v1.z = t_points[2].z - t_points[0].z;
	v2.x = point.x - t_points[0].x;
	v2.z = point.z - t_points[0].z;
	L3_Unit d00 = L3_vec2Dot_xz(v0, v0);
	L3_Unit d01 = L3_vec2Dot_xz(v0, v1);
	L3_Unit d11 = L3_vec2Dot_xz(v1, v1);
	L3_Unit d20 = L3_vec2Dot_xz(v2, v0);
	L3_Unit d21 = L3_vec2Dot_xz(v2, v1);
	float denom = (float)d00 * (float)d11 - (float)d01 * (float)d01;

	float v = ((float)d11 * (float)d20 - (float)d01 * (float)d21) / denom;
	float w = ((float)d00 * (float)d21 - (float)d01 * (float)d20) / denom;
	float u = 1.0f - v - w;
	out.x = u * L3_F;
	out.y = v * L3_F;
	out.z = w * L3_F;
	return out;

}

static void do_collision_terrain(const Engine_Object *collider_object, Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 pos = collider_object->visual.transform.translation;
	L3_Vec4 points[3] = {0};
	L3_Unit distance[3];
	size_t last_furthest = 0;
	L3_Unit final_y = 0;
	L3_Unit furthest;

	distance[0] = 0xFFFFFF;
	distance[1] = 0xFFFFFF;
	distance[2] = 0xFFFFFF;
	//distance[3] = 0xFFFFFF;

	for (int i = 0; i < collider->terrain.points_cnt; i++) {
		L3_Vec4 t_point = collider->terrain.points[i];
		t_point.x *= collider_object->visual.transform.scale.x / L3_F;
		t_point.y *= collider_object->visual.transform.scale.y / L3_F;
		t_point.z *= collider_object->visual.transform.scale.z / L3_F;
		t_point.x += pos.x;
		t_point.y += pos.y;
		t_point.z += pos.z;
		//L3_Unit cur_distance = L3_distanceManhattan(point_noy, t_point);
		L3_Unit cur_distance = sqrt((t_point.x - point.x) * (t_point.x - point.x)
								+ (t_point.z - point.z) * (t_point.z - point.z));

		if (cur_distance < distance[last_furthest])
		{
			distance[last_furthest] = cur_distance;
			points[last_furthest] = t_point;
			last_furthest = get_furthest(distance);
		}
	}

	L3_Vec4 barycentric = to_barycentric_2d(points, point);

	final_y += (float)points[0].y * (float)barycentric.x / (float)L3_F;
	final_y += (float)points[1].y * (float)barycentric.y / (float)L3_F;
	final_y += (float)points[2].y * (float)barycentric.z / (float)L3_F;

	if (point.y > final_y) {
		return;
	}
	if (object->physics.last_transform.translation.y < final_y && collider->axisplane.traverseable) {
		return;
	}
	object->physics.transform->translation.y = final_y;
	object->physics.speeds.translation.y = -object->physics.speeds.translation.y * collider->axisplane.bouncyness / L3_F;
}

static void do_collision(Engine_DObject *object, const E_Collider_pair *collider, L3_Vec4 point)
{
	switch (collider->collider->type)
	{
		case ENGINE_COLLIDER_CUBE:
			do_collision_cube(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_SPHERE:
			do_collision_sphere(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_APLANEY:
			do_collision_axisplane_y(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_APLANEX:
			do_collision_axisplane_x(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_APLANEZ:
			do_collision_axisplane_z(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_CAPSULE:
			do_collision_capsule(collider->parent, object, collider->collider, point);
		break;
		case ENGINE_COLLIDER_TERRAIN:
			do_collision_terrain(collider->parent, object, collider->collider, point);
		break;
	}
}


static void do_DObjects_speeds(Engine_DObject *object)
{
	object->physics.transform->translation.x += object->physics.speeds.translation.x;
	object->physics.transform->translation.y += object->physics.speeds.translation.y;
	object->physics.transform->translation.z += object->physics.speeds.translation.z;
	object->physics.transform->rotation.x += object->physics.speeds.rotation.x;
	object->physics.transform->rotation.y += object->physics.speeds.rotation.y;
	object->physics.transform->rotation.z += object->physics.speeds.rotation.z;
}

static void run_all_DObjects(void)
{
	L3_Vec4 point_transformed;
	L3_Mat4 transMat;

	for (int i = 0; i < engine_dynamic_objects_count; i++) {
		do_DObjects_speeds(&(engine_dynamic_objects[i]));
		if (engine_dynamic_objects[i].physics.pointOffsetsCount == 0) {
			for (int j = 0; j < engine_colliders_count; j++) {
				do_collision(&(engine_dynamic_objects[i]), &(engine_colliders[j]), engine_dynamic_objects[i].physics.transform->translation);
			}
		} else {
			L3_makeWorldMatrix(*engine_dynamic_objects[i].physics.transform, transMat);
			for (int k = 0; k < engine_dynamic_objects[i].physics.pointOffsetsCount; k++) {
				point_transformed = engine_dynamic_objects[i].physics.pointOffsets[k];
				L3_vec3Xmat4(&point_transformed, transMat);
				for (int j = 0; j < engine_colliders_count; j++) {
					do_collision(&(engine_dynamic_objects[i]), &(engine_colliders[j]), point_transformed);
				}
			}
		}
		engine_dynamic_objects[i].physics.last_transform = *engine_dynamic_objects[i].physics.transform;
	}
}

int engine_remove_all_objects_past(int cnt)
{
	engine_objects_count = cnt;
	return 0;
}

Engine_Scene *engine_getscene(void)
{
	return engine_current_scene;
}

int	engine_cleanscene(void)
{
	engine_current_scene = NULL;

	engine_camera.transform.translation.x = 0;
	engine_camera.transform.translation.y = 0;
	engine_camera.transform.translation.z = 0;
	engine_camera.transform.translation.w = L3_F;
	engine_camera.transform.rotation.x = 0;
	engine_camera.transform.rotation.y = 0;
	engine_camera.transform.rotation.z = 0;
	engine_camera.transform.rotation.w = L3_F;
	engine_camera.transform.scale.x = L3_F;
	engine_camera.transform.scale.y = L3_F;
	engine_camera.transform.scale.z = L3_F;
	engine_camera.transform.scale.w = 0;
	engine_camera.focalLength = 256;

	for (int i = 0; i < CONFIG_MAX_OBJECTS; i++)
	{
		engine_objects[i].visual_type = ENGINE_VISUAL_UNUSED;
	}
	engine_objects_count = 0;
	static_engine_objects = NULL;
	static_engine_objects_count = 0;
	static_engine_objects_enabled = false;
	static_engine_filters = NULL;
	static_engine_filters_count = 0;
	engine_dynamic_objects_count = 0;
	for (int i = 0; i < ENGINE_MAX_PARTICLES; i++) {
		engine_particles[i].life  = 0;
	}
	return 0;
}

int	engine_initscene(Engine_Scene *scene)
{
	if (scene->statics_count > 0)
	{
		static_engine_objects = scene->statics;
		static_engine_objects_count = scene->statics_count;
		static_engine_objects_enabled = true;
	}
	if (scene->filters_count > 0) {
		static_engine_filters_count = scene->filters_count;
		static_engine_filters = scene->filters;
	}
	(*scene->inf)(scene->data);
	engine_current_scene = scene;
	return 0;
}

int	engine_switchscene(Engine_Scene *scene)
{
	engine_cleanscene();
	engine_initscene(scene);
	return 0;
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
		if (engine_current_scene != NULL)
			if (engine_current_scene->pf != NULL)
				(*engine_current_scene->pf)(engine_current_scene);
		run_all_object_process();
		run_particles();
		build_collider_list();
		run_all_DObjects();

		k_mutex_unlock(&engine_objects_lock);

#if	CONFIG_LOG_PERFORMANCE
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		LOG_INF_RATELIMIT_RATE(1000, "total process us: %u ms:%u fps:%u", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
#endif

		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_USEC(1));
			k_yield();
		}
		/* force unready thread to avoid monopolization of CPU time */
		k_sleep(K_NSEC(10));
		k_yield();
	}
}


void engine_set_statics(const Engine_Object *objects, uint32_t count)
{
	static_engine_objects = objects;
	static_engine_objects_count = count;
}

void engine_statics_enabled(bool yes)
{
	static_engine_objects_enabled = yes;
}

#ifdef CONFIG_BLIT_THREAD
static struct k_thread blit_thread;
K_THREAD_STACK_DEFINE(blit_thread_stack, CONFIG_BLIT_THREAD_STACK);
#endif

static struct k_thread render_thread;
K_THREAD_STACK_DEFINE(render_thread_stack, CONFIG_RENDER_THREAD_STACK);

static struct k_thread process_thread;
K_THREAD_STACK_DEFINE(process_thread_stack, CONFIG_PROCESS_THREAD_STACK);

int init_engine(Engine_pf pf)
{
	int ret;

	engine_pf = pf;
	engine_cleanscene();

	ret = init_engine_UI();
	if (ret < 0) {
		LOG_ERR("Couldnt init UI");
		return ret;
	}

	k_thread_create(&render_thread, render_thread_stack, CONFIG_RENDER_THREAD_STACK,
					render_function, NULL, NULL, NULL,
					15, 0, K_NO_WAIT);

#ifdef CONFIG_BLIT_THREAD
	k_thread_create(&blit_thread, blit_thread_stack, CONFIG_BLIT_THREAD_STACK,
					blit_function, NULL, NULL, NULL,
					50, 0, K_NO_WAIT);
#endif

	k_thread_create(&process_thread, process_thread_stack, CONFIG_PROCESS_THREAD_STACK,
					process_function, NULL, NULL, NULL,
					5, 0, K_NO_WAIT);

	return 0;
}
