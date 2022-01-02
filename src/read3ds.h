/* read3ds.h */

/* Mini-library for loading .3DS files
 * 3D Studio loading code by Jare/Iguana et. al. (see read3ds.c)
 * API interface & portability mods by Straker Skunk <skunk@mit.edu>
 *
 * THIS FILE IS IN THE PUBLIC DOMAIN
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef unsigned char  uint8;	/* "byte" == 8 bits */
typedef unsigned short uint16;	/* "word" == 16 bits */
typedef unsigned int   uint32;	/* "double word" == 32 bits */


/**** Defines, macros, enums, etc. ****************************/

#ifndef TRUE
#define TRUE			1
#endif

#ifndef FALSE
#define FALSE			0
#endif

#ifndef ABS
#define ABS(a)			(((a) < 0) ? -(a) : (a))
#endif

#ifndef CLAMP
#define CLAMP(x,low,high)	(((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

/* Type conversion macros */
#define R3DS_OBJECT(x)		((r3ds_object *)(x))
#define R3DS_LIGHT(x)		((r3ds_light *)(x))
#define R3DS_CAMERA(x)		((r3ds_camera *)(x))
#define R3DS_TRIMESH(x)		((r3ds_trimesh *)(x))

/* Use to access r3ds_triangle->flags bitfield (each returns TRUE/FALSE) */
#define R3DS_TRI_AB_VIS(tri)	(((tri)->flags & 0x04) != 0)
#define R3DS_TRI_BC_VIS(tri)	(((tri)->flags & 0x02) != 0)
#define R3DS_TRI_CA_VIS(tri)	(((tri)->flags & 0x01) != 0)
#define R3DS_TRI_U_WRAP(tri)	(((tri)->flags & 0x08) != 0)
#define R3DS_TRI_V_WRAP(tri)	(((tri)->flags & 0x10) != 0)

/* Use to access r3ds_triangle->smgroups bitfield. Returns TRUE if triangle is
 * in the smoothing group specified (1-32), FALSE if otherwise. If 0 is passed,
 * return TRUE if triangle is not in ANY smoothing group, FALSE otherwise */
#define R3DS_TRI_IN_SMGROUP(tri,g) \
	(((g) <= 0) ? ((tri)->smgroups == 0) : (((tri)->smgroups & ((uint32)1 << ((g) - 1))) != 0))

/* Messages for the r3ds_build() state machine */
enum {
	R3DS_INITIALIZE,

	R3DS_PROJECT_SCALE,

	R3DS_NEW_OBJECT,
	R3DS_DEF_LIGHT,
	R3DS_DEF_CAMERA,
	R3DS_DEF_TRIMESH,

	R3DS_NUM_VERTS,
	R3DS_VERT,
	R3DS_NUM_VERT_MAPPINGS,
	R3DS_VERT_MAPPING,
	R3DS_NUM_TRIS,
	R3DS_TRI_FACE,
	R3DS_TRI_MATERIAL_CURRENT,
	R3DS_TRI_MATERIAL,
	R3DS_BEGIN_TRI_SMOOTH,
	R3DS_TRI_SMOOTH,
	R3DS_ROT_MATRIX,
	R3DS_TRANS_MATRIX,

	R3DS_NEW_MATERIAL,
	R3DS_MAT_AMBIENT_COLOR,
	R3DS_MAT_DIFFUSE_COLOR,
	R3DS_MAT_SPECULAR_COLOR,
	R3DS_MAT_2SIDED,

	R3DS_COLOR24,

	R3DS_GET_SCENE
};

/* Messages for r3ds_split_trimeshes( ) */
enum {
	R3DS_SPLIT_BY_MATERIAL,
	R3DS_SPLIT_BY_SMGROUP
};

/* Object types
 * (for r3ds_object->type field) */
typedef enum {
	R3DS_OBJ_LIGHT,
	R3DS_OBJ_CAMERA,
	R3DS_OBJ_TRIMESH,
	R3DS_OBJ_UNDEFINED
} R3dsObjType;

/* Light types
 * (for r3ds_light->ltype field) */
typedef enum {
	R3DS_LIGHT_POINT,
	R3DS_LIGHT_SPOT
} R3dsLightType;

/* Shading types
 * (for r3ds_material->shading_type field) */
typedef enum {
	R3DS_SHADE_FLAT,
	R3DS_SHADE_GOURAUD,
	R3DS_SHADE_PHONG,
	R3DS_SHADE_METAL
} R3dsShadeType;


/**** Data structures *****************************************/

/** Elements **/

/* 24-bit RGB color */
typedef struct r3ds_color24_struct r3ds_color24;
struct r3ds_color24_struct {
	/* Each component has a range of 0-255 */
	int red;
	int green;
	int blue;
};

/* Floating-point RGB color */
typedef struct r3ds_colorF_struct r3ds_colorF;
struct r3ds_colorF_struct {
	float red;
	float green;
	float blue;
};

