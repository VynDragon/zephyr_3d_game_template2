#include <stdlib.h>
#include <math.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(L3);

#include "L3.h"

/* Privates --------------------------------------------------------------------------------------*/

#define L3_UNUSED(what) (void)(what) ///< helper macro for unused vars

#define L3_HALF_RESOLUTION_X (L3_RESOLUTION_X >> 1)
#define L3_HALF_RESOLUTION_Y (L3_RESOLUTION_Y >> 1)

#define L3_PROJECTION_PLANE_HEIGHT\
	((L3_RESOLUTION_Y * L3_F * 2) / L3_RESOLUTION_X)

#ifndef L3_MAX_PIXELS
#define L3_MAX_PIXELS L3_RESOLUTION_X * L3_RESOLUTION_Y
#endif

#if L3_SORT != 0
typedef struct
{
	uint8_t objectIndex;
	L3_Index triangleIndex;
	uint16_t sortValue;
} _L3_TriangleToSort;

_L3_TriangleToSort L3_sortArray[L3_MAX_TRIANGES_DRAWN];
uint16_t L3_sortArrayLength;
#endif

/* Data ------------------------------------------------------------------------------------------*/

#if DT_HAS_CHOSEN(zephyr_dtcm)
__attribute__((section("DTCM"))) L3_COLORTYPE L3_video_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];
/* why not? After all... */
#elif DT_HAS_CHOSEN(zephyr_itcm)
__attribute__((section("ITCM"))) L3_COLORTYPE L3_video_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];
#else
L3_COLORTYPE L3_video_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];
#endif

#if L3_Z_BUFFER == 1
	#define L3_MAX_DEPTH 2147483647
#if DT_HAS_CHOSEN(zephyr_dtcm)
__attribute__((section("DTCM"))) L3_Unit L3_zBuffer[L3_MAX_PIXELS];
#elif DT_HAS_CHOSEN(zephyr_itcm)
__attribute__((section("ITCM"))) L3_Unit L3_zBuffer[L3_MAX_PIXELS];
#else
	L3_Unit L3_zBuffer[L3_MAX_PIXELS];
#endif
	#define L3_zBufferFormat(depth) (depth)
#elif L3_Z_BUFFER == 2
	#define L3_MAX_DEPTH 255
#if DT_HAS_CHOSEN(zephyr_dtcm)
__attribute__((section("DTCM"))) uint8_t L3_zBuffer[L3_MAX_PIXELS];
#elif DT_HAS_CHOSEN(zephyr_itcm)
__attribute__((section("ITCM"))) uint8_t L3_zBuffer[L3_MAX_PIXELS];
#else
	uint8_t L3_zBuffer[L3_MAX_PIXELS];
#endif
	#define L3_zBufferFormat(depth)\
		L3_min(255,(depth) >> L3_REDUCED_Z_BUFFER_GRANULARITY)
#endif

const L3_Object *engine_global_objects[L3_MAX_OBJECTS] = {0};
L3_Scene engine_global_scene = {0};

/* Code ------------------------------------------------------------------------------------------*/

L3_PERFORMANCE_FUNCTION
void L3_clearScreen(L3_COLORTYPE color)
{
	uint32_t index = 0;

	for (; index < L3_RESOLUTION_Y * L3_RESOLUTION_X; index++)
	{
		L3_video_buffer[index] = color;
	}
}

