/* A lot of this is pulled from a CC0 library called 'small3dlib'.
* This document will not be crediting the author, you may use google to find out why.
*/

#pragma once

#include <stdint.h>

#define L3_RESOLUTION_X CONFIG_RESOLUTION_X
#define L3_RESOLUTION_Y CONFIG_RESOLUTION_Y

#define L3_PIXEL_FUNCTION		zephyr_putpixel
#define L3_TRIANGLE_FUNCTION	zephyr_drawtriangle
#define L3_BILLBOARD_FUNCTION	zephyr_drawbillboard
#define L3_MODEL_FUNCTION		zephyr_model

#define L3_COLORTYPE uint8_t


#pragma GCC push_options
#pragma GCC optimize ("O0")
/* fucking idiot gcc */
static k_timepoint_t inline L3_FPS_TIMEPOINT(uint64_t fps)
{
	return sys_timepoint_calc(K_MSEC(1000 / fps));
}

#pragma GCC pop_options


#define L3_MAX_OBJECTS 0x2FF

#define L3_VISIBLE_INVISIBLE		0
#define L3_VISIBLE_TEXTURED			BIT(0)
#define L3_VISIBLE_WIREFRAME		BIT(1)
#define L3_VISIBLE_SOLID			BIT(2)
#define L3_VISIBLE_BILLBOARD		BIT(4)

#define L3_VISIBLE_NORMALLIGHT		BIT(15)
#define L3_VISIBLE_DISTANCELIGHT	BIT(14)

#if 1
	#define L3_PERFORMANCE_FUNCTION	__attribute__((optimize(3))) __attribute__((hot))
#else
	#define L3_PERFORMANCE_FUNCTION
#endif

/** Specifies how the library will handle triangles that partially cross the
near plane. These are problematic and require special handling. Possible
values:

0: Strictly cull any triangle crossing the near plane. This will make such
	triangles disappear. This is good for performance or models viewed only
	from at least small distance.
1: Forcefully push the vertices crossing near plane in front of it. This is
	a cheap technique that can be good enough for displaying simple
	environments on slow devices, but texturing and geometric artifacts/warps
	will appear.
2: Geometrically correct the triangles crossing the near plane. This may
	result in some triangles being subdivided into two and is a little more
	expensive, but the results will be geometrically correct, even though
	barycentric correction is not performed so texturing artifacts will
	appear. Can be ideal with L3_FLAT.
3: Perform both geometrical and barycentric correction of triangle crossing
	the near plane. This is significantly more expensive but results in
	correct rendering. */
#define L3_NEAR_CROSS_STRATEGY 3

/** If on, disables computation of per-pixel values such as barycentric
coordinates and depth -- these will still be available but will be the same
for the whole triangle. This can be used to create flat-shaded renders and
will be a lot faster. With this option on you will probably want to use
sorting instead of z-buffer. */
#define L3_FLAT 0

/** Specifies what type of perspective correction (PC) to use. Remember this
is an expensive operation! Possible values:

0: No perspective correction. Fastest, inaccurate from most angles.
1: Per-pixel perspective correction, accurate but very expensive.
2: Approximation (computing only at every L3_PC_APPROX_LENGTHth pixel).
	Quake-style approximation is used, which only computes the PC after
	L3_PC_APPROX_LENGTH pixels. This is reasonably accurate and fast. */
#define L3_PERSPECTIVE_CORRECTION 2

/** Whether to compute depth for each pixel (fragment). Some other options
may turn this on automatically. If you don't need depth information, turning
this off can save performance. Depth will still be accessible in
L3_PixelInfo, but will be constant -- equal to center point depth -- over
the whole triangle. */
#define L3_COMPUTE_DEPTH 1

/** What type of z-buffer (depth buffer) to use for visibility determination.
Possible values:

0: Don't use z-buffer. This saves a lot of memory, but visibility checking
	won't be pixel-accurate and has to mostly be done by other means (typically
	sorting).
1: Use full z-buffer (of L3_Units) for visibiltiy determination. This is the
	most accurate option (and also a fast one), but requires a big amount of
	memory.
2: Use reduced-size z-buffer (of bytes). This is fast and somewhat accurate,
	but inaccuracies can occur and a considerable amount of memory is
	needed. */
#define L3_Z_BUFFER 2