/* Trimesh vertex */
typedef struct r3ds_point_struct r3ds_point;
struct r3ds_point_struct {
	/* Location coords. */
	float x;
	float y;
	float z;
	/* Texture mapping coords. */
	float u;
	float v;
};

/* Trimesh triangle */
typedef struct r3ds_triangle_struct r3ds_triangle;
struct r3ds_triangle_struct {
	int a, b, c;	/* Vertex indices (into r3ds_trimesh->verts) */
	uint16 flags;	/* Edge visibility & texture wrap flags */
	int mat_id;	/* Material index (into r3ds_scene->mats) */
	uint32 smgroups; /* Bitfield: smoothing group(s) membership */
};

/* Material */
/* INCOMPLETE */
typedef struct r3ds_material_struct r3ds_material;
struct r3ds_material_struct {
	char *name;
	r3ds_color24 ambient;
	r3ds_color24 diffuse;
	r3ds_color24 specular;
	R3dsShadeType shading;	/* Flat, Gouraud, Phong, or Metal */
	int two_sided : 1;	/* 2-sided material (flag) */
	int shine;		/* Shininess % */
	int shine_st;		/* Shininess strength % */
	int transp;		/* Transparency % */
	int transp_fo;		/* Transparency falloff % */
	int ref_blur;		/* Reflection blur % */
	int self_illum;		/* Self-illumination % */
	int wireframe_th;	/* Wireframe thickness */
	int texmap;		/* Texture map % */
	char *texmap_file;	/* Filename of texture map */
	int refmap;		/* Reflection map % */
	char *refmap_file;	/* Filename of reflection map */
};

/** Objects **/

/* Light object */
/* INCOMPLETE */
typedef struct r3ds_light_struct r3ds_light;
struct r3ds_light_struct {
	R3dsObjType type;	/* = R3DS_OBJ_LIGHT */
	char *name;
	R3dsLightType ltype;
	float x;
	float y;
	float z;
	r3ds_colorF color;	/* Light color */
	float radius1;		/* Inner radius */
	float radius2;		/* Outer radius */
	float mult;		/* Multiplier */
};

/* Camera object */
typedef struct r3ds_camera_struct r3ds_camera;
struct r3ds_camera_struct {
	R3dsObjType type;	/* = R3DS_OBJ_CAMERA */
	char *name;
	float x;
	float y;		/* Position */
	float z;
	float targ_x;
	float targ_y;		/* Target */
	float targ_z;
	float bank;
	float lens;		/* Lens length in mm */
};

/* Triangle mesh object */
typedef struct r3ds_trimesh_struct r3ds_trimesh;
struct r3ds_trimesh_struct {
	R3dsObjType type;	/* = R3DS_OBJ_TRIMESH */
	char *name;
	int num_verts;
	r3ds_point *verts;
	int num_tris;
	r3ds_triangle *tris;
	float rot_matrix[9];
	float trans_matrix[3];
};

/* Generalized object */
typedef struct r3ds_object_struct r3ds_object;
struct r3ds_object_struct {
	R3dsObjType type;
	char *name;
};

/** Scene **/

typedef struct r3ds_scene_struct r3ds_scene;
struct r3ds_scene_struct {
	float inches_per_unit;

	int num_objs;
	r3ds_object **objs;
	/* Note: The array above and the 3 below point to the SAME objects */
	int num_lites;
	r3ds_light **lites;
	int num_cams;
	r3ds_camera **cams;
	int num_tmeshes;
	r3ds_trimesh **tmeshes;

	int num_mats;
	r3ds_material *mats;
};


/**** Forward declarations ************************************/

/* Wrappers for the system memory management functions */
extern void *xmalloc( size_t size );
extern void *xrealloc( void *block, size_t size );
extern char *xstrdup( const char *str );
extern void xfree( void *ptr );

int r3ds_read_uint32( uint32 *data, int count, FILE *f );
int r3ds_read_uint16( uint16 *data, int count, FILE *f );
int r3ds_read_uint8( uint8 *data, int count, FILE *f );
int r3ds_read_float( float *data, int count, FILE *f );
int r3ds_file_is_3ds( const char *filename );
int r3ds_file_is_prj( const char *filename );
void r3ds_build( int data_id, const void *data );
r3ds_scene *read3ds( const char *filename );
void r3ds_dump_light( r3ds_light *light );
void r3ds_dump_camera( r3ds_camera *cam );
void r3ds_dump_trimesh( r3ds_trimesh *trimesh );
void r3ds_dump_object( r3ds_object *obj );
void r3ds_free_object( r3ds_object *obj );
void r3ds_free_material( r3ds_material *mat );
void r3ds_free_scene( r3ds_scene *scene );
int r3ds_first_smgroup( r3ds_triangle *tri );
void r3ds_rebuild_scene_objs( r3ds_scene *scene );
int r3ds_split_trimesh( r3ds_trimesh ***trimeshes_ptr, int message );
int r3ds_split_scene_trimeshes( r3ds_scene *scene, int message );
void r3ds_ChunkReader( FILE *f, int ind, int p );

/* end read3ds.h */