L3_PERFORMANCE_FUNCTION
void L3_plot_line (L3_COLORTYPE color, int x0, int y0, int x1, int y1)
{
	int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */

	for (;;){  /* loop */
		if (0 < x0 && L3_RESOLUTION_X > x0 && 0 < y0 && L3_RESOLUTION_Y > y0) {
			L3_video_buffer[x0 + y0 * L3_RESOLUTION_X] = color;
#if L3_Z_BUFFER
			L3_zBuffer[y0 * L3_RESOLUTION_X + x0] = 0;
#endif
		}
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

#if L3_Z_BUFFER
int8_t L3_zTest(
	L3_ScreenCoord x,
	L3_ScreenCoord y,
	L3_Unit depth)
{
	uint32_t index = y * L3_RESOLUTION_X + x;

	depth = L3_zBufferFormat(depth);

#if L3_Z_BUFFER == 2
	#define cmp <= /* For reduced z-buffer we need equality test, because
										otherwise pixels at the maximum depth (255) would never be
										drawn over the background (which also has the depth of
										255). */
#else
	#define cmp <  /* For normal z-buffer we leave out equality test to not waste
										time by drawing over already drawn pixls. */
#endif

	if (depth cmp L3_zBuffer[index])
	{
		L3_zBuffer[index] = depth;
		return 1;
	}

#undef cmp

	return 0;
}
#endif

L3_Unit L3_zBufferRead(L3_ScreenCoord x, L3_ScreenCoord y)
{
#if L3_Z_BUFFER
	return L3_zBuffer[y * L3_RESOLUTION_X + x];
#else
	L3_UNUSED(x);
	L3_UNUSED(y);

	return 0;
#endif
}

void L3_zBufferWrite(L3_ScreenCoord x, L3_ScreenCoord y, L3_Unit value)
{
#if L3_Z_BUFFER
	L3_zBuffer[y * L3_RESOLUTION_X + x] = value;
#else
	L3_UNUSED(x);
	L3_UNUSED(y);
	L3_UNUSED(value);
#endif
}

#if L3_STENCIL_BUFFER
	#define L3_STENCIL_BUFFER_SIZE\
		((L3_RESOLUTION_X * L3_RESOLUTION_Y - 1) / 8 + 1)

uint8_t L3_stencilBuffer[L3_STENCIL_BUFFER_SIZE];

static inline int8_t L3_stencilTest(
	L3_ScreenCoord x,
	L3_ScreenCoord y)
{
	uint32_t index = y * L3_RESOLUTION_X + x;
	uint32_t bit = (index & 0x00000007);
	index = index >> 3;

	uint8_t val = L3_stencilBuffer[index];

	if ((val >> bit) & 0x1)
		return 0;

	L3_stencilBuffer[index] = val | (0x1 << bit);

	return 1;
}
#endif

#define L3_COMPUTE_LERP_DEPTH\
	(L3_COMPUTE_DEPTH && (L3_PERSPECTIVE_CORRECTION == 0))

#define L3_SIN_TABLE_LENGTH 128

#if L3_SIN_METHOD == 0
static const L3_Unit L3_sinTable[L3_SIN_TABLE_LENGTH] =
{
	/* 511 was chosen here as a highest number that doesn't overflow during
		compilation for L3_F == 1024 */

	(0*L3_F)/511, (6*L3_F)/511,
	(12*L3_F)/511, (18*L3_F)/511,
	(25*L3_F)/511, (31*L3_F)/511,
	(37*L3_F)/511, (43*L3_F)/511,
	(50*L3_F)/511, (56*L3_F)/511,
	(62*L3_F)/511, (68*L3_F)/511,
	(74*L3_F)/511, (81*L3_F)/511,
	(87*L3_F)/511, (93*L3_F)/511,
	(99*L3_F)/511, (105*L3_F)/511,
	(111*L3_F)/511, (118*L3_F)/511,
	(124*L3_F)/511, (130*L3_F)/511,
	(136*L3_F)/511, (142*L3_F)/511,
	(148*L3_F)/511, (154*L3_F)/511,
	(160*L3_F)/511, (166*L3_F)/511,
	(172*L3_F)/511, (178*L3_F)/511,
	(183*L3_F)/511, (189*L3_F)/511,
	(195*L3_F)/511, (201*L3_F)/511,
	(207*L3_F)/511, (212*L3_F)/511,
	(218*L3_F)/511, (224*L3_F)/511,
	(229*L3_F)/511, (235*L3_F)/511,
	(240*L3_F)/511, (246*L3_F)/511,
	(251*L3_F)/511, (257*L3_F)/511,
	(262*L3_F)/511, (268*L3_F)/511,
	(273*L3_F)/511, (278*L3_F)/511,
	(283*L3_F)/511, (289*L3_F)/511,
	(294*L3_F)/511, (299*L3_F)/511,
	(304*L3_F)/511, (309*L3_F)/511,
	(314*L3_F)/511, (319*L3_F)/511,
	(324*L3_F)/511, (328*L3_F)/511,
	(333*L3_F)/511, (338*L3_F)/511,
	(343*L3_F)/511, (347*L3_F)/511,
	(352*L3_F)/511, (356*L3_F)/511,
	(361*L3_F)/511, (365*L3_F)/511,
	(370*L3_F)/511, (374*L3_F)/511,
	(378*L3_F)/511, (382*L3_F)/511,
	(386*L3_F)/511, (391*L3_F)/511,
	(395*L3_F)/511, (398*L3_F)/511,
	(402*L3_F)/511, (406*L3_F)/511,
	(410*L3_F)/511, (414*L3_F)/511,
	(417*L3_F)/511, (421*L3_F)/511,
	(424*L3_F)/511, (428*L3_F)/511,
	(431*L3_F)/511, (435*L3_F)/511,
	(438*L3_F)/511, (441*L3_F)/511,
	(444*L3_F)/511, (447*L3_F)/511,
	(450*L3_F)/511, (453*L3_F)/511,
	(456*L3_F)/511, (459*L3_F)/511,
	(461*L3_F)/511, (464*L3_F)/511,
	(467*L3_F)/511, (469*L3_F)/511,
	(472*L3_F)/511, (474*L3_F)/511,
	(476*L3_F)/511, (478*L3_F)/511,
	(481*L3_F)/511, (483*L3_F)/511,
	(485*L3_F)/511, (487*L3_F)/511,
	(488*L3_F)/511, (490*L3_F)/511,
	(492*L3_F)/511, (494*L3_F)/511,
	(495*L3_F)/511, (497*L3_F)/511,
	(498*L3_F)/511, (499*L3_F)/511,
	(501*L3_F)/511, (502*L3_F)/511,
	(503*L3_F)/511, (504*L3_F)/511,
	(505*L3_F)/511, (506*L3_F)/511,
	(507*L3_F)/511, (507*L3_F)/511,
	(508*L3_F)/511, (509*L3_F)/511,
	(509*L3_F)/511, (510*L3_F)/511,
	(510*L3_F)/511, (510*L3_F)/511,
	(510*L3_F)/511, (510*L3_F)/511
};
#endif

#define L3_SIN_TABLE_UNIT_STEP\
	(L3_F / (L3_SIN_TABLE_LENGTH * 4))

void L3_mat4Copy(L3_Mat4 src, L3_Mat4 dst)
{
	for (uint8_t j = 0; j < 4; ++j)
		for (uint8_t i = 0; i < 4; ++i)
			dst[i][j] = src[i][j];
}

void L3_reflect(L3_Vec4 toLight, L3_Vec4 normal, L3_Vec4 *result)
{
	L3_Unit d = 2 * L3_vec3Dot(toLight,normal);

	result->x = (normal.x * d) / L3_F - toLight.x;
	result->y = (normal.y * d) / L3_F - toLight.y;
	result->z = (normal.z * d) / L3_F - toLight.z;
}

void L3_vec3Cross(L3_Vec4 a, L3_Vec4 b, L3_Vec4 *result)
{
	result->x = a.y * b.z - a.z * b.y;
	result->y = a.z * b.x - a.x * b.z;
	result->z = a.x * b.y - a.y * b.x;
}

void L3_triangleNormal(L3_Vec4 t0, L3_Vec4 t1, L3_Vec4 t2, L3_Vec4 *n)
{
	#define ANTI_OVERFLOW 32

	t1.x = (t1.x - t0.x) / ANTI_OVERFLOW;
	t1.y = (t1.y - t0.y) / ANTI_OVERFLOW;
	t1.z = (t1.z - t0.z) / ANTI_OVERFLOW;

	t2.x = (t2.x - t0.x) / ANTI_OVERFLOW;
	t2.y = (t2.y - t0.y) / ANTI_OVERFLOW;
	t2.z = (t2.z - t0.z) / ANTI_OVERFLOW;

	#undef ANTI_OVERFLOW

	L3_vec3Cross(t1,t2,n);

	L3_vec3Normalize(n);
}

void L3_getIndexedTriangleValues(
	L3_Index triangleIndex,
	const L3_Index *indices,
	const L3_Unit *values,
	uint8_t numComponents,
	L3_Vec4 *v0,
	L3_Vec4 *v1,
	L3_Vec4 *v2)
{
	uint32_t i0, i1;
	L3_Unit *value;

	i0 = triangleIndex * 3;
	i1 = indices[i0] * numComponents;
	value = (L3_Unit *) v0;

	if (numComponents > 4)
		numComponents = 4;

	for (uint8_t j = 0; j < numComponents; ++j)
	{
		*value = values[i1];
		i1++;
		value++;
	}

	i0++;
	i1 = indices[i0] * numComponents;
	value = (L3_Unit *) v1;

	for (uint8_t j = 0; j < numComponents; ++j)
	{
		*value = values[i1];
		i1++;
		value++;
	}

	i0++;
	i1 = indices[i0] * numComponents;
	value = (L3_Unit *) v2;

	for (uint8_t j = 0; j < numComponents; ++j)
	{
		*value = values[i1];
		i1++;
		value++;
	}
}

L3_PERFORMANCE_FUNCTION
void L3_computeModelNormals(L3_Object *object, L3_Unit *dst,
	int8_t transformNormals)
{
	L3_Index vPos = 0;

	L3_Vec4 n;

	n.w = 0;

	L3_Vec4 ns[L3_NORMAL_COMPUTE_MAXIMUM_AVERAGE];
	L3_Index normalCount;

	for (uint32_t i = 0; i < object->model->vertexCount; ++i)
	{
		normalCount = 0;

		for (uint32_t j = 0; j < object->model->triangleCount * 3; j += 3)
		{
			if (
				(object->model->triangles[j] == i) ||
				(object->model->triangles[j + 1] == i) ||
				(object->model->triangles[j + 2] == i))
			{
				L3_Vec4 t0, t1, t2;
				uint32_t vIndex;

				#define getVertex(n)\
					vIndex = object->model->triangles[j + n] * 3;\
					t##n.x = object->model->vertices[vIndex];\
					vIndex++;\
					t##n.y = object->model->vertices[vIndex];\
					vIndex++;\
					t##n.z = object->model->vertices[vIndex];

				getVertex(0)
				getVertex(1)
				getVertex(2)

				#undef getVertex

				L3_triangleNormal(t0,t1,t2,&(ns[normalCount]));

				normalCount++;

				if (normalCount >= L3_NORMAL_COMPUTE_MAXIMUM_AVERAGE)
					break;
			}
		}

		n.x = L3_F;
		n.y = 0;
		n.z = 0;

		if (normalCount != 0)
		{
			// compute average

			n.x = 0;

			for (uint8_t i = 0; i < normalCount; ++i)
			{
				n.x += ns[i].x;
				n.y += ns[i].y;
				n.z += ns[i].z;
			}

			n.x /= normalCount;
			n.y /= normalCount;
			n.z /= normalCount;

			L3_vec3Normalize(&n);
		}

		dst[vPos] = n.x;
		vPos++;

		dst[vPos] = n.y;
		vPos++;

		dst[vPos] = n.z;
		vPos++;
	}

	L3_Mat4 m;

	L3_makeWorldMatrix(object->transform,m);

	if (transformNormals)
		for (L3_Index i = 0; i < object->model->vertexCount * 3; i += 3)
		{
			n.x = dst[i];
			n.y = dst[i + 1];
			n.z = dst[i + 2];

			L3_vec4Xmat4(&n,m);

			dst[i] = n.x;
			dst[i + 1] = n.y;
			dst[i + 2] = n.z;
		}
}

L3_PERFORMANCE_FUNCTION
void L3_vec4Xmat4(L3_Vec4 *v, L3_Mat4 m)
{
	L3_Vec4 vBackup;

	vBackup.x = v->x;
	vBackup.y = v->y;
	vBackup.z = v->z;
	vBackup.w = v->w;

	#define dotCol(col)\
		((vBackup.x * m[col][0]) +\
		(vBackup.y * m[col][1]) +\
		(vBackup.z * m[col][2]) +\
		(vBackup.w * m[col][3])) / L3_F

	v->x = dotCol(0);
	v->y = dotCol(1);
	v->z = dotCol(2);
	v->w = dotCol(3);
}

L3_PERFORMANCE_FUNCTION
void L3_vec3Xmat4(L3_Vec4 *v, L3_Mat4 m)
{
	L3_Vec4 vBackup;

	#undef dotCol
	#define dotCol(col)\
		(vBackup.x * m[col][0]) / L3_F +\
		(vBackup.y * m[col][1]) / L3_F +\
		(vBackup.z * m[col][2]) / L3_F +\
		m[col][3]

	vBackup.x = v->x;
	vBackup.y = v->y;
	vBackup.z = v->z;
	vBackup.w = v->w;

	v->x = dotCol(0);
	v->y = dotCol(1);
	v->z = dotCol(2);
	v->w = L3_F;
}

#undef dotCol

L3_PERFORMANCE_FUNCTION
void L3_mat4Xmat4(L3_Mat4 m1, L3_Mat4 m2)
{
	L3_Mat4 mat1;

	for (uint16_t row = 0; row < 4; ++row)
		for (uint16_t col = 0; col < 4; ++col)
			mat1[col][row] = m1[col][row];

	for (uint16_t row = 0; row < 4; ++row)
		for (uint16_t col = 0; col < 4; ++col)
		{
			m1[col][row] = 0;

			for (uint16_t i = 0; i < 4; ++i)
				m1[col][row] +=
					(mat1[i][row] * m2[col][i]) / L3_F;
		}
}

L3_PERFORMANCE_FUNCTION
L3_Unit L3_sin(L3_Unit x)
{
#if L3_SIN_METHOD == 0
	x = L3_wrap(x / L3_SIN_TABLE_UNIT_STEP,L3_SIN_TABLE_LENGTH * 4);
	int8_t positive = 1;

	if (x < L3_SIN_TABLE_LENGTH)
	{
	}
	else if (x < L3_SIN_TABLE_LENGTH * 2)
	{
		x = L3_SIN_TABLE_LENGTH * 2 - x - 1;
	}
	else if (x < L3_SIN_TABLE_LENGTH * 3)
	{
		x = x - L3_SIN_TABLE_LENGTH * 2;
		positive = 0;
	}
	else
	{
		x = L3_SIN_TABLE_LENGTH - (x - L3_SIN_TABLE_LENGTH * 3) - 1;
		positive = 0;
	}

	return positive ? L3_sinTable[x] : -1 * L3_sinTable[x];
#else
	int8_t sign = 1;

	if (x < 0) // odd function
	{
		x *= -1;
		sign = -1;
	}

	x %= L3_F;

	if (x > L3_F / 2)
	{
		x -= L3_F / 2;
		sign *= -1;
	}

	L3_Unit tmp = L3_F - 2 * x;

	#define _PI2 ((L3_Unit) (9.8696044 * L3_F))
	return sign * // Bhaskara's approximation
		(((32 * x * _PI2) / L3_F) * tmp) /
		((_PI2 * (5 * L3_F - (8 * x * tmp) /
			L3_F)) / L3_F);
	#undef _PI2
#endif
}

L3_PERFORMANCE_FUNCTION
L3_Unit L3_asin(L3_Unit x)
{
#if L3_SIN_METHOD == 0
	x = L3_clamp(x,-L3_F,L3_F);

	int8_t sign = 1;

	if (x < 0)
	{
		sign = -1;
		x *= -1;
	}

	int16_t low = 0, high = L3_SIN_TABLE_LENGTH -1, middle = 0;

	while (low <= high) // binary search
	{
		middle = (low + high) / 2;

		L3_Unit v = L3_sinTable[middle];

		if (v > x)
			high = middle - 1;
		else if (v < x)
			low = middle + 1;
		else
			break;
	}

	middle *= L3_SIN_TABLE_UNIT_STEP;

	return sign * middle;
#else
	L3_Unit low = -1 * L3_F / 4,
					high = L3_F / 4,
					middle;

	while (low <= high) // binary search
	{
		middle = (low + high) / 2;

		L3_Unit v = L3_sin(middle);

		if (v > x)
			high = middle - 1;
		else if (v < x)
			low = middle + 1;
		else
			break;
	}

	return middle;
#endif
}

void L3_makeTranslationMat(
	L3_Unit offsetX,
	L3_Unit offsetY,
	L3_Unit offsetZ,
	L3_Mat4 m)
{
	#define M(x,y) m[x][y]
	#define S L3_F

	M(0,0) = S; M(1,0) = 0; M(2,0) = 0; M(3,0) = 0;
	M(0,1) = 0; M(1,1) = S; M(2,1) = 0; M(3,1) = 0;
	M(0,2) = 0; M(1,2) = 0; M(2,2) = S; M(3,2) = 0;
	M(0,3) = offsetX; M(1,3) = offsetY; M(2,3) = offsetZ; M(3,3) = S;

	#undef M
	#undef S
}

void L3_makeScaleMatrix(
	L3_Unit scaleX,
	L3_Unit scaleY,
	L3_Unit scaleZ,
	L3_Mat4 m)
{
	#define M(x,y) m[x][y]

	M(0,0) = scaleX; M(1,0) = 0;      M(2,0) = 0;     M(3,0) = 0;
	M(0,1) = 0;      M(1,1) = scaleY; M(2,1) = 0; M(3,1) = 0;
	M(0,2) = 0;      M(1,2) = 0;      M(2,2) = scaleZ; M(3,2) = 0;
	M(0,3) = 0;      M(1,3) = 0;     M(2,3) = 0; M(3,3) = L3_F;

	#undef M
}

L3_PERFORMANCE_FUNCTION
void L3_makeRotationMatrixZXY(
	L3_Unit byX,
	L3_Unit byY,
	L3_Unit byZ,
	L3_Mat4 m)
{
	byX *= -1;
	byY *= -1;
	byZ *= -1;

	L3_Unit sx = L3_sin(byX);
	L3_Unit sy = L3_sin(byY);
	L3_Unit sz = L3_sin(byZ);

	L3_Unit cx = L3_cos(byX);
	L3_Unit cy = L3_cos(byY);
	L3_Unit cz = L3_cos(byZ);

	#define M(x,y) m[x][y]
	#define S L3_F

	M(0,0) = (cy * cz) / S + (sy * sx * sz) / (S * S);
	M(1,0) = (cx * sz) / S;
	M(2,0) = (cy * sx * sz) / (S * S) - (cz * sy) / S;
	M(3,0) = 0;

	M(0,1) = (cz * sy * sx) / (S * S) - (cy * sz) / S;
	M(1,1) = (cx * cz) / S;
	M(2,1) = (cy * cz * sx) / (S * S) + (sy * sz) / S;
	M(3,1) = 0;

	M(0,2) = (cx * sy) / S;
	M(1,2) = -1 * sx;
	M(2,2) = (cy * cx) / S;
	M(3,2) = 0;

	M(0,3) = 0;
	M(1,3) = 0;
	M(2,3) = 0;
	M(3,3) = L3_F;

	#undef M
	#undef S
}

L3_Unit L3_sqrt(L3_Unit value)
{
	int8_t sign = 1;

	if (value < 0)
	{
		sign = -1;
		value *= -1;
	}

	uint32_t result = 0;
	uint32_t a = value;
	uint32_t b = 1u << 30;

	while (b > a)
		b >>= 2;

	while (b != 0)
	{
		if (a >= result + b)
		{
			a -= result + b;
			result = result +  2 * b;
		}

		b >>= 2;
		result >>= 1;
	}

	return result * sign;
}

L3_Unit L3_vec3Length(L3_Vec4 v)
{
	return L3_sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

L3_Unit L3_vec2Length(L3_Vec4 v)
{
	return L3_sqrt(v.x * v.x + v.y * v.y);
}

L3_PERFORMANCE_FUNCTION
void L3_vec3Normalize(L3_Vec4 *v)
{
	#define SCALE 16
	#define BOTTOM_LIMIT 16
	#define UPPER_LIMIT 900

	/* Here we try to decide if the vector is too small and would cause
		inaccurate result due to very its inaccurate length. If so, we scale
		it up. We can't scale up everything as big vectors overflow in length
		calculations. */

	if (
		L3_abs(v->x) <= BOTTOM_LIMIT &&
		L3_abs(v->y) <= BOTTOM_LIMIT &&
		L3_abs(v->z) <= BOTTOM_LIMIT)
	{
		v->x *= SCALE;
		v->y *= SCALE;
		v->z *= SCALE;
	}
	else if (
		L3_abs(v->x) > UPPER_LIMIT ||
		L3_abs(v->y) > UPPER_LIMIT ||
		L3_abs(v->z) > UPPER_LIMIT)
	{
		v->x /= SCALE;
		v->y /= SCALE;
		v->z /= SCALE;
	}

	#undef SCALE
	#undef BOTTOM_LIMIT
	#undef UPPER_LIMIT

	L3_Unit l = L3_vec3Length(*v);

	if (l == 0)
		return;

	v->x = (v->x * L3_F) / l;
	v->y = (v->y * L3_F) / l;
	v->z = (v->z * L3_F) / l;
}

/** Performs perspecive division (z-divide). Does NOT check for division by
	zero. */
static inline void L3_perspectiveDivide(L3_Vec4 *vector,
	L3_Unit focalLength)
{
	if (focalLength == 0)
		return;

	vector->x = (vector->x * focalLength) / vector->z;
	vector->y = (vector->y * focalLength) / vector->z;
}

static inline void L3_mapProjectionPlaneToScreen(
	L3_Vec4 point,
	L3_ScreenCoord *screenX,
	L3_ScreenCoord *screenY)
{
	*screenX =
		L3_HALF_RESOLUTION_X +
		(point.x * L3_HALF_RESOLUTION_X) / L3_F;

	*screenY =
		L3_HALF_RESOLUTION_Y -
		(point.y * L3_HALF_RESOLUTION_X) / L3_F;
}

L3_PERFORMANCE_FUNCTION
void L3_project3DPointToScreen(
	L3_Vec4 point,
	L3_Camera camera,
	L3_Vec4 *result)
{
	// TODO: hotfix to prevent a mapping bug probably to overlfows
	L3_Vec4 toPoint = point, camForw;

	L3_vec3Sub(&toPoint,camera.transform.translation);

	L3_vec3Normalize(&toPoint);

	L3_rotationToDirections(camera.transform.rotation,L3_FRACTIONS_PER_UNIT,
		&camForw,0,0);

	if (L3_vec3Dot(toPoint,camForw) < L3_FRACTIONS_PER_UNIT / 6)
	{
		result->z = -1;
		result->w = 0;
		return;
	}
	// end of hotfix
	L3_Mat4 m;
	L3_makeCameraMatrix(camera.transform,m);

	L3_Unit s = point.w;

	point.w = L3_F;

	L3_vec3Xmat4(&point,m);

	point.z = L3_nonZero(point.z);

	L3_perspectiveDivide(&point,camera.focalLength);

	L3_ScreenCoord x, y;

	L3_mapProjectionPlaneToScreen(point,&x,&y);

	result->x = x;
	result->y = y;
	result->z = point.z;

	result->w =
		(point.z <= 0) ? 0 :
		(
			camera.focalLength > 0 ?(
			(s * camera.focalLength * L3_RESOLUTION_X) /
				(point.z * L3_F)) :
			((camera.transform.scale.x * L3_RESOLUTION_X) / L3_F)
		);
}

void L3_lookAt(L3_Vec4 pointTo, L3_Transform3D *t)
{
	L3_Vec4 v;

	v.x = pointTo.x - t->translation.x;
	v.y = pointTo.z - t->translation.z;

	L3_Unit dx = v.x;
	L3_Unit l = L3_vec2Length(v);

	dx = (v.x * L3_F) / L3_nonZero(l); // normalize

	t->rotation.y = -1 * L3_asin(dx);

	if (v.y < 0)
		t->rotation.y = L3_F / 2 - t->rotation.y;

	v.x = pointTo.y - t->translation.y;
	v.y = l;

	l = L3_vec2Length(v);

	dx = (v.x * L3_F) / L3_nonZero(l);

	t->rotation.x = L3_asin(dx);
}

void L3_transform3DSet(
	L3_Unit tx,
	L3_Unit ty,
	L3_Unit tz,
	L3_Unit rx,
	L3_Unit ry,
	L3_Unit rz,
	L3_Unit sx,
	L3_Unit sy,
	L3_Unit sz,
	L3_Transform3D *t)
{
	t->translation.x = tx;
	t->translation.y = ty;
	t->translation.z = tz;

	t->rotation.x = rx;
	t->rotation.y = ry;
	t->rotation.z = rz;

	t->scale.x = sx;
	t->scale.y = sy;
	t->scale.z = sz;
}

void L3_cameraInit(L3_Camera *camera)
{
	camera->focalLength = L3_F;
	L3_transform3DInit(&(camera->transform));
}

void L3_rotationToDirections(
	L3_Vec4 rotation,
	L3_Unit length,
	L3_Vec4 *forw,
	L3_Vec4 *right,
	L3_Vec4 *up)
{
	L3_Mat4 m;

	L3_makeRotationMatrixZXY(rotation.x,rotation.y,rotation.z,m);

	if (forw != 0)
	{
		forw->x = 0;
		forw->y = 0;
		forw->z = length;
		L3_vec3Xmat4(forw,m);
	}

	if (right != 0)
	{
		right->x = length;
		right->y = 0;
		right->z = 0;
		L3_vec3Xmat4(right,m);
	}

	if (up != 0)
	{
		up->x = 0;
		up->y = length;
		up->z = 0;
		L3_vec3Xmat4(up,m);
	}
}

void L3_sceneInit(
	const L3_Object **objects,
	L3_Index objectCount,
	L3_Scene *scene)
{
	scene->objects = objects;
	scene->objectCount = objectCount;
	L3_cameraInit(&(scene->camera));
}

void L3_drawConfigInit(L3_DrawConfig *config)
{
	config->backfaceCulling = 2;
	config->visible = 1;
}





#define L3_getFastLerpValue(state)\
	(state.valueScaled >> L3_FAST_LERP_QUALITY)

#define L3_stepFastLerp(state)\
	state.valueScaled += state.stepScaled

L3_PERFORMANCE_FUNCTION
void L3_zBufferClear(void)
{
#if L3_Z_BUFFER
	for (uint32_t i = 0; i < L3_RESOLUTION_X * L3_RESOLUTION_Y; ++i)
		L3_zBuffer[i] = L3_MAX_DEPTH;
#endif
}

L3_PERFORMANCE_FUNCTION
void L3_stencilBufferClear(void)
{
#if L3_STENCIL_BUFFER
	for (uint32_t i = 0; i < L3_STENCIL_BUFFER_SIZE; ++i)
		L3_stencilBuffer[i] = 0;
#endif
}

L3_PERFORMANCE_FUNCTION
void L3_newFrame(void)
{
	L3_zBufferClear();
	L3_stencilBufferClear();
}

/* the following serves to communicate info about if the triangle has been split
	and how the barycentrics should be remapped. */
uint8_t _L3_projectedTriangleState = 0; // 0 = normal, 1 = cut, 2 = split

#if L3_NEAR_CROSS_STRATEGY == 3
L3_Vec4 _L3_triangleRemapBarycentrics[6];
#endif

__attribute__((flatten))
L3_PERFORMANCE_FUNCTION
void L3_drawTriangle(
	L3_Vec4 point0,
	L3_Vec4 point1,
	L3_Vec4 point2,
	L3_Index objectIndex,
	L3_Index triangleIndex)
{
	L3_PixelInfo p;
	L3_pixelInfoInit(&p);
	p.objectIndex = objectIndex;
	p.triangleIndex = triangleIndex;
	p.triangleID = (objectIndex << 16) | triangleIndex;

	L3_Vec4 *tPointSS, *lPointSS, *rPointSS; /* points in Screen Space (in
																							L3_Units, normalized by
																							L3_F) */

	L3_Unit *barycentric0; // bar. coord that gets higher from L to R
	L3_Unit *barycentric1; // bar. coord that gets higher from R to L
	L3_Unit *barycentric2; // bar. coord that gets higher from bottom up

	// sort the vertices:

	#define assignPoints(t,a,b)\
		{\
			tPointSS = &point##t;\
			barycentric2 = &(p.barycentric[t]);\
			if (L3_triangleWinding(point##t.x,point##t.y,point##a.x,point##a.y,\
				point##b.x,point##b.y) >= 0)\
			{\
				lPointSS = &point##a; rPointSS = &point##b;\
				barycentric0 = &(p.barycentric[b]);\
				barycentric1 = &(p.barycentric[a]);\
			}\
			else\
			{\
				lPointSS = &point##b; rPointSS = &point##a;\
				barycentric0 = &(p.barycentric[a]);\
				barycentric1 = &(p.barycentric[b]);\
			}\
		}

	if (point0.y <= point1.y)
	{
		if (point0.y <= point2.y)
			assignPoints(0,1,2)
		else
			assignPoints(2,0,1)
	}
	else
	{
		if (point1.y <= point2.y)
			assignPoints(1,0,2)
		else
			assignPoints(2,0,1)
	}

	#undef assignPoints

#if L3_FLAT
	*barycentric0 = L3_F / 3;
	*barycentric1 = L3_F / 3;
	*barycentric2 = L3_F - 2 * (L3_F / 3);
#endif

	p.triangleSize[0] = rPointSS->x - lPointSS->x;
	p.triangleSize[1] =
		(rPointSS->y > lPointSS->y ? rPointSS->y : lPointSS->y) - tPointSS->y;

	// now draw the triangle line by line:

	L3_ScreenCoord splitY; // Y of the vertically middle point of the triangle
	L3_ScreenCoord endY;   // bottom Y of the whole triangle
	int splitOnLeft;        /* whether splitY is the y coord. of left or right
														point */

	if (rPointSS->y <= lPointSS->y)
	{
		splitY = rPointSS->y;
		splitOnLeft = 0;
		endY = lPointSS->y;
	}
	else
	{
		splitY = lPointSS->y;
		splitOnLeft = 1;
		endY = rPointSS->y;
	}

	L3_ScreenCoord currentY = tPointSS->y;

	/* We'll be using an algorithm similar to Bresenham line algorithm. The
		specifics of this algorithm are among others:

		- drawing possibly NON-CONTINUOUS line
		- NOT tracing the line exactly, but rather rasterizing one the right
			side of it, according to the pixel CENTERS, INCLUDING the pixel
			centers

		The principle is this:

		- Move vertically by pixels and accumulate the error (abs(dx/dy)).
		- If the error is greater than one (crossed the next pixel center), keep
			moving horizontally and substracting 1 from the error until it is less
			than 1 again.
		- To make this INTEGER ONLY, scale the case so that distance between
			pixels is equal to dy (instead of 1). This way the error becomes
			dx/dy * dy == dx, and we're comparing the error to (and potentially
			substracting) 1 * dy == dy. */

	int16_t
		/* triangle side:
		left     right */
		lX,      rX,       // current x position on the screen
		lDx,     rDx,      // dx (end point - start point)
		lDy,     rDy,      // dy (end point - start point)
		lInc,    rInc,     // direction in which to increment (1 or -1)
		lErr,    rErr,     // current error (Bresenham)
		lErrCmp, rErrCmp,  // helper for deciding comparison (> vs >=)
		lErrAdd, rErrAdd,  // error value to add in each Bresenham cycle
		lErrSub, rErrSub;  // error value to substract when moving in x direction

	L3_FastLerpState lSideFLS, rSideFLS;

#if L3_COMPUTE_LERP_DEPTH
	L3_FastLerpState lDepthFLS, rDepthFLS;

	#define initDepthFLS(s,p1,p2)\
		s##DepthFLS.valueScaled = p1##PointSS->z << L3_FAST_LERP_QUALITY;\
		s##DepthFLS.stepScaled = ((p2##PointSS->z << L3_FAST_LERP_QUALITY) -\
			s##DepthFLS.valueScaled) / (s##Dy != 0 ? s##Dy : 1);
#else
	#define initDepthFLS(s,p1,p2) ;
#endif

	/* init side for the algorithm, params:
		s - which side (l or r)
		p1 - point from (t, l or r)
		p2 - point to (t, l or r)
		down - whether the side coordinate goes top-down or vice versa */
	#define initSide(s,p1,p2,down)\
		s##X = p1##PointSS->x;\
		s##Dx = p2##PointSS->x - p1##PointSS->x;\
		s##Dy = p2##PointSS->y - p1##PointSS->y;\
		initDepthFLS(s,p1,p2)\
		s##SideFLS.stepScaled = (L3_F << L3_FAST_LERP_QUALITY)\
											/ (s##Dy != 0 ? s##Dy : 1);\
		s##SideFLS.valueScaled = 0;\
		if (!down)\
		{\
			s##SideFLS.valueScaled =\
				L3_F << L3_FAST_LERP_QUALITY;\
			s##SideFLS.stepScaled *= -1;\
		}\
		s##Inc = s##Dx >= 0 ? 1 : -1;\
		if (s##Dx < 0)\
			{s##Err = 0;     s##ErrCmp = 0;}\
		else\
			{s##Err = s##Dy; s##ErrCmp = 1;}\
		s##ErrAdd = L3_abs(s##Dx);\
		s##ErrSub = s##Dy != 0 ? s##Dy : 1; /* don't allow 0, could lead to an
																					infinite substracting loop */

	#define stepSide(s)\
		while (s##Err - s##Dy >= s##ErrCmp)\
		{\
			s##X += s##Inc;\
			s##Err -= s##ErrSub;\
		}\
		s##Err += s##ErrAdd;

	initSide(r,t,r,1)
	initSide(l,t,l,1)

#if L3_PERSPECTIVE_CORRECTION
	/* PC is done by linearly interpolating reciprocals from which the corrected
		velues can be computed. See
		http://www.lysator.liu.se/~mikaelk/doc/perspectivetexture/ */

	#if L3_PERSPECTIVE_CORRECTION == 1
		#define Z_RECIP_NUMERATOR\
			(L3_F * L3_F * L3_F)
	#elif L3_PERSPECTIVE_CORRECTION == 2
		#define Z_RECIP_NUMERATOR\
			(L3_F * L3_F)
	#endif
	/* ^ This numerator is a number by which we divide values for the
		reciprocals. For PC == 2 it has to be lower because linear interpolation
		scaling would make it overflow -- this results in lower depth precision
		in bigger distance for PC == 2. */

	L3_Unit
		tPointRecipZ, lPointRecipZ, rPointRecipZ, /* Reciprocals of the depth of
																								each triangle point. */
		lRecip0, lRecip1, rRecip0, rRecip1;       /* Helper variables for swapping
																								the above after split. */

	tPointRecipZ = Z_RECIP_NUMERATOR / L3_nonZero(tPointSS->z);
	lPointRecipZ = Z_RECIP_NUMERATOR / L3_nonZero(lPointSS->z);
	rPointRecipZ = Z_RECIP_NUMERATOR / L3_nonZero(rPointSS->z);

	lRecip0 = tPointRecipZ;
	lRecip1 = lPointRecipZ;
	rRecip0 = tPointRecipZ;
	rRecip1 = rPointRecipZ;

	#define manageSplitPerspective(b0,b1)\
		b1##Recip0 = b0##PointRecipZ;\
		b1##Recip1 = b1##PointRecipZ;\
		b0##Recip0 = b0##PointRecipZ;\
		b0##Recip1 = tPointRecipZ;
#else
	#define manageSplitPerspective(b0,b1) ;
#endif

	// clip to the screen in y dimension:

	endY = L3_min(endY,L3_RESOLUTION_Y);

	/* Clipping above the screen (y < 0) can't be easily done here, will be
		handled inside the loop. */

	while (currentY < endY)   /* draw the triangle from top to bottom -- the
															bottom-most row is left out because, following
															from the rasterization rules (see start of the
															file), it is to never be rasterized. */
	{
		if (currentY == splitY) // reached a vertical split of the triangle?
		{
			#define manageSplit(b0,b1,s0,s1)\
				L3_Unit *tmp = barycentric##b0;\
				barycentric##b0 = barycentric##b1;\
				barycentric##b1 = tmp;\
				s0##SideFLS.valueScaled = (L3_F\
					<< L3_FAST_LERP_QUALITY) - s0##SideFLS.valueScaled;\
				s0##SideFLS.stepScaled *= -1;\
				manageSplitPerspective(s0,s1)

			if (splitOnLeft)
			{
				initSide(l,l,r,0);
				manageSplit(0,2,r,l)
			}
			else
			{
				initSide(r,r,l,0);
				manageSplit(1,2,l,r)
			}
		}

		stepSide(r)
		stepSide(l)

		if (currentY >= 0) /* clipping of pixels whose y < 0 (can't be easily done
													outside the loop because of the Bresenham-like
													algorithm steps) */
		{
			p.y = currentY;

			// draw the horizontal line

#if !L3_FLAT
			L3_Unit rowLength = L3_nonZero(rX - lX - 1); // prevent zero div

	#if L3_PERSPECTIVE_CORRECTION
			L3_Unit lOverZ, lRecipZ, rOverZ, rRecipZ, lT, rT;

			lT = L3_getFastLerpValue(lSideFLS);
			rT = L3_getFastLerpValue(rSideFLS);

			lOverZ  = L3_interpolateByUnitFrom0(lRecip1,lT);
			lRecipZ = L3_interpolateByUnit(lRecip0,lRecip1,lT);

			rOverZ  = L3_interpolateByUnitFrom0(rRecip1,rT);
			rRecipZ = L3_interpolateByUnit(rRecip0,rRecip1,rT);
	#else
			L3_FastLerpState b0FLS, b1FLS;

		#if L3_COMPUTE_LERP_DEPTH
			L3_FastLerpState  depthFLS;

			depthFLS.valueScaled = lDepthFLS.valueScaled;
			depthFLS.stepScaled =
				(rDepthFLS.valueScaled - lDepthFLS.valueScaled) / rowLength;
		#endif

			b0FLS.valueScaled = 0;
			b1FLS.valueScaled = lSideFLS.valueScaled;

			b0FLS.stepScaled = rSideFLS.valueScaled / rowLength;
			b1FLS.stepScaled = -1 * lSideFLS.valueScaled / rowLength;
	#endif
#endif

			// clip to the screen in x dimension:

			L3_ScreenCoord rXClipped = L3_min(rX,L3_RESOLUTION_X),
											lXClipped = lX;

			if (lXClipped < 0)
			{
				lXClipped = 0;

#if !L3_PERSPECTIVE_CORRECTION && !L3_FLAT
				b0FLS.valueScaled -= lX * b0FLS.stepScaled;
				b1FLS.valueScaled -= lX * b1FLS.stepScaled;

	#if L3_COMPUTE_LERP_DEPTH
				depthFLS.valueScaled -= lX * depthFLS.stepScaled;
	#endif
#endif
			}

#if L3_PERSPECTIVE_CORRECTION
			L3_ScreenCoord i = lXClipped - lX;  /* helper var to save one
																							substraction in the inner
																							loop */
#endif

#if L3_PERSPECTIVE_CORRECTION == 2
			L3_FastLerpState
				depthPC, // interpolates depth between row segments
				b0PC,    // interpolates barycentric0 between row segments
				b1PC;    // interpolates barycentric1 between row segments

			/* ^ These interpolate values between row segments (lines of pixels
					of L3_PC_APPROX_LENGTH length). After each row segment perspective
					correction is recomputed. */

			depthPC.valueScaled =
				(Z_RECIP_NUMERATOR /
				L3_nonZero(L3_interpolate(lRecipZ,rRecipZ,i,rowLength)))
				<< L3_FAST_LERP_QUALITY;

			b0PC.valueScaled =
					(
						L3_interpolateFrom0(rOverZ,i,rowLength)
						* depthPC.valueScaled
					) / (Z_RECIP_NUMERATOR / L3_F);

			b1PC.valueScaled =
					(
						(lOverZ - L3_interpolateFrom0(lOverZ,i,rowLength))
						* depthPC.valueScaled
					) / (Z_RECIP_NUMERATOR / L3_F);

			int8_t rowCount = L3_PC_APPROX_LENGTH;
#endif

#if L3_Z_BUFFER
			uint32_t zBufferIndex = p.y * L3_RESOLUTION_X + lXClipped;
#endif

			// draw the row -- inner loop:
			for (L3_ScreenCoord x = lXClipped; x < rXClipped; ++x)
			{
				int8_t testsPassed = 1;

#if L3_STENCIL_BUFFER
				if (!L3_stencilTest(x,p.y))
					testsPassed = 0;
#endif
				p.x = x;

#if L3_COMPUTE_DEPTH
	#if L3_PERSPECTIVE_CORRECTION == 1
				p.depth = Z_RECIP_NUMERATOR /
					L3_nonZero(L3_interpolate(lRecipZ,rRecipZ,i,rowLength));
	#elif L3_PERSPECTIVE_CORRECTION == 2
				if (rowCount >= L3_PC_APPROX_LENGTH)
				{
					// init the linear interpolation to the next PC correct value

					rowCount = 0;

					L3_Unit nextI = i + L3_PC_APPROX_LENGTH;

					if (nextI < rowLength)
					{
						L3_Unit nextDepthScaled =
							(
							Z_RECIP_NUMERATOR /
							L3_nonZero(L3_interpolate(lRecipZ,rRecipZ,nextI,rowLength))
							) << L3_FAST_LERP_QUALITY;

						depthPC.stepScaled =
							(nextDepthScaled - depthPC.valueScaled) / L3_PC_APPROX_LENGTH;

						L3_Unit nextValue =
						(
							L3_interpolateFrom0(rOverZ,nextI,rowLength)
							* nextDepthScaled
						) / (Z_RECIP_NUMERATOR / L3_F);

						b0PC.stepScaled =
							(nextValue - b0PC.valueScaled) / L3_PC_APPROX_LENGTH;

						nextValue =
						(
							(lOverZ - L3_interpolateFrom0(lOverZ,nextI,rowLength))
							* nextDepthScaled
						) / (Z_RECIP_NUMERATOR / L3_F);

						b1PC.stepScaled =
							(nextValue - b1PC.valueScaled) / L3_PC_APPROX_LENGTH;
					}
					else
					{
						/* A special case where we'd be interpolating outside the triangle.
							It seems like a valid approach at first, but it creates a bug
							in a case when the rasaterized triangle is near screen 0 and can
							actually never reach the extrapolated screen position. So we
							have to clamp to the actual end of the triangle here. */

						L3_Unit maxI = L3_nonZero(rowLength - i);

						L3_Unit nextDepthScaled =
							(
							Z_RECIP_NUMERATOR /
							L3_nonZero(rRecipZ)
							) << L3_FAST_LERP_QUALITY;

						depthPC.stepScaled =
							(nextDepthScaled - depthPC.valueScaled) / maxI;

						L3_Unit nextValue =
						(
							rOverZ
							* nextDepthScaled
						) / (Z_RECIP_NUMERATOR / L3_F);

						b0PC.stepScaled =
							(nextValue - b0PC.valueScaled) / maxI;

						b1PC.stepScaled =
							-1 * b1PC.valueScaled / maxI;
					}
				}

				p.depth = L3_getFastLerpValue(depthPC);
	#else
				p.depth = L3_getFastLerpValue(depthFLS);
				L3_stepFastLerp(depthFLS);
	#endif
#else   // !L3_COMPUTE_DEPTH
				p.depth = (tPointSS->z + lPointSS->z + rPointSS->z) / 3;
#endif

#if L3_Z_BUFFER
				p.previousZ = L3_zBuffer[zBufferIndex];

				zBufferIndex++;

				if (!L3_zTest(p.x,p.y,p.depth))
					testsPassed = 0;
#endif

				if (testsPassed)
				{
#if !L3_FLAT
	#if L3_PERSPECTIVE_CORRECTION == 0
					*barycentric0 = L3_getFastLerpValue(b0FLS);
					*barycentric1 = L3_getFastLerpValue(b1FLS);
	#elif L3_PERSPECTIVE_CORRECTION == 1
					*barycentric0 =
					(
						L3_interpolateFrom0(rOverZ,i,rowLength)
						* p.depth
					) / (Z_RECIP_NUMERATOR / L3_F);

					*barycentric1 =
					(
						(lOverZ - L3_interpolateFrom0(lOverZ,i,rowLength))
						* p.depth
					) / (Z_RECIP_NUMERATOR / L3_F);
	#elif L3_PERSPECTIVE_CORRECTION == 2
					*barycentric0 = L3_getFastLerpValue(b0PC);
					*barycentric1 = L3_getFastLerpValue(b1PC);
	#endif

					*barycentric2 =
						L3_F - *barycentric0 - *barycentric1;
#endif

#if L3_NEAR_CROSS_STRATEGY == 3
					if (_L3_projectedTriangleState != 0)
					{
						L3_Unit newBarycentric[3];

						newBarycentric[0] = L3_interpolateBarycentric(
							_L3_triangleRemapBarycentrics[0].x,
							_L3_triangleRemapBarycentrics[1].x,
							_L3_triangleRemapBarycentrics[2].x,
							p.barycentric);

						newBarycentric[1] = L3_interpolateBarycentric(
							_L3_triangleRemapBarycentrics[0].y,
							_L3_triangleRemapBarycentrics[1].y,
							_L3_triangleRemapBarycentrics[2].y,
							p.barycentric);

						newBarycentric[2] = L3_interpolateBarycentric(
							_L3_triangleRemapBarycentrics[0].z,
							_L3_triangleRemapBarycentrics[1].z,
							_L3_triangleRemapBarycentrics[2].z,
							p.barycentric);

						p.barycentric[0] = newBarycentric[0];
						p.barycentric[1] = newBarycentric[1];
						p.barycentric[2] = newBarycentric[2];
					}
#endif
					L3_PIXEL_FUNCTION(&p);
				} // tests passed

#if !L3_FLAT
	#if L3_PERSPECTIVE_CORRECTION
					i++;
		#if L3_PERSPECTIVE_CORRECTION == 2
					rowCount++;

					L3_stepFastLerp(depthPC);
					L3_stepFastLerp(b0PC);
					L3_stepFastLerp(b1PC);
		#endif
	#else
					L3_stepFastLerp(b0FLS);
					L3_stepFastLerp(b1FLS);
	#endif
#endif
			} // inner loop
		} // y clipping

#if !L3_FLAT
		L3_stepFastLerp(lSideFLS);
		L3_stepFastLerp(rSideFLS);

	#if L3_COMPUTE_LERP_DEPTH
		L3_stepFastLerp(lDepthFLS);
		L3_stepFastLerp(rDepthFLS);
	#endif
#endif

		++currentY;
	} // row drawing

	#undef manageSplit
	#undef initPC
	#undef initSide
	#undef stepSide
	#undef Z_RECIP_NUMERATOR
}

void L3_rotate2DPoint(L3_Unit *x, L3_Unit *y, L3_Unit angle)
{
	if (angle < L3_SIN_TABLE_UNIT_STEP)
		return; // no visible rotation

	L3_Unit angleSin = L3_sin(angle);
	L3_Unit angleCos = L3_cos(angle);

	L3_Unit xBackup = *x;

	*x =
		(angleCos * (*x)) / L3_F -
		(angleSin * (*y)) / L3_F;

	*y =
		(angleSin * xBackup) / L3_F +
		(angleCos * (*y)) / L3_F;
}

L3_PERFORMANCE_FUNCTION
void L3_makeWorldMatrix(L3_Transform3D worldTransform, L3_Mat4 m)
{
	L3_makeScaleMatrix(
		worldTransform.scale.x,
		worldTransform.scale.y,
		worldTransform.scale.z,
		m);

	L3_Mat4 t;

	L3_makeRotationMatrixZXY(
		worldTransform.rotation.x,
		worldTransform.rotation.y,
		worldTransform.rotation.z,
		t);

	L3_mat4Xmat4(m,t);

	L3_makeTranslationMat(
		worldTransform.translation.x,
		worldTransform.translation.y,
		worldTransform.translation.z,
		t);

	L3_mat4Xmat4(m,t);
}

L3_PERFORMANCE_FUNCTION
void L3_mat4Transpose(L3_Mat4 m)
{
	L3_Unit tmp;

	for (uint8_t y = 0; y < 3; ++y)
		for (uint8_t x = 1 + y; x < 4; ++x)
		{
			tmp = m[x][y];
			m[x][y] = m[y][x];
			m[y][x] = tmp;
		}
}

void L3_makeCameraMatrix(L3_Transform3D cameraTransform, L3_Mat4 m)
{
	L3_makeTranslationMat(
		-1 * cameraTransform.translation.x,
		-1 * cameraTransform.translation.y,
		-1 * cameraTransform.translation.z,
		m);

	L3_Mat4 r;

	L3_makeRotationMatrixZXY(
		cameraTransform.rotation.x,
		cameraTransform.rotation.y,
		cameraTransform.rotation.z,
		r);

	L3_mat4Transpose(r); // transposing creates an inverse transform

	L3_Mat4 s;

	L3_makeScaleMatrix(
	cameraTransform.scale.x,
	cameraTransform.scale.y,
	cameraTransform.scale.z,s);

	L3_mat4Xmat4(m,r);
	L3_mat4Xmat4(m,s);
}

/** Checks if given triangle (in Screen Space) is at least partially visible,
	i.e. returns false if the triangle is either completely outside the frustum
	(left, right, top, bottom, near) or is invisible due to backface culling. */
L3_PERFORMANCE_FUNCTION
static inline int8_t L3_triangleIsVisible(
	L3_Vec4 p0,
	L3_Vec4 p1,
	L3_Vec4 p2,
	uint8_t backfaceCulling)
{
	#define clipTest(c,cmp,v)\
		(p0.c cmp (v) && p1.c cmp (v) && p2.c cmp (v))

	if ( // outside frustum?
#if L3_NEAR_CROSS_STRATEGY == 0
			p0.z <= L3_NEAR || p1.z <= L3_NEAR || p2.z <= L3_NEAR ||
			// ^ partially in front of NEAR?
#else
			clipTest(z,<=,L3_NEAR) || // completely in front of NEAR?
#endif
			clipTest(x,<,0) ||
			clipTest(x,>=,L3_RESOLUTION_X) ||
			clipTest(y,<,0) ||
			clipTest(y,>,L3_RESOLUTION_Y)
		)
		return 0;

	#undef clipTest

	if (backfaceCulling != 0)
	{
		int8_t winding =
			L3_triangleWinding(p0.x,p0.y,p1.x,p1.y,p2.x,p2.y);

		if ((backfaceCulling == 1 && winding > 0) ||
				(backfaceCulling == 2 && winding < 0))
			return 0;
	}

	return 1;
}

L3_PERFORMANCE_FUNCTION
void _L3_projectVertex(const L3_Object *object, L3_Index triangleIndex,
	uint8_t vertex, L3_Mat4 projectionMatrix, L3_Vec4 *result)
{
	uint32_t vertexIndex = object->model->triangles[triangleIndex * 3 + vertex] * 3;

	result->x = object->model->vertices[vertexIndex];
	result->y = object->model->vertices[vertexIndex + 1];
	result->z = object->model->vertices[vertexIndex + 2];
	result->w = L3_F; // needed for translation

	L3_vec3Xmat4(result,projectionMatrix);

	result->w = result->z;
	/* We'll keep the non-clamped z in w for sorting. */
}

L3_PERFORMANCE_FUNCTION
void _L3_mapProjectedVertexToScreen(L3_Vec4 *vertex, L3_Unit focalLength)
{
	vertex->z = vertex->z >= L3_NEAR ? vertex->z : L3_NEAR;
	/* ^ This firstly prevents zero division in the follwoing z-divide and
		secondly "pushes" vertices that are in front of near a little bit forward,
		which makes them behave a bit better. If all three vertices end up exactly
		on NEAR, the triangle will be culled. */

	L3_perspectiveDivide(vertex,focalLength);

	L3_ScreenCoord sX, sY;

	L3_mapProjectionPlaneToScreen(*vertex,&sX,&sY);

	vertex->x = sX;
	vertex->y = sY;
}

/** Projects a triangle to the screen. If enabled, a triangle can be potentially
	subdivided into two if it crosses the near plane, in which case two projected
	triangles are returned (the info about splitting or cutting the triangle is
	passed in global variables, see above). */
L3_PERFORMANCE_FUNCTION
void _L3_projectTriangle(
	const L3_Object *object,
	L3_Index triangleIndex,
	L3_Mat4 matrix,
	uint32_t focalLength,
	L3_Vec4 transformed[6])
{
	_L3_projectVertex(object,triangleIndex,0,matrix,&(transformed[0]));
	_L3_projectVertex(object,triangleIndex,1,matrix,&(transformed[1]));
	_L3_projectVertex(object,triangleIndex,2,matrix,&(transformed[2]));
	_L3_projectedTriangleState = 0;

#if L3_NEAR_CROSS_STRATEGY == 2 || L3_NEAR_CROSS_STRATEGY == 3
	uint8_t infront = 0;
	uint8_t behind = 0;
	uint8_t infrontI[3];
	uint8_t behindI[3];

	for (uint8_t i = 0; i < 3; ++i)
		if (transformed[i].z < L3_NEAR)
		{
			infrontI[infront] = i;
			infront++;
		}
		else
		{
			behindI[behind] = i;
			behind++;
		}

#if L3_NEAR_CROSS_STRATEGY == 3
		for (int i = 0; i < 3; ++i)
			L3_vec4Init(&(_L3_triangleRemapBarycentrics[i]));

		_L3_triangleRemapBarycentrics[0].x = L3_F;
		_L3_triangleRemapBarycentrics[1].y = L3_F;
		_L3_triangleRemapBarycentrics[2].z = L3_F;
#endif

#define interpolateVertex \
	L3_Unit ratio =\
		((transformed[be].z - L3_NEAR) * L3_F) /\
		(transformed[be].z - transformed[in].z);\
	transformed[in].x = transformed[be].x - \
		((transformed[be].x - transformed[in].x) * ratio) /\
			L3_F;\
	transformed[in].y = transformed[be].y -\
		((transformed[be].y - transformed[in].y) * ratio) /\
			L3_F;\
	transformed[in].z = L3_NEAR;\
	if (beI != 0) {\
		beI->x = (beI->x * ratio) / L3_F;\
		beI->y = (beI->y * ratio) / L3_F;\
		beI->z = (beI->z * ratio) / L3_F;\
		ratio = L3_F - ratio;\
		beI->x += (beB->x * ratio) / L3_F;\
		beI->y += (beB->y * ratio) / L3_F;\
		beI->z += (beB->z * ratio) / L3_F; }

	if (infront == 2)
	{
		// shift the two vertices forward along the edge
		for (uint8_t i = 0; i < 2; ++i)
		{
			uint8_t be = behindI[0], in = infrontI[i];

#if L3_NEAR_CROSS_STRATEGY == 3
			L3_Vec4 *beI = &(_L3_triangleRemapBarycentrics[in]),
							*beB = &(_L3_triangleRemapBarycentrics[be]);
#else
			L3_Vec4 *beI = 0, *beB = 0;
#endif

			interpolateVertex

			_L3_projectedTriangleState = 1;
		}
	}
	else if (infront == 1)
	{
		// create another triangle and do the shifts
		transformed[3] = transformed[behindI[1]];
		transformed[4] = transformed[infrontI[0]];
		transformed[5] = transformed[infrontI[0]];

#if L3_NEAR_CROSS_STRATEGY == 3
		_L3_triangleRemapBarycentrics[3] =
			_L3_triangleRemapBarycentrics[behindI[1]];
		_L3_triangleRemapBarycentrics[4] =
			_L3_triangleRemapBarycentrics[infrontI[0]];
		_L3_triangleRemapBarycentrics[5] =
			_L3_triangleRemapBarycentrics[infrontI[0]];
#endif

		for (uint8_t i = 0; i < 2; ++i)
		{
			uint8_t be = behindI[i], in = i + 4;

#if L3_NEAR_CROSS_STRATEGY == 3
			L3_Vec4 *beI = &(_L3_triangleRemapBarycentrics[in]),
							*beB = &(_L3_triangleRemapBarycentrics[be]);
#else
			L3_Vec4 *beI = 0, *beB = 0;
#endif

			interpolateVertex
		}

#if L3_NEAR_CROSS_STRATEGY == 3
		_L3_triangleRemapBarycentrics[infrontI[0]] =
			_L3_triangleRemapBarycentrics[4];
#endif

		transformed[infrontI[0]] = transformed[4];

		_L3_mapProjectedVertexToScreen(&transformed[3],focalLength);
		_L3_mapProjectedVertexToScreen(&transformed[4],focalLength);
		_L3_mapProjectedVertexToScreen(&transformed[5],focalLength);

		_L3_projectedTriangleState = 2;
	}

#undef interpolateVertex
#endif // L3_NEAR_CROSS_STRATEGY == 2

	_L3_mapProjectedVertexToScreen(&transformed[0],focalLength);
	_L3_mapProjectedVertexToScreen(&transformed[1],focalLength);
	_L3_mapProjectedVertexToScreen(&transformed[2],focalLength);
}

/* returns triangles drawn */
__attribute__((flatten))
L3_PERFORMANCE_FUNCTION
uint32_t L3_drawScene(L3_Scene scene)
{
	uint32_t drawnTriangles = 0;
	L3_Mat4 matFinal, matCamera;
	L3_Vec4 transformed[6]; // transformed triangle coords, for 2 triangles

	const L3_Object *object;
	L3_Index objectIndex, triangleIndex;

	L3_makeCameraMatrix(scene.camera.transform,matCamera);

#if L3_SORT != 0
	uint16_t previousModel = 0;
	L3_sortArrayLength = 0;
#endif

	for (objectIndex = 0; objectIndex < scene.objectCount; ++objectIndex)
	{
		if (!scene.objects[objectIndex]->config.visible)
			continue;

		if(scene.objects[objectIndex]->config.visible & L3_VISIBLE_BILLBOARD)
		{
			L3_makeWorldMatrix(scene.objects[objectIndex]->transform,matFinal);
			L3_mat4Xmat4(matFinal,matCamera);

			transformed->x = 0;
			transformed->y = 0;
			transformed->z = 0;
			transformed->w = L3_F;

			L3_vec3Xmat4(transformed, matFinal);

			transformed->w = transformed->z;

			_L3_mapProjectedVertexToScreen(transformed, scene.camera.focalLength);
			if (transformed->x < 0 || transformed->x >= L3_RESOLUTION_X || transformed->y < 0 || transformed->y >= L3_RESOLUTION_Y || transformed->z <= L3_NEAR)
			{
				continue;
			}

			L3_BILLBOARD_FUNCTION(*transformed, scene.objects[objectIndex]);
			continue;
		} else {
			L3_MODEL_FUNCTION(scene.objects[objectIndex]);
		}

#if L3_SORT != 0
		if (L3_sortArrayLength >= L3_MAX_TRIANGES_DRAWN)
			break;

		previousModel = objectIndex;
#endif

		L3_makeWorldMatrix(scene.objects[objectIndex]->transform,matFinal);
		L3_mat4Xmat4(matFinal,matCamera);

		L3_Index triangleCount = scene.objects[objectIndex]->model->triangleCount;

		triangleIndex = 0;

		object = scene.objects[objectIndex];

		while (triangleIndex < triangleCount)
		{
			/* Some kind of cache could be used in theory to not project perviously
				already projected vertices, but after some testing this was abandoned,
				no gain was seen. */

			_L3_projectTriangle(object,triangleIndex,matFinal,
				scene.camera.focalLength,transformed);

			if (L3_triangleIsVisible(transformed[0],transformed[1],transformed[2],
				object->config.backfaceCulling))
			{
				if (L3_TRIANGLE_FUNCTION(transformed[0],transformed[1],transformed[2],objectIndex,triangleIndex))
				{
#if L3_SORT == 0
				// without sorting draw right away
				L3_drawTriangle(transformed[0],transformed[1],transformed[2],objectIndex,
					triangleIndex);

				if (_L3_projectedTriangleState == 2) // draw potential subtriangle
				{
#if L3_NEAR_CROSS_STRATEGY == 3
					_L3_triangleRemapBarycentrics[0] = _L3_triangleRemapBarycentrics[3];
					_L3_triangleRemapBarycentrics[1] = _L3_triangleRemapBarycentrics[4];
					_L3_triangleRemapBarycentrics[2] = _L3_triangleRemapBarycentrics[5];
#endif

					L3_drawTriangle(transformed[3],transformed[4],transformed[5],
					objectIndex, triangleIndex);
				}
#else

				if (L3_sortArrayLength >= L3_MAX_TRIANGES_DRAWN)
					break;

				// with sorting add to a sort list
				L3_sortArray[L3_sortArrayLength].objectIndex = objectIndex;
				L3_sortArray[L3_sortArrayLength].triangleIndex = triangleIndex;
				L3_sortArray[L3_sortArrayLength].sortValue = L3_zeroClamp(
					transformed[0].w + transformed[1].w + transformed[2].w) >> 2;
				/* ^
					The w component here stores non-clamped z.

					As a simple approximation we sort by the triangle center point,
					which is a mean coordinate -- we don't actually have to divide by 3
					(or anything), that is unnecessary for sorting! We shift by 2 just
					as a fast operation to prevent overflow of the sum over uint_16t. */

				L3_sortArrayLength++;
#endif
				}
				drawnTriangles += 1;
			}

			triangleIndex++;
		}
	}

#if L3_SORT != 0

	#if L3_SORT == 1
		#define cmp <
	#else
		#define cmp >
	#endif

	/* Sort the triangles. We use insertion sort, because it has many advantages,
	especially for smaller arrays (better than bubble sort, in-place, stable,
	simple, ...). */

	for (int16_t i = 1; i < L3_sortArrayLength; ++i)
	{
		_L3_TriangleToSort tmp = L3_sortArray[i];

		int16_t j = i - 1;

		while (j >= 0 && L3_sortArray[j].sortValue cmp tmp.sortValue)
		{
			L3_sortArray[j + 1] = L3_sortArray[j];
			j--;
		}

		L3_sortArray[j + 1] = tmp;
	}

	#undef cmp

	for (L3_Index i = 0; i < L3_sortArrayLength; ++i) // draw sorted triangles
	{
		objectIndex = L3_sortArray[i].objectIndex;
		triangleIndex = L3_sortArray[i].triangleIndex;

		object = &(scene.objects[objectIndex]);

		if (objectIndex != previousModel)
		{
			// only recompute the matrix when the model has changed
			L3_makeWorldMatrix(object->transform,matFinal);
			L3_mat4Xmat4(matFinal,matCamera);
			previousModel = objectIndex;
		}

		/* Here we project the points again, which is redundant and slow as they've
			already been projected above, but saving the projected points would
			require a lot of memory, which for small resolutions could be even
			worse than z-bufer. So this seems to be the best way memory-wise. */

		_L3_projectTriangle(object,triangleIndex,matFinal,scene.camera.focalLength,
			transformed);

		L3_drawTriangle(transformed[0],transformed[1],transformed[2],objectIndex,
			triangleIndex);

		if (_L3_projectedTriangleState == 2)
		{
#if L3_NEAR_CROSS_STRATEGY == 3
			_L3_triangleRemapBarycentrics[0] = _L3_triangleRemapBarycentrics[3];
			_L3_triangleRemapBarycentrics[1] = _L3_triangleRemapBarycentrics[4];
			_L3_triangleRemapBarycentrics[2] = _L3_triangleRemapBarycentrics[5];
#endif

			L3_drawTriangle(transformed[3],transformed[4],transformed[5],
			objectIndex, triangleIndex);
		}
	}
#endif
	return drawnTriangles;
}


/* Zephyr Code -----------------------------------------------------------------------------------*/

static int L3_zephyr_putpixel_current_render_mode = 0;
static L3_Vec4	triangleNormal;
L3_PERFORMANCE_FUNCTION
inline void zephyr_putpixel(L3_PixelInfo *p)
{
	float depthmul = 1.0;
	const L3_Object *object = engine_global_scene.objects[p->objectIndex];
	L3_COLORTYPE color;

	if (L3_zephyr_putpixel_current_render_mode & L3_VISIBLE_TEXTURED) {
		if (object->model->triangleTextureIndex[p->triangleIndex] < 0) {
			return;
		}
		L3_Unit uv[2];

		const L3_Unit *uvs = &(object->model->triangleUVs[p->triangleIndex * 6]);

		uv[0] = abs(L3_interpolateBarycentric(uvs[0], uvs[2], uvs[4], p->barycentric) % L3_TEXTURE_WH);
		uv[1] = abs(L3_interpolateBarycentric(uvs[1], uvs[3], uvs[5], p->barycentric) % L3_TEXTURE_WH);
		color = object->model->triangleTextures[object->model->triangleTextureIndex[p->triangleIndex]][(uv[0] >> 0) + (uv[1] >> 0) * L3_TEXTURE_WH];
	} else {
		color = object->solid_color;
	}
	if (unlikely(0 > p->x && L3_RESOLUTION_X <= p->x && 0 > p->y && L3_RESOLUTION_Y <= p->y)) return;
	if (L3_zephyr_putpixel_current_render_mode & L3_VISIBLE_DISTANCELIGHT) {
		//depthmul = (p->depth * p->depth) / 524288;
		depthmul = sqrt(p->depth) / 64;
		depthmul = 1 / (depthmul != 0 ? depthmul : 1);
		depthmul = depthmul > 1 ? 1 : depthmul;
		color *= depthmul;
	}
	if (L3_zephyr_putpixel_current_render_mode & L3_VISIBLE_NORMALLIGHT)
	{
		L3_Vec4 down = {0,L3_F,0,L3_F};
		L3_Unit dot = L3_vec3Dot(triangleNormal, down);
		if (dot > 0)
			color = color / 4 + (color * dot) / L3_F * 3/4;
		else
			color /= 4;
	}
	L3_video_buffer[p->x + p->y * L3_RESOLUTION_X] = color;
}

L3_PERFORMANCE_FUNCTION
inline int zephyr_drawtriangle(L3_Vec4 point0, L3_Vec4 point1, L3_Vec4 point2,
								L3_Index objectIndex, L3_Index triangleIndex)
{
	L3_zephyr_putpixel_current_render_mode = engine_global_scene.objects[objectIndex]->config.visible;
	if (L3_zephyr_putpixel_current_render_mode & L3_VISIBLE_WIREFRAME) {
		L3_plot_line(255, point0.x, point0.y, point1.x, point1.y);
		L3_plot_line(255, point2.x, point2.y, point1.x, point1.y);
		L3_plot_line(255, point2.x, point2.y, point0.x, point0.y);
		if (!(L3_zephyr_putpixel_current_render_mode & ~L3_VISIBLE_WIREFRAME))
			return 0;
	}
	if (L3_zephyr_putpixel_current_render_mode & L3_VISIBLE_NORMALLIGHT) {
		const L3_Object *object = engine_global_scene.objects[objectIndex];
		L3_Mat4 matFinal;
		L3_Index v1 = object->model->triangles[triangleIndex * 3];
		L3_Index v2 = object->model->triangles[triangleIndex * 3 + 1];
		L3_Index v3 = object->model->triangles[triangleIndex * 3 + 2];
		L3_Vec4 a1 = {object->model->vertices[v1 * 3], object->model->vertices[v1 * 3 + 1], object->model->vertices[v1 * 3 + 2], L3_F};
		L3_Vec4 a2 = {object->model->vertices[v2 * 3], object->model->vertices[v2 * 3 + 1], object->model->vertices[v2 * 3 + 2], L3_F};
		L3_Vec4 a3 = {object->model->vertices[v3 * 3], object->model->vertices[v3 * 3 + 1], object->model->vertices[v3 * 3 + 2], L3_F};

		L3_makeWorldMatrix(engine_global_scene.objects[objectIndex]->transform, matFinal);
		L3_vec3Xmat4(&a1, matFinal);
		L3_vec3Xmat4(&a2, matFinal);
		L3_vec3Xmat4(&a3, matFinal);
		L3_triangleNormal(a1, a2, a3, &triangleNormal);
	}
	return 1;
}

L3_PERFORMANCE_FUNCTION
inline int zephyr_drawbillboard(L3_Vec4 point, const L3_Object *billboard)
{
	if (!L3_zTest(point.x,point.y,point.z))
		return 0;

	int focal = L3_SCENE.camera.focalLength != 0 ? L3_SCENE.camera.focalLength : 1;
	float scale_x = ((float)(billboard->transform.scale.x * focal) / (float)point.z) * ((float)billboard->billboard->scale/0x4000) * L3_RESOLUTION_X / L3_F;
	float scale_y = ((float)(billboard->transform.scale.y * focal) / (float)point.z) * ((float)billboard->billboard->scale/0x4000) * L3_RESOLUTION_X / L3_F;
	int scaled_width = billboard->billboard->width * scale_x;
	int scaled_height = billboard->billboard->height * scale_y;

	int startx = point.x - scaled_width / 2;
	int endx =  MIN(point.x + scaled_width / 2, L3_RESOLUTION_X);
	int starty = point.y - scaled_height / 2;
	int endy =  MIN(point.y + scaled_height / 2, L3_RESOLUTION_Y);
	L3_Unit z = L3_zBufferFormat(point.z);
	for (int i = MAX(0, startx); i < endx; i++) {
		int m = (float)(i - startx) / scale_x;
		for (int j = MAX(0, starty); j < endy; j++) {
			int n = (float)(j - starty) / scale_y;
			if (billboard->billboard->texture[m + n * billboard->billboard->width] > billboard->billboard->transparency_threshold) {
				L3_video_buffer[j * L3_RESOLUTION_X + i] = billboard->billboard->texture[m + n * billboard->billboard->width];
				#if L3_Z_BUFFER
				L3_zBuffer[j * L3_RESOLUTION_X + i] = z;
				#endif
			}
		}
	}
	return 0;
}

L3_PERFORMANCE_FUNCTION
int zephyr_model(const L3_Object *object)
{
	return 0;
}