#if L3_Z_BUFFER == 2
#define L3_ZBUFTYPE uint8_t
#elif L3_Z_BUFFER == 1
#define L3_ZBUFTYPE L3_Unit
#else
#define L3_ZBUFTYPE L3_Unit
#endif

/* Limit on the on-screen triangle rendering size (not in-model size) to prevent rendering errors
 * trying to draw ludicrously big triangles, resulting in flashing display at certain angles.
 * Also theorically increases performance
 */
#define L3_TRI_OVRFL L3_F * 24

/** Whether to use stencil buffer for drawing -- with this a pixel that would
be resterized over an already rasterized pixel (within a frame) will be
discarded. This is mostly for front-to-back sorted drawing. */
#define L3_STENCIL_BUFFER 0

/** Defines how to sort triangles before drawing a frame. This can be used to
solve visibility in case z-buffer is not used, to prevent overwriting already
rasterized pixels, implement transparency etc. Note that for simplicity and
performance a relatively simple sorting is used which doesn't work completely
correctly, so mistakes can occur (even the best sorting wouldn't be able to
solve e.g. intersecting triangles). Note that sorting requires a bit of extra
memory -- an array of the triangles to sort -- the size of this array limits
the maximum number of triangles that can be drawn in a single frame
(L3_MAX_TRIANGES_DRAWN). Possible values:

0: Don't sort triangles. This is fastest and doesn't use extra memory.
1: Sort triangles from back to front. This can in most cases solve visibility
	without requiring almost any extra memory compared to z-buffer.
2: Sort triangles from front to back. This can be faster than back to front
	because we prevent computing pixels that will be overwritten by nearer
	ones, but we need a 1b stencil buffer for this (enable L3_STENCIL_BUFFER),
	so a bit more memory is needed. */
#define L3_SORT 0

/** Distance of the near clipping plane. Points in front or EXATLY ON this
plane are considered outside the frustum. This must be >= 0. */
#if L3_RESOLUTION_X > 256
#define L3_NEAR 128 * (L3_RESOLUTION_X / (L3_F / 2))
#else
#define L3_NEAR L3_RESOLUTION_X
#endif

/** If true, the library will use wider data types which will largely supress
many rendering bugs and imprecisions happening due to overflows, but this will
also consumer more RAM and may potentially be slower on computers with smaller
native integer. */
#define L3_USE_WIDER_TYPES 0

/** Says which method should be used for computing sin/cos functions, possible
values: 0 (lookup table, takes more program memory), 1 (Bhaskara's
approximation, slower). This may cause the trigonometric functions give
slightly different results.
2: math.h sin
3: sin lookup that doesnt fail stupidly
*/
#define L3_SIN_METHOD 3

/** For L3_PERSPECTIVE_CORRECTION == 2, this specifies after how many pixels
PC is recomputed. Should be a power of two to keep up the performance.
Smaller is nicer but slower. */
#define L3_PC_APPROX_LENGTH 64

/** For L3_Z_BUFFER == 2 this sets the reduced z-buffer granularity. */
#define L3_REDUCED_Z_BUFFER_GRANULARITY 7

/** Maximum number of triangles that can be drawn in sorted modes. This
affects the size of the cache used for triangle sorting. */
#define L3_MAX_TRIANGES_DRAWN 128

/** Affects the L3_computeModelNormals function. See its description for
details. */
#define L3_NORMAL_COMPUTE_MAXIMUM_AVERAGE 6

/** Quality (scaling) of SOME (stepped) linear interpolations. 0 will most
likely be a tiny bit faster, but artifacts can occur for bigger tris, while
higher values can fix this -- in theory all higher values will have the same
speed (it is a shift value), but it mustn't be too high to prevent
overflow. */
#define L3_FAST_LERP_QUALITY 10

/** Units of measurement in 3D space. There is L3_FRACTIONS_PER_UNIT in one
spatial unit. By dividing the unit into fractions we effectively achieve a
fixed point arithmetic. The number of fractions is a constant that serves as
1.0 in floating point arithmetic (normalization etc.). */

typedef
#if L3_USE_WIDER_TYPES
	int64_t
#else
	int32_t
#endif
	L3_Unit;

