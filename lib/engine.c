/*
* APACHE 2
*/


#include "engine.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(engine);

/* ------------------------------------------------------------------------------------------- */

static Engine_Object engine_objects[CONFIG_MAX_OBJECTS] = {0};
static uint32_t		engine_objects_count = 0;
static const E_Collider	*engine_colliders[ENGINE_MAX_COLLIDERS];
static uint32_t		engine_colliders_count = 0;
Engine_DObject	engine_dynamic_objects[ENGINE_MAX_DOBJECTS];
uint32_t		engine_dynamic_objects_count = 0;
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
			k_yield();
		}
		/* force unready thread to avoid monopolization of CPU time */
		k_sleep(K_NSEC(10));
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

L3_Camera *engine_getcamera(void)
{
	return &(L3_SCENE.camera);
}

static void build_render_list(void)
{
	const L3_Object **render_o = L3_OBJECTS;
	int o_cnt = 0;
	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].visual_type >= ENGINE_VISUAL_MODEL) {
			if (o_cnt < L3_MAX_OBJECTS) {
				if (engine_objects[i].view_range > L3_distanceManhattan(engine_objects[i].visual.transform.translation, L3_SCENE.camera.transform.translation)) {
					*render_o = &(engine_objects[i].visual);
					render_o++;
					o_cnt++;
				}
			}
		}
	}
#if	CONFIG_LOG_PERFORMANCE
	printf("selected %d objects\n", o_cnt);
#endif
	L3_SCENE.objectCount = o_cnt;
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

static void build_collider_list(void)
{
	const E_Collider **colliders = engine_colliders;
	engine_colliders_count = 0;
	for (int i = 0; i < engine_objects_count; i++) {
		if (engine_objects[i].collisions != 0) {
			if (engine_colliders_count < ENGINE_MAX_COLLIDERS) {
				for (int j = 0; j < engine_objects[i].collisions->colliderCount; j++) {
					*colliders = &(engine_objects[i].collisions->colliders[j]);
					colliders++;
					engine_colliders_count++;
				}
			}
		}
	}
#if	CONFIG_LOG_PERFORMANCE
	printf("selected %d colliders\n", engine_colliders_count);
#endif
}