/** How many fractions a spatial unit is split into, i.e. this is the fixed
point scaling. This is NOT SUPPOSED TO BE REDEFINED, so rather don't do it
(otherwise things may overflow etc.). */
#define L3_FRACTIONS_PER_UNIT 512
#define L3_F L3_FRACTIONS_PER_UNIT

typedef
#if L3_USE_WIDER_TYPES
	int32_t
#else
	int16_t
#endif
	L3_ScreenCoord;

typedef
#if L3_USE_WIDER_TYPES
	uint32_t
#else
	uint16_t
#endif
	L3_Index;

#if L3_FLAT
	#define L3_COMPUTE_DEPTH 0
	#define L3_PERSPECTIVE_CORRECTION 0
	// don't disable z-buffer, it makes sense to use it with no sorting
#endif

#if L3_PERSPECTIVE_CORRECTION
	#define L3_COMPUTE_DEPTH 1 // PC inevitably computes depth, so enable it
#endif

#ifndef L3_NEAR
	/** Distance of the near clipping plane. Points in front or EXATLY ON this
	plane are considered outside the frustum. This must be >= 0. */
	#define L3_NEAR (L3_F / 4)
#endif

#if L3_NEAR <= 0
#define L3_NEAR 1 // Can't be <= 0.
#endif

#ifndef L3_MAX_PIXELS
#define L3_MAX_PIXELS L3_RESOLUTION_X * L3_RESOLUTION_Y
#endif

/** Vector that consists of four scalars and can represent homogenous
	coordinates, but is generally also used as Vec3 and Vec2 for various
	purposes. */
typedef struct
{
	L3_Unit x;
	L3_Unit y;
	L3_Unit z;
	L3_Unit w;
} L3_Vec4;

#define L3_logVec4(v)\
	printf("Vec4: %d %d %d %d\n",((v).x),((v).y),((v).z),((v).w))

/* Some vec funcs */
L3_Unit L3_vec3Length(L3_Vec4 v);
L3_Unit L3_sqrt(L3_Unit value);

static inline void L3_vec4Init(L3_Vec4 *v)
{
	v->x = 0; v->y = 0; v->z = 0; v->w = L3_F;
}

static inline void L3_vec4Set(L3_Vec4 *v, L3_Unit x, L3_Unit y, L3_Unit z, L3_Unit w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

static inline void L3_vec3Add(L3_Vec4 *result, L3_Vec4 added)
{
	result->x += added.x;
	result->y += added.y;
	result->z += added.z;
}

static inline void L3_vec3Sub(L3_Vec4 *result, L3_Vec4 substracted)
{
	result->x -= substracted.x;
	result->y -= substracted.y;
	result->z -= substracted.z;
}

/** Normalizes Vec3. Note that this function tries to normalize correctly
	rather than quickly! If you need to normalize quickly, do it yourself in a
	way that best fits your case. */
void L3_vec3Normalize(L3_Vec4 *v);

/** Like L3_vec3Normalize, but doesn't perform any checks on the input vector,
	which is faster, but can be very innacurate or overflowing. You are supposed
	to provide a "nice" vector (not too big or small). */
static inline void L3_vec3NormalizeFast(L3_Vec4 *v)
{
	L3_Unit l = L3_vec3Length(*v);

	if (l == 0)
		return;

	v->x = (v->x * L3_F) / l;
	v->y = (v->y * L3_F) / l;
	v->z = (v->z * L3_F) / l;
}

L3_Unit L3_vec2Length(L3_Vec4 v);
void L3_vec3Cross(L3_Vec4 a, L3_Vec4 b, L3_Vec4 *result);
static inline L3_Unit L3_vec3Dot(L3_Vec4 a, L3_Vec4 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z) / L3_F;
}

static inline L3_Unit L3_vec2Dot_xz(L3_Vec4 a, L3_Vec4 b)
{
	return (a.x * b.x + a.z * b.z) / L3_F;
}

/** Computes a reflection direction (typically used e.g. for specular component
	in Phong illumination). The input vectors must be normalized. The result will
	be normalized as well. */
void L3_reflect(L3_Vec4 toLight, L3_Vec4 normal, L3_Vec4 *result);

/** Determines the winding of a triangle, returns 1 (CW, clockwise), -1 (CCW,
	counterclockwise) or 0 (points lie on a single line). */
static inline int8_t L3_triangleWinding(
	L3_ScreenCoord x0,
	L3_ScreenCoord y0,
	L3_ScreenCoord x1,
	L3_ScreenCoord y1,
	L3_ScreenCoord x2,
	L3_ScreenCoord y2)
{
	int32_t winding =
		(y1 - y0) * (x2 - x1) - (x1 - x0) * (y2 - y1);
		// ^ cross product for points with z == 0

	return winding > 0 ? 1 : (winding < 0 ? -1 : 0);
}

typedef struct
{
	L3_Vec4 translation;
	L3_Vec4 rotation; /**< Euler angles. Rortation is applied in this order:
													1. z = by z (roll) CW looking along z+
													2. x = by x (pitch) CW looking along x+
													3. y = by y (yaw) CW looking along y+ */
	L3_Vec4 scale;
} L3_Transform3D;

#define L3_logTransform3D(t)\
	printf("Transform3D: T = [%d %d %d], R = [%d %d %d], S = [%d %d %d]\n",\
		(t).translation.x,(t).translation.y,(t).translation.z,\
		(t).rotation.x,(t).rotation.y,(t).rotation.z,\
		(t).scale.x,(t).scale.y,(t).scale.z)

static inline void L3_transform3DInit(L3_Transform3D *t)
{
	L3_vec4Init(&(t->translation));
	L3_vec4Init(&(t->rotation));
	t->scale.x = L3_F;
	t->scale.y = L3_F;
	t->scale.z = L3_F;
	t->scale.w = 0;
}

void L3_lookAt(L3_Vec4 pointTo, L3_Transform3D *t);

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
	L3_Transform3D *t);

/** Converts rotation transformation to three direction vectors of given length
	(any one can be NULL, in which case it won't be computed). */
void L3_rotationToDirections(
	L3_Vec4 rotation,
	L3_Unit length,
	L3_Vec4 *forw,
	L3_Vec4 *right,
	L3_Vec4 *up);

/** 4x4 matrix, used mostly for 3D transforms. The indexing is this:
		matrix[column][row]. */
typedef L3_Unit L3_Mat4[4][4];

#define L3_logMat4(m)\
	printf("Mat4:\n  %d %d %d %d\n  %d %d %d %d\n  %d %d %d %d\n  %d %d %d %d\n"\
	,(m)[0][0],(m)[1][0],(m)[2][0],(m)[3][0],\
		(m)[0][1],(m)[1][1],(m)[2][1],(m)[3][1],\
		(m)[0][2],(m)[1][2],(m)[2][2],(m)[3][2],\
		(m)[0][3],(m)[1][3],(m)[2][3],(m)[3][3])

/** Initializes a 4x4 matrix to identity. */
static inline void L3_mat4Init(L3_Mat4 m)
{
	#define M(x,y) m[x][y]
	#define S L3_F

	M(0,0) = S; M(1,0) = 0; M(2,0) = 0; M(3,0) = 0;
	M(0,1) = 0; M(1,1) = S; M(2,1) = 0; M(3,1) = 0;
	M(0,2) = 0; M(1,2) = 0; M(2,2) = S; M(3,2) = 0;
	M(0,3) = 0; M(1,3) = 0; M(2,3) = 0; M(3,3) = S;

	#undef M
	#undef S
}

void L3_mat4Copy(L3_Mat4 src, L3_Mat4 dst);

void L3_mat4Transpose(L3_Mat4 m);

void L3_makeTranslationMat(
	L3_Unit offsetX,
	L3_Unit offsetY,
	L3_Unit offsetZ,
	L3_Mat4 m);

/** Makes a scaling matrix. DON'T FORGET: scale of 1.0 is set with
	L3_FRACTIONS_PER_UNIT! */
void L3_makeScaleMatrix(
	L3_Unit scaleX,
	L3_Unit scaleY,
	L3_Unit scaleZ,
	L3_Mat4 m);

/** Makes a matrix for rotation in the ZXY order. */
void L3_makeRotationMatrixZXY(
	L3_Unit byX,
	L3_Unit byY,
	L3_Unit byZ,
	L3_Mat4 m);

void L3_makeWorldMatrix(L3_Transform3D worldTransform, L3_Mat4 m);
void L3_makeCameraMatrix(L3_Transform3D cameraTransform, L3_Mat4 m);

/** Multiplies a vector by a matrix with normalization by
	L3_FRACTIONS_PER_UNIT. Result is stored in the input vector. */