static void do_collision_cube(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 up = {0, L3_F, 0, L3_F};
	L3_Vec4 right = {L3_F, 0, 0, L3_F};
	L3_Vec4 far = {0, 0, L3_F, L3_F};
	L3_Mat4 transMat;
	L3_Vec4 muln, mulo;
	L3_Vec4 plane_pos;

	plane_pos.x = collider->transform->translation.x + collider->cube.offset.x;
	plane_pos.y = collider->transform->translation.y + collider->cube.offset.y;
	plane_pos.z = collider->transform->translation.z + collider->cube.offset.z;

	muln.x = point.x - plane_pos.x;
	muln.y = point.y - plane_pos.y;
	muln.z = point.z - plane_pos.z;

	L3_makeRotationMatrixZXY(collider->transform->rotation.x,
							collider->transform->rotation.y,
							collider->transform->rotation.z,
							transMat);

	L3_vec3Xmat4(&up, transMat);
	L3_vec3Xmat4(&right, transMat);
	L3_vec3Xmat4(&far, transMat);

	L3_Unit dotx = L3_vec3Dot(right, muln);
	L3_Unit x = collider->cube.size.x * collider->transform->scale.x / L3_F;
	L3_Unit y = collider->cube.size.y * collider->transform->scale.y / L3_F;
	L3_Unit z = collider->cube.size.z * collider->transform->scale.z / L3_F;
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

static void do_collision_sphere(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 pos, up;

	pos.x = collider->transform->translation.x + collider->sphere.offset.x;
	pos.y = collider->transform->translation.y + collider->sphere.offset.y;
	pos.z = collider->transform->translation.z + collider->sphere.offset.z;

	L3_Unit x = point.x - pos.x;
	L3_Unit y = point.y - pos.y;
	L3_Unit z = point.z - pos.z;
	L3_Unit d = abs(x) + abs(y) + abs(z);
	if (d*1.1 > collider->sphere.size / 2)
		return;
	if (d < collider->sphere.size / 2) {
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

static void do_collision_capsule(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 pos, up;
	L3_Vec4 r_size;
	L3_Mat4 transMat;

	pos.x = collider->transform->translation.x + collider->capsule.offset.x;
	pos.y = collider->transform->translation.y + collider->capsule.offset.y;
	pos.z = collider->transform->translation.z + collider->capsule.offset.z;

	L3_makeRotationMatrixZXY(collider->transform->rotation.x,
						collider->transform->rotation.y,
						collider->transform->rotation.z,
						transMat);

	r_size = collider->capsule.size;
	r_size.x *= collider->transform->scale.x / L3_F;
	r_size.y *= collider->transform->scale.y / L3_F;
	r_size.z *= collider->transform->scale.z / L3_F;
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

static void do_collision_axisplane_y(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider->transform->translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider->transform->translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider->transform->translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.x * collider->transform->scale.x / L3_F;
	L3_Unit z = collider->axisplane.size.z * collider->transform->scale.z / L3_F;
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

static void do_collision_axisplane_z(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider->transform->translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider->transform->translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider->transform->translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.x * collider->transform->scale.x / L3_F;
	L3_Unit z = collider->axisplane.size.y * collider->transform->scale.y / L3_F;
	if (point.x > plane_pos.x + x / 2 || point.x < plane_pos.x - x / 2)
		return;
	if (point.y > plane_pos.y + z / 2 || point.y < plane_pos.y - z / 2)
		return;
	if (collider->axisplane.size.z > 0) {
		if (point.y > plane_pos.z) {
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

static void do_collision_axisplane_x(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	L3_Vec4 plane_pos;

	plane_pos.x = collider->transform->translation.x + collider->axisplane.offset.x;
	plane_pos.y = collider->transform->translation.y + collider->axisplane.offset.y;
	plane_pos.z = collider->transform->translation.z + collider->axisplane.offset.z;

	L3_Unit x = collider->axisplane.size.y * collider->transform->scale.y / L3_F;
	L3_Unit z = collider->axisplane.size.z * collider->transform->scale.z / L3_F;
	if (point.y > plane_pos.y + x / 2 || point.y < plane_pos.y - x / 2)
		return;
	if (point.z > plane_pos.z + z / 2 || point.z < plane_pos.z - z / 2)
		return;
	if (collider->axisplane.size.x > 0) {
		if (point.y > plane_pos.x) {
			return;
		}
		if (object->physics.last_transform.translation.x < plane_pos.x && collider->axisplane.traverseable) {
			return;
		}
	} else {
		if (point.y < plane_pos.x) {
			return;
		}
		if (object->physics.last_transform.translation.x > plane_pos.x && collider->axisplane.traverseable) {
			return;
		}
	}
	object->physics.transform->translation.x = plane_pos.x;
	object->physics.speeds.translation.x = -object->physics.speeds.translation.x * collider->axisplane.bouncyness / L3_F;
}

static void do_collision(Engine_DObject *object, const E_Collider *collider, L3_Vec4 point)
{
	switch (collider->type)
	{
		case ENGINE_COLLIDER_CUBE:
			do_collision_cube(object, collider, point);
		break;
		case ENGINE_COLLIDER_SPHERE:
			do_collision_sphere(object, collider, point);
		break;
		case ENGINE_COLLIDER_APLANEY:
			do_collision_axisplane_y(object, collider, point);
		break;
		case ENGINE_COLLIDER_APLANEX:
			do_collision_axisplane_x(object, collider, point);
		break;
		case ENGINE_COLLIDER_APLANEZ:
			do_collision_axisplane_z(object, collider, point);
		break;
		case ENGINE_COLLIDER_CAPSULE:
			do_collision_capsule(object, collider, point);
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
				do_collision(&(engine_dynamic_objects[i]), engine_colliders[j], engine_dynamic_objects[i].physics.transform->translation);
			}
		} else {
			L3_makeWorldMatrix(*engine_dynamic_objects[i].physics.transform, transMat);
			for (int k = 0; k < engine_dynamic_objects[i].physics.pointOffsetsCount; k++) {
				point_transformed = engine_dynamic_objects[i].physics.pointOffsets[k];
				L3_vec3Xmat4(&point_transformed, transMat);
				for (int j = 0; j < engine_colliders_count; j++) {
					do_collision(&(engine_dynamic_objects[i]), engine_colliders[j], point_transformed);
				}
			}
		}
		engine_dynamic_objects[i].physics.last_transform = *engine_dynamic_objects[i].physics.transform;
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
		build_collider_list();
		run_all_DObjects();
		k_mutex_unlock(&engine_objects_lock);

#if	CONFIG_LOG_PERFORMANCE
		end_time = timing_counter_get();
		total_time_us = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time)) / 1000;
		printf("total process us: %u ms:%u fps:%u\n", total_time_us, (total_time_us) / 1000, 1000000 / (total_time_us != 0 ? total_time_us : 1));
#endif

		while (!sys_timepoint_expired(timing)) {
			k_sleep(K_NSEC(100));
			k_yield();
		}
		/* force unready thread to avoid monopolization of CPU time */
		k_sleep(K_NSEC(10));
		k_yield();
	}
}

static struct k_thread render_thread;
K_THREAD_STACK_DEFINE(render_thread_stack, CONFIG_RENDER_THREAD_STACK);

static struct k_thread process_thread;
K_THREAD_STACK_DEFINE(process_thread_stack, CONFIG_PROCESS_THREAD_STACK);

int init_engine(Engine_pf pf)
{
	L3_sceneInit(L3_OBJECTS, 0, &L3_SCENE);
	L3_SCENE.camera.transform.translation.y = 0 * L3_F;
	L3_SCENE.camera.transform.translation.z = 0 * L3_F;
	//L3_SCENE.camera.focalLength = 0;

	engine_pf = pf;


	k_thread_create(&render_thread, render_thread_stack, CONFIG_RENDER_THREAD_STACK,
                render_function, NULL, NULL, NULL,
                15, 0, K_NO_WAIT);

	k_thread_create(&process_thread, process_thread_stack, CONFIG_PROCESS_THREAD_STACK,
                process_function, NULL, NULL, NULL,
                5, 0, K_NO_WAIT);

	return 0;
}