void L3_vec4Xmat4(L3_Vec4 *v, L3_Mat4 m);

/** Same as L3_vec4Xmat4 but faster, because this version doesn't compute the
	W component of the result, which is usually not needed. */
void L3_vec3Xmat4(L3_Vec4 *v, L3_Mat4 m);

/** Multiplies two matrices with normalization by L3_FRACTIONS_PER_UNIT.
	Result is stored in the first matrix. The result represents a transformation
	that has the same effect as applying the transformation represented by m1 and
	then m2 (in that order). */
void L3_mat4Xmat4(L3_Mat4 m1, L3_Mat4 m2);

typedef struct
{
	L3_Unit focalLength;       /**< Defines the field of view (FOV). 0 sets an
																	orthographics projection (scale is controlled
																	with camera's scale in its transform). */
	L3_Transform3D transform;
} L3_Camera;

void L3_cameraInit(L3_Camera *camera);

typedef struct
{
	uint8_t backfaceCulling;    /**< What backface culling to use. Possible
																	values:
																	- 0 none
																	- 1 clock-wise
																	- 2 counter clock-wise */
	uint16_t visible;             /**< Can be used to easily hide the model. */
} L3_DrawConfig;

void L3_drawConfigInit(L3_DrawConfig *config);

typedef struct
{
	const L3_Unit *vertices;
	L3_Index vertexCount;
	const L3_Index *triangles;
	L3_Index triangleCount;
	const L3_Unit *triangleUVs;
	const L3_Index *triangleTextureIndex;
	const L3_COLORTYPE **triangleTextures;
	const L3_Unit *triangleTextureWidth;
	const L3_Unit *triangleTextureHeight;
} L3_Model3D;

typedef struct
{
	const L3_Unit	*texture;
	L3_Unit			height;
	L3_Unit			width;
	L3_Unit			scale;
	L3_COLORTYPE	transparency_threshold;
	/* x / 0xFF */
	uint8_t			transparency;
} L3_Billboard;


typedef struct
{
	L3_Transform3D	transform;
	L3_DrawConfig	config;
	L3_COLORTYPE	solid_color;
	union {
		const L3_Model3D		*model;
		const L3_Billboard	*billboard;
	};
} L3_Object;

typedef struct
{
	L3_ScreenCoord x;          ///< Screen X coordinate.
	L3_ScreenCoord y;          ///< Screen Y coordinate.

	L3_Unit barycentric[3]; /**< Barycentric coords correspond to the three
															vertices. These serve to locate the pixel on a
															triangle and interpolate values between its
															three points. Each one goes from 0 to
															L3_FRACTIONS_PER_UNIT (including), but due to
															rounding error may fall outside this range (you
															can use L3_correctBarycentricCoords to fix this
															for the price of some performance). The sum of
															the three coordinates will always be exactly
															L3_FRACTIONS_PER_UNIT. */
	L3_Index objectIndex;    ///< Object index within the scene.
	L3_Index triangleIndex; ///< Triangle index within the model.
	uint32_t triangleID;     /**< Unique ID of the triangle withing the whole
															scene. This can be used e.g. by a cache to
															quickly find out if a triangle has changed. */
	L3_Unit depth;         ///< Depth (only if depth is turned on).
	L3_Unit previousZ;     /**< Z-buffer value (not necessarily world depth in
															L3_Units!) that was in the z-buffer on the
															pixels position before this pixel was
															rasterized. This can be used to set the value
															back, e.g. for transparency. */
	L3_ScreenCoord triangleSize[2]; /**< Rasterized triangle width and height,
															can be used e.g. for MIP mapping. */
} L3_PixelInfo;         /**< Used to pass the info about a rasterized pixel
															(fragment) to the user-defined drawing func. */

static inline void L3_pixelInfoInit(L3_PixelInfo *p)
{
	p->x = 0;
	p->y = 0;
	p->barycentric[0] = L3_F;
	p->barycentric[1] = 0;
	p->barycentric[2] = 0;
	p->objectIndex = 0;
	p->triangleIndex = 0;
	p->triangleID = 0;
	p->depth = 0;
	p->previousZ = 0;
}

// general helper functions
static inline L3_Unit L3_abs(L3_Unit value)
{
	return value * (((value >= 0) << 1) - 1);
}

static inline L3_Unit L3_min(L3_Unit v1, L3_Unit v2)
{
	return v1 >= v2 ? v2 : v1;
}

static inline L3_Unit L3_max(L3_Unit v1, L3_Unit v2)
{
	return v1 >= v2 ? v1 : v2;
}

static inline L3_Unit L3_clamp(L3_Unit v, L3_Unit v1, L3_Unit v2)
{
	return v >= v1 ? (v <= v2 ? v : v2) : v1;
}

static inline L3_Unit L3_zeroClamp(L3_Unit value)
{
	return (value * (value >= 0));
}

static inline L3_Unit L3_wrap(L3_Unit value, L3_Unit mod)
{
	return value >= 0 ? (value % mod) : (mod + (value % mod) - 1);
}

static inline L3_Unit L3_nonZero(L3_Unit value)
{
	return (value + (value == 0));
}

L3_Unit L3_sin(L3_Unit x);
L3_Unit L3_asin(L3_Unit x);
static inline L3_Unit L3_cos(L3_Unit x)
{
	return L3_sin(x + L3_F / 4);
}

/** Corrects barycentric coordinates so that they exactly meet the defined
	conditions (each fall into <0,L3_FRACTIONS_PER_UNIT>, sum =
	L3_FRACTIONS_PER_UNIT). Note that doing this per-pixel can slow the program
	down significantly. */
static inline void L3_correctBarycentricCoords(L3_Unit barycentric[3])
{
	barycentric[0] = L3_clamp(barycentric[0],0,L3_F);
	barycentric[1] = L3_clamp(barycentric[1],0,L3_F);

	L3_Unit d = L3_F - barycentric[0] - barycentric[1];

	if (d < 0)
	{
		barycentric[0] += d;
		barycentric[2] = 0;
	}
	else
		barycentric[2] = d;
}


/** Projects a single point from 3D space to the screen space (pixels), which
	can be useful e.g. for drawing sprites. The w component of input and result
	holds the point size. If this size is 0 in the result, the sprite is outside
	the view. */
void L3_project3DPointToScreen(
	L3_Vec4 point,
	L3_Camera camera,
	L3_Vec4 *result);

/** Computes a normalized normal of given triangle. */
void L3_triangleNormal(L3_Vec4 t0, L3_Vec4 t1, L3_Vec4 t2,
	L3_Vec4 *n);

/** Helper function for retrieving per-vertex indexed values from an array,
	e.g. texturing (UV) coordinates. The 'indices' array contains three indices
	for each triangle, each index pointing into 'values' array, which contains
	the values, each one consisting of 'numComponents' components (e.g. 2 for
	UV coordinates). The three values are retrieved into 'v0', 'v1' and 'v2'
	vectors (into x, y, z and w, depending on 'numComponents'). This function is
	meant to be used per-triangle (typically from a cache), NOT per-pixel, as it
	is not as fast as possible! */
void L3_getIndexedTriangleValues(
	L3_Index triangleIndex,
	const L3_Index *indices,
	const L3_Unit *values,
	uint8_t numComponents,
	L3_Vec4 *v0,
	L3_Vec4 *v1,
	L3_Vec4 *v2);

/** Computes a normalized normal for every vertex of given model (this is
	relatively slow and SHOUDN'T be done each frame). The dst array must have a
	sufficient size preallocated! The size is: number of model vertices * 3 *
	sizeof(L3_Unit). Note that for advanced allowing sharp edges it is not
	sufficient to have per-vertex normals, but must be per-triangle. This
	function doesn't support this.

	The function computes a normal for each vertex by averaging normals of
	the triangles containing the vertex. The maximum number of these triangle
	normals that will be averaged is set with
	L3_NORMAL_COMPUTE_MAXIMUM_AVERAGE. */
void L3_computeModelNormals(L3_Object *model, L3_Unit *dst,
	int8_t transformNormals);

/** Interpolated between two values, v1 and v2, in the same ratio as t is to
	tMax. Does NOT prevent zero division. */
static inline L3_Unit L3_interpolate(L3_Unit v1, L3_Unit v2, L3_Unit t, L3_Unit tMax)
{
	return v1 + ((v2 - v1) * t) / tMax;
}

/** Like L3_interpolate, but uses a parameter that goes from 0 to
	L3_FRACTIONS_PER_UNIT - 1, which can be faster. */
static inline L3_Unit L3_interpolateByUnit(L3_Unit v1, L3_Unit v2, L3_Unit t)
{
	return v1 + ((v2 - v1) * t) / L3_F;
}

/** Same as L3_interpolateByUnit but with v1 == 0. Should be faster. */
static inline L3_Unit L3_interpolateByUnitFrom0(L3_Unit v2, L3_Unit t)
{
	return (v2 * t) / L3_F;
}
/** Same as L3_interpolate but with v1 == 0. Should be faster. */
static inline L3_Unit L3_interpolateFrom0(L3_Unit v2, L3_Unit t, L3_Unit tMax)
{
	return (v2 * t) / tMax;
}

static inline L3_Unit L3_distanceManhattan(L3_Vec4 a, L3_Vec4 b)
{
	return
		L3_abs(a.x - b.x) +
		L3_abs(a.y - b.y) +
		L3_abs(a.z - b.z);
}

/** Returns a value interpolated between the three triangle vertices based on
	barycentric coordinates. */
static inline L3_Unit L3_interpolateBarycentric(
	L3_Unit value0,
	L3_Unit value1,
	L3_Unit value2,
	L3_Unit barycentric[3])
{
	return
		(
			(value0 * barycentric[0]) +
			(value1 * barycentric[1]) +
			(value2 * barycentric[2])
		) / L3_F;
}

/** Draws a triangle according to given config. The vertices are specified in
	Screen Space space (pixels). If perspective correction is enabled, each
	vertex has to have a depth (Z position in camera space) specified in the Z
	component. */
void L3_drawTriangle(
	L3_Vec4 point0,
	L3_Vec4 point1,
	L3_Vec4 point2,
	L3_Index objectIndex,
	L3_Index triangleIndex);

/** This should be called before rendering each frame. The function clears
	buffers and does potentially other things needed for the frame. */
void L3_newFrame(void);

void L3_zBufferClear(void);
void L3_stencilBufferClear(void);

/** Writes a value (not necessarily depth! depends on the format of z-buffer)
	to z-buffer (if enabled). Does NOT check boundaries! */
void L3_zBufferWrite(L3_ScreenCoord x, L3_ScreenCoord y, L3_Unit value);

/** Reads a value (not necessarily depth! depends on the format of z-buffer)
	from z-buffer (if enabled). Does NOT check boundaries! */
L3_Unit L3_zBufferRead(L3_ScreenCoord x, L3_ScreenCoord y);

void L3_rotate2DPoint(L3_Unit *x, L3_Unit *y, L3_Unit angle);

/** Predefined vertices of a cube to simply insert in an array. These come with
		L3_CUBE_TRIANGLES and L3_CUBE_TEXCOORDS. */
#define L3_CUBE_VERTICES(m)\
/* 0 front, bottom, right */\
m/2, -m/2, -m/2,\
/* 1 front, bottom, left */\
-m/2, -m/2, -m/2,\
/* 2 front, top,    right */\
m/2,  m/2, -m/2,\
/* 3 front, top,    left */\
-m/2,  m/2, -m/2,\
/* 4 back,  bottom, right */\
m/2, -m/2,  m/2,\
/* 5 back,  bottom, left */\
-m/2, -m/2,  m/2,\
/* 6 back,  top,    right */\
m/2,  m/2,  m/2,\
/* 7 back,  top,    left */\
-m/2,  m/2,  m/2

#define L3_CUBE_VERTEX_COUNT 8

/** Predefined triangle indices of a cube, to be used with L3_CUBE_VERTICES
		and L3_CUBE_TEXCOORDS. */
#define L3_CUBE_TRIANGLES\
	3, 0, 2, /* front  */\
	1, 0, 3,\
	0, 4, 2, /* right  */\
	2, 4, 6,\
	4, 5, 6, /* back   */\
	7, 6, 5,\
	3, 7, 1, /* left   */\
	1, 7, 5,\
	6, 3, 2, /* top    */\
	7, 3, 6,\
	1, 4, 0, /* bottom */\
	5, 4, 1

#define L3_CUBE_TRIANGLE_COUNT 12

/** Predefined texture coordinates of a cube, corresponding to triangles (NOT
		vertices), to be used with L3_CUBE_VERTICES and L3_CUBE_TRIANGLES. */
#define L3_CUBE_TEXCOORDS(m)\
	0,0,  m,m,  m,0,\
	0,m,  m,m,  0,0,\
	m,m,  m,0,  0,m,\
	0,m,  m,0,  0,0,\
	m,0,  0,0,  m,m,\
	0,m,  m,m,  0,0,\
	0,0,  0,m,  m,0,\
	m,0,  0,m,  m,m,\
	0,0,  m,m,  m,0,\
	0,m,  m,m,  0,0,\
	m,0,  0,m,  m,m,\
	0,0,  0,m,  m,0

#ifndef L3_PIXEL_FUNCTION
	#error Pixel rendering function (L3_PIXEL_FUNCTION) not specified!
#endif
#ifndef L3_TRIANGLE_FUNCTION
	#error Triangle rendering function (L3_TRIANGLE_FUNCTION) not specified!
#endif
#ifndef L3_BILLBOARD_FUNCTION
	#error Billboard rendering function (L3_BILLBOARD_FUNCTION) not specified!
#endif
#ifndef L3_MODEL_FUNCTION
	#error model processing function (L3_MODEL_FUNCTION) not specified!
#endif
void L3_PIXEL_FUNCTION(L3_PixelInfo *pixel); // forward decl
int L3_TRIANGLE_FUNCTION(L3_Vec4 point0, L3_Vec4 point1, L3_Vec4 point2,
	L3_Index modelIndex, L3_Index triangleIndex);
int L3_BILLBOARD_FUNCTION(L3_Vec4 point, const L3_Object *billboard, L3_Camera camera);
int L3_MODEL_FUNCTION(const L3_Object *object);

/** Serves to accelerate linear interpolation for performance-critical
	code. Functions such as L3_interpolate require division to compute each
	interpolated value, while L3_FastLerpState only requires a division for
	the initiation and a shift for retrieving each interpolated value.

	L3_FastLerpState stores a value and a step, both scaled (shifted by
	L3_FAST_LERP_QUALITY) to increase precision. The step is being added to the
	value, which achieves the interpolation. This will only be useful for
	interpolations in which we need to get the interpolated value in every step.

	BEWARE! Shifting a negative value is undefined, so handling shifting of
	negative values has to be done cleverly. */
typedef struct
{
	L3_Unit valueScaled;
	L3_Unit stepScaled;
} L3_FastLerpState;

uint32_t L3_draw(L3_Camera camera, const L3_Object **objects, L3_Index objectCount);
void L3_clearScreen(L3_COLORTYPE color);
/* functions to clear background */
typedef L3_COLORTYPE (*L3_ClearPixFunc)(L3_Unit x, L3_Unit y);
void L3_clearScreen_with(L3_ClearPixFunc func);
void L3_plot_line(L3_COLORTYPE color, int x0, int y0, int x1, int y1);

void _L3_mapProjectedVertexToScreen(L3_Vec4 *vertex, L3_Unit focalLength);

#if L3_Z_BUFFER
int8_t L3_zTest(L3_ScreenCoord x, L3_ScreenCoord y, L3_Unit depth);
#endif

/* Instanciate a object from a original */
#define INSTANCIATE_OBJECT(name, object) L3_Object name = {	\
.model = object.model,																		\
.billboard = object.billboard,														\
.transform.scale.x = L3_F,																\
.transform.scale.y = L3_F,																\
.transform.scale.z = L3_F,																\
.transform.scale.w = 0,																		\
.transform.translation.x = 0,															\
.transform.translation.y = 0,															\
.transform.translation.z = 0,															\
.transform.translation.w = L3_F,													\
.transform.rotation.x = 0,																\
.transform.rotation.y = 0,																\
.transform.rotation.z = 0,																\
.transform.rotation.w = L3_F,															\
.config.backfaceCulling = object.config.backfaceCulling,	\
.config.visible = object.config.visible,									\
};

extern L3_COLORTYPE L3_video_buffer[L3_RESOLUTION_X * L3_RESOLUTION_Y];
#if L3_Z_BUFFER
extern L3_ZBUFTYPE L3_zBuffer[L3_MAX_PIXELS];
#endif
extern const L3_Object *engine_global_objects[L3_MAX_OBJECTS];
extern L3_Index engine_objectCount;
extern L3_Camera engine_camera;

