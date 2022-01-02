/* read3ds.c */

/* Mini-library for loading .3DS files
 * 3D Studio loading code by Jare/Iguana et. al. (see further down for details)
 * API interface & portability mods by Straker Skunk <skunk@mit.edu>
 *
 * I wrote this interface only as far as I needed it. Much is lacking; in
 * particular, material property loading and light definitions.
 * Anyone want a go at it?
 *
 * NOTE: This code is in NO WAY connected to AutoDesk Inc. or Kinetix Inc.
 * Jare expressed concern that he might have violated AutoDesk's
 * intellectual property in releasing this 3DS interpreter; however, he
 * reverse-engineered the format in perfectly legal manner, and made use
 * of equally legitimate documentation written by others on the Internet.
 * This library contains ABSOLUTELY NO CODE from AutoDesk's own 3DS File
 * Toolkit, which I believe was even released subsequent to the authoring
 * of this interpreter.
 *
 * THIS FILE IS IN THE PUBLIC DOMAIN
 */


#include "read3ds.h"


int
r3ds_read_uint8( uint8 *data, int count, FILE *f )
{
	int n;

	n = fread( data, sizeof(uint8), count, f );
	if (n != count)
		return FALSE; /* failure */

	return TRUE; /* success */
}


int
r3ds_read_uint16( uint16 *data, int count, FILE *f )
{
	uint8 bytes[2];
	int i, rs = TRUE; /* rs == return status */

	for (i = 0; i < count; i++) {
		rs = rs && r3ds_read_uint8( bytes, 2, f );
		data[i] = ((uint16)bytes[1] << 8) | (uint16)bytes[0];
	}

	return rs;
}


int
r3ds_read_uint32( uint32 *data, int count, FILE *f )
{
	uint8 bytes[4];
	int i, rs = TRUE;

	for (i = 0; i < count; i++) {
		rs = rs && r3ds_read_uint8( bytes, 4, f );
		data[i] = ((uint32)bytes[3] << 24) | ((uint32)bytes[2] << 16) | ((uint32)bytes[1] << 8) | (uint32)bytes[0];
	}

	return rs;
}


int
r3ds_read_float( float *data, int count, FILE *f )
{
	uint32 fpdata;
	int i, rs = TRUE;

	for (i = 0; i < count; i++) {
		rs = rs && r3ds_read_uint32( &fpdata, 1, f );
		data[i] = *((float *)&fpdata);
	}

	return rs;
}


int
r3ds_file_is_3ds( const char *filename )
{
	FILE *file_3ds;
	int filesize;
	uint16 chunk_id;
	uint32 chunk_len;

	file_3ds = fopen( filename, "rb" );
	if (file_3ds == NULL)
		return 0;

	/* Get file size */
	fseek( file_3ds, 0, SEEK_END );
	filesize = ftell( file_3ds );
	fseek( file_3ds, 0, SEEK_SET );

	/* Read in main chunk header */
	r3ds_read_uint16( &chunk_id, 1, file_3ds );
	r3ds_read_uint32( &chunk_len, 1, file_3ds );
	fclose( file_3ds );

	if (chunk_id != 0x4D4D)
		return 0;

	/* Check that file size is *around* what it should be */
	if (ABS(filesize - (int)chunk_len) > 16)
		return 0;

	/* Yes, this *does* appear to be a valid 3D Studio file */
	return 1;
}


int
r3ds_file_is_prj( const char *filename )
{
	FILE *file_prj;
	int filesize;
	uint16 chunk_id;
	uint32 chunk_len;

	file_prj = fopen( filename, "rb" );
	if (file_prj == NULL)
		return 0;

	/* Get file size */
	fseek( file_prj, 0, SEEK_END );
	filesize = ftell( file_prj );
	fseek( file_prj, 0, SEEK_SET );

	/* Read in main chunk header */
	r3ds_read_uint16( &chunk_id, 1, file_prj );
	r3ds_read_uint32( &chunk_len, 1, file_prj );
	fclose( file_prj );

	if (chunk_id != 0xC23D)
		return 0;

	/* Check that file size is *around* what it should be */
	if (ABS(filesize - (int)chunk_len) > 16)
		return 0;

	/* Yes, this does appear to be a valid 3D Studio project file */
	return 1;
}


void
r3ds_build( int data_id, const void *data )
{
	static r3ds_object **objects;
	static r3ds_light **lights;
	static r3ds_camera **cameras;
	static r3ds_trimesh **trimeshes;
	static r3ds_material *materials;
	static r3ds_object *obj;
	static r3ds_light *lite; /* A better abbreviation, anyone? */
	static r3ds_camera *cam;
	static r3ds_trimesh *tmesh;
	static r3ds_material *mat;
	static r3ds_color24 *color24;
	static r3ds_color24 default_color24 = { 128, 128, 128 };
	static float inches_per_unit;
	static int num_objects;
	static int num_lights;
	static int num_cameras;
	static int num_trimeshes;
	static int num_materials;
	static int vert_alloc = 0;
	static int tri_alloc = 0;
	static int cur_vert = 0;
	static int cur_tri = 0;
	static int cur_mat;
	r3ds_scene *scene;
	r3ds_triangle *tri;
	r3ds_point *vert;
	uint32 *dwordp;
	float *floatp;
	int *intp;
	int i;
	char *name;

	switch (data_id) {
	case R3DS_INITIALIZE:
		inches_per_unit = 1.0;
		objects = NULL;
		lights = NULL;
		cameras = NULL;
		trimeshes = NULL;
		materials = NULL;
		color24 = NULL;
		num_objects = 0;
		num_lights = 0;
		num_cameras = 0;
		num_trimeshes = 0;
		num_materials = 0;
		break;

	case R3DS_PROJECT_SCALE:
		floatp = (float *)data;
		inches_per_unit = *floatp;
		break;

		/* Object definition input
		 * (to be followed by light/camera/trimesh def. input */

	case R3DS_NEW_OBJECT:
		name = (char *)data;
		++num_objects;
		objects = xrealloc( objects, num_objects * sizeof(r3ds_object *) );
		obj = xmalloc( sizeof(r3ds_object) );
		obj->type = R3DS_OBJ_UNDEFINED;
		obj->name = xstrdup( name );
		objects[num_objects - 1] = obj;
#ifdef R3DS_VERBOSE
		printf( "\n\nNew object: \"%s\"\n", name );
#endif
		break;

		/* Light definition input */
		/* INCOMPLETE */

	case R3DS_DEF_LIGHT:
		++num_lights;
		lights = xrealloc( lights, num_lights * sizeof(r3ds_light *) );
		lite = xrealloc( obj, sizeof(r3ds_light) );
		lite->type = R3DS_OBJ_LIGHT;
		objects[num_objects - 1] = R3DS_OBJECT(lite);
		lights[num_lights - 1] = lite;
#ifdef R3DS_VERBOSE
		printf( "Light\n" );
#endif
		break;

		/* Camera definiton input */

	case R3DS_DEF_CAMERA:
		floatp = (float *)data;
		++num_cameras;
		cameras = xrealloc( cameras, num_cameras * sizeof(r3ds_camera *) );
		cam = xrealloc( obj, sizeof(r3ds_camera) );
		cam->type = R3DS_OBJ_CAMERA;
		cam->x = floatp[0];
		cam->y = floatp[1];
		cam->z = floatp[2];
		cam->targ_x = floatp[3];
		cam->targ_y = floatp[4];
		cam->targ_z = floatp[5];
		cam->bank = floatp[6];
		cam->lens = floatp[7];
		objects[num_objects - 1] = R3DS_OBJECT(cam);
		cameras[num_cameras - 1] = cam;
#ifdef R3DS_VERBOSE
		printf( "Camera\n" );
#endif
		break;

		/* Triangle mesh definition input */

	case R3DS_DEF_TRIMESH:
		++num_trimeshes;
		trimeshes = xrealloc( trimeshes, num_trimeshes * sizeof(r3ds_trimesh *) );
		tmesh = xrealloc( obj, sizeof(r3ds_trimesh) );
		tmesh->type = R3DS_OBJ_TRIMESH;
		tmesh->num_verts = 0;
		tmesh->num_tris = 0;
		objects[num_objects - 1] = R3DS_OBJECT(tmesh);
		trimeshes[num_trimeshes - 1] = tmesh;
		vert_alloc = 0;
		cur_vert = 0;
		tri_alloc = 0;
		cur_tri = 0;
		cur_mat = -1;
#ifdef R3DS_VERBOSE
		printf( "Triangle mesh\n" );
#endif
		break;

	case R3DS_NUM_VERTS:
		i = *((int *)data);
		vert_alloc = i;
		tmesh->num_verts = i;
		if (i > 0)
			tmesh->verts = xmalloc( i * sizeof(r3ds_point) );
		for (i = 0; i < tmesh->num_verts; i++) {
			vert = &tmesh->verts[i];
			vert->x = 0.0;
			vert->y = 0.0;
			vert->z = 0.0;
			vert->u = 0.0;
			vert->v = 0.0;
		}
#ifdef R3DS_VERBOSE
		printf( "\tBegin %d vertices\n", tmesh->num_verts );
#endif
		break;

	case R3DS_VERT:
		floatp = (float *)data;
		if (cur_vert >= vert_alloc)
			break;
		vert = &tmesh->verts[cur_vert];
		vert->x = floatp[0];
		vert->y = floatp[1];
		vert->z = floatp[2];
#ifdef R3DS_VERBOSE
		printf( "\t\tVertex %d: (%.2f, %.2f, %.2f)\n", cur_vert, floatp[0], floatp[1], floatp[2] );
#endif
		++cur_vert;
		break;

	case R3DS_NUM_VERT_MAPPINGS:
		cur_vert = 0;
#ifdef R3DS_VERBOSE
		printf( "\n" );
#endif
		break;

	case R3DS_VERT_MAPPING:
		floatp = (float *)data;
		if (cur_vert >= vert_alloc)
			break;
		vert = &tmesh->verts[cur_vert];
		vert->u = floatp[0];
		vert->v = floatp[1];
#ifdef R3DS_VERBOSE
		printf( "\t\tMapping %d: (%.2f, %.2f)\n", cur_vert, floatp[0], floatp[1] );
#endif
		++cur_vert;
		break;

	case R3DS_NUM_TRIS:
		i = *((int *)data);
		tri_alloc = i;
		tmesh->num_tris = i;
		if (i > 0)
			tmesh->tris = xmalloc( i * sizeof(r3ds_triangle) );
		for (i = 0; i < tmesh->num_tris; i++) {
			tri = &tmesh->tris[i];
			tri->a = 0.0;
			tri->b = 0.0;
			tri->c = 0.0;
			tri->flags = 0;
			tri->mat_id = -1;
			tri->smgroups = 0;
		}
#ifdef R3DS_VERBOSE
		printf( "\tBegin %d triangles\n", tmesh->num_tris );
#endif
		break;

	case R3DS_TRI_FACE:
		intp = (int *)data;
		if (cur_tri >= tri_alloc)
			break;
		tri = &tmesh->tris[cur_tri];
		tri->a = CLAMP(intp[0], 0, vert_alloc - 1);
		tri->b = CLAMP(intp[1], 0, vert_alloc - 1);
		tri->c = CLAMP(intp[2], 0, vert_alloc - 1);
		tri->flags = (uint16)intp[3];
#ifdef R3DS_VERBOSE
		printf( "\t\tTriangle %d: %d--%d--%d    Flags=0x%X\n", cur_tri, wordp[0], wordp[1], wordp[2], wordp[3] );
#endif
		++cur_tri;
		break;

	case R3DS_TRI_MATERIAL_CURRENT:
		name = (char *)data;
		cur_mat = -1; /* default material */
		/* Find material index */
		for (i = 0; i < num_materials; i++)
			if (!strcmp( name, materials[i].name )) {
				cur_mat = i;
				break;
			}
#ifdef R3DS_VERBOSE
		printf( "\tBegin triangles with material: \"%s\" (%d)\n", name, cur_mat );
#endif
		break;

	case R3DS_TRI_MATERIAL:
		i = *((int *)data);
		if ((i < 0) || (i >= tri_alloc))
			break;
		tmesh->tris[i].mat_id = cur_mat;
#ifdef R3DS_VERBOSE
		printf( "\t\tTriangle %d\n", i );
#endif
		break;

	case R3DS_BEGIN_TRI_SMOOTH:
		cur_tri = 0;
#ifdef R3DS_VERBOSE
		printf( "\tBegin smoothing group definitions\n" );
#endif
		break;

	case R3DS_TRI_SMOOTH:
		dwordp = (uint32 *)data;
		if (cur_tri >= tri_alloc)
			break;
		tmesh->tris[cur_tri].smgroups = *dwordp;
#ifdef R3DS_VERBOSE
		printf( "\t\tTriangle %d:", cur_tri );
		if (R3DS_TRI_IN_SMGROUP(&tmesh->tris[cur_tri],0))
			printf( " none" );
		else
			for (i = 1; i <= 32; i++)
				if (R3DS_TRI_IN_SMGROUP(&tmesh->tris[cur_tri],i))
					printf( " %d", i );
		printf( "\n" );
#endif
		++cur_tri;
		break;

	case R3DS_ROT_MATRIX:
		floatp = (float *)data;
		for (i = 0; i < 9; i++)
			tmesh->rot_matrix[i] = floatp[i];
		break;

	case R3DS_TRANS_MATRIX:
		floatp = (float *)data;
		for (i = 0; i < 3; i++)
			tmesh->trans_matrix[i] = floatp[i];
		break;

		/* Material definition input */

	case R3DS_NEW_MATERIAL:
		name = (char *)data;
		++num_materials;
		materials = xrealloc( materials, num_materials * sizeof(r3ds_material) );
		mat = &materials[num_materials - 1];
		mat->name = xstrdup( name );
		memcpy( &mat->ambient, &default_color24, sizeof(r3ds_color24) );
		memcpy( &mat->diffuse, &default_color24, sizeof(r3ds_color24) );
		memcpy( &mat->specular, &default_color24, sizeof(r3ds_color24) );
		mat->two_sided = 0;
		mat->texmap_file = NULL;
		mat->refmap_file = NULL;
#ifdef R3DS_VERBOSE
		printf( "\nNew material: \"%s\" (%d)\n", name, num_materials - 1 );
#endif
		break;

	case R3DS_MAT_AMBIENT_COLOR:
		color24 = &mat->ambient;
#ifdef R3DS_VERBOSE
		printf( "\tAmbient color: " );
#endif
		break;

	case R3DS_MAT_DIFFUSE_COLOR:
		color24 = &mat->diffuse;
#ifdef R3DS_VERBOSE
		printf( "\tDiffuse color: " );
#endif
		break;

	case R3DS_MAT_SPECULAR_COLOR:
		color24 = &mat->specular;
#ifdef R3DS_VERBOSE
		printf( "\tSpecular color: " );
#endif
		break;

	case R3DS_MAT_2SIDED:
		mat->two_sided = 1;
#ifdef R3DS_VERBOSE
		printf( "\tMaterial is 2-sided\n" );
#endif
		break;

	case R3DS_COLOR24:
		intp = (int *)data;
		if (color24 != NULL) {
			color24->red = CLAMP(intp[0], 0, 255);
			color24->green = CLAMP(intp[1], 0, 255);
			color24->blue = CLAMP(intp[2], 0, 255);
#ifdef R3DS_VERBOSE
			printf( "(%d, %d, %d)\n", color24->red, color24->green, color24->blue );
#endif
		}
		color24 = NULL;
		break;

		/* Output */

	case R3DS_GET_SCENE:
		scene = (r3ds_scene *)data;
		scene->inches_per_unit = inches_per_unit;
		scene->num_objs = num_objects;
		scene->objs = objects;
		scene->num_lites = num_lights;
		scene->lites = lights;
		scene->num_cams = num_cameras;
		scene->cams = cameras;
		scene->num_tmeshes = num_trimeshes;
		scene->tmeshes = trimeshes;
		scene->num_mats = num_materials;
		scene->mats = materials;
		break;
	}

#ifdef R3DS_VERBOSE
	fflush( stdout );
#endif
}


r3ds_scene *
read3ds( const char *filename )
{
	r3ds_scene *scene;
	FILE *file_3ds;
	int filesize;

	file_3ds = fopen( filename, "rb" );
	if (file_3ds == NULL)
		return NULL;

	/* Get file size */
	fseek( file_3ds, 0, SEEK_END );
	filesize = ftell( file_3ds );
	fseek( file_3ds, 0, SEEK_SET );

	/* Read file */
	r3ds_build( R3DS_INITIALIZE, NULL );
	r3ds_ChunkReader( file_3ds, 0, filesize );
	fclose( file_3ds );

	scene = xmalloc( sizeof(r3ds_scene) );
	r3ds_build( R3DS_GET_SCENE, scene );

	return scene;
}


/* Verbosely describe a light */
void
r3ds_dump_light( r3ds_light *light )
{
	printf( "Light \"%s\"\n", light->name );
}


/* Verbosely describe a camera */
void
r3ds_dump_camera( r3ds_camera *cam )
{
	printf( "Camera \"%s\"\n", cam->name );
	printf( "    Location: (%.3f, %.3f, %.3f)\n", cam->x, cam->y, cam->z );
	printf( "    Target: (%.3f, %.3f, %.3f)\n", cam->targ_x, cam->targ_y, cam->targ_z );
	printf( "    Bank: %.2f    Lens: %.2f\n", cam->bank, cam->lens );
}


/* Verbosely describe a triangle mesh */
void
r3ds_dump_trimesh( r3ds_trimesh *trimesh )
{
	r3ds_point *vert;
	r3ds_triangle *tri;
	int i, g;

	printf( "Triangle mesh \"%s\"\n", trimesh->name );

	/* Vertices */
	printf( "    %d vertices\n", trimesh->num_verts );
	for (i = 0; i < trimesh->num_verts; i++) {
		vert = &trimesh->verts[i];
		printf( "\tVert #%d:  XYZ=(%.3f, %.3f, %.3f)", i, vert->x, vert->y, vert->z );
		printf( "  UV=(%.3f, %.3f)\n", vert->u, vert->v );
	}

	/* Triangles */
	printf( "    %d triangles\n", trimesh->num_tris );
	for (i = 0; i < trimesh->num_tris; i++) {
		tri = &trimesh->tris[i];
		printf( "\tTri #%d:  %d ", i, tri->a );
		if (R3DS_TRI_AB_VIS(tri))
			printf( "+" );
		else
			printf( "-" );
		printf( " %d ", tri->b );
		if (R3DS_TRI_BC_VIS(tri))
			printf( "+" );
		else
			printf( "-" );
		printf( " %d ", tri->c );
		if (R3DS_TRI_CA_VIS(tri))
			printf( "+" );
		else
			printf( "-" );
		if (R3DS_TRI_U_WRAP(tri))
			printf( "  Uwrap" );
		if (R3DS_TRI_V_WRAP(tri))
			printf( "  Vwrap" );

		/* Smoothing groups */
		printf( "    SmGrps:" );
		if (R3DS_TRI_IN_SMGROUP(tri,0))
			printf( " none" );
		else {
			for (g = 1; g <= 32; g++)
				if (R3DS_TRI_IN_SMGROUP(tri,g))
					printf( " %d", g );
		}
		printf( "\n" );
	}

	/* Rotation matrix */
	printf( "    Coordinate system alignment" );
	for (i = 0; i < 9; i++) {
		if ((i % 3) == 0)
			printf( "\n\t" );
		printf( "% .6f  ", trimesh->rot_matrix[i] );
	}

	/* Translation matrix */
	printf( "\n    Coordinate system origin\n\t" );
	for (i = 0; i < 3; i++)
		printf( "% .6f  ", trimesh->trans_matrix[i] );
	printf( "\n" );

	fflush( stdout );
}


/* Verbosely describe an object, whatever it may be */
void
r3ds_dump_object( r3ds_object *obj )
{
	printf( "\n" );
	switch (obj->type) {
	case R3DS_OBJ_LIGHT:
		r3ds_dump_light( R3DS_LIGHT(obj) );
		break;

	case R3DS_OBJ_CAMERA:
		r3ds_dump_camera( R3DS_CAMERA(obj) );
		break;

	case R3DS_OBJ_TRIMESH:
		r3ds_dump_trimesh( R3DS_TRIMESH(obj) );
		break;

	case R3DS_OBJ_UNDEFINED:
		printf( "Object \"%s\" - undefined\n", obj->name );
		break;

	default:
		printf( "Error: invalid r3ds_object\n" );
		break;
	}
}


/* Deallocates an object */
void
r3ds_free_object( r3ds_object *obj )
{
	r3ds_light *light;
	r3ds_camera *cam;
	r3ds_trimesh *tmesh;

	switch (obj->type) {
	case R3DS_OBJ_LIGHT:
		light = R3DS_LIGHT(obj);
		xfree( light->name );
		xfree( light );
		break;

	case R3DS_OBJ_CAMERA:
		cam = R3DS_CAMERA(obj);
		xfree( cam->name );
		xfree( cam );
		break;

	case R3DS_OBJ_TRIMESH:
		tmesh = R3DS_TRIMESH(obj);
		xfree( tmesh->name );
		if (tmesh->num_verts > 0)
			xfree( tmesh->verts );
		if (tmesh->num_tris > 0)
			xfree( tmesh->tris );
		xfree( tmesh );
		break;

	case R3DS_OBJ_UNDEFINED:
		xfree( obj->name );
		xfree( obj );
		break;

#ifdef R3DS_DEBUG
	default:
		printf( "ERROR: attempted to free invalid r3ds_object\n" );
		fflush( stdout );
		break;
#endif
	}
}


/* Deallocates a material */
void
r3ds_free_material( r3ds_material *mat )
{
	xfree( mat->name );
	if (mat->texmap_file != NULL)
		xfree( mat->texmap_file );
	if (mat->refmap_file != NULL)
		xfree( mat->refmap_file );
}


/* Deallocates an entire scene-- i.e. EVERYTHING */
void
r3ds_free_scene( r3ds_scene *scene )
{
	int i;

	/* Free scene objects */
	for (i = 0; i < scene->num_objs; i++)
		r3ds_free_object( scene->objs[i] );
	/* Free scene materials */
	for (i = 0; i < scene->num_mats; i++)
		r3ds_free_material( &scene->mats[i] );
	/* Free pointer arrays */
	if (scene->num_objs > 0)
		xfree( scene->objs );
	if (scene->num_lites > 0)
		xfree( scene->lites );
	if (scene->num_cams > 0)
		xfree( scene->cams );
	if (scene->num_tmeshes > 0)
		xfree( scene->tmeshes );
	if (scene->num_mats > 0)
		xfree( scene->mats );
	/* Free scene itself */
	xfree( scene );
}


/* Pass this a triangle, and it will return the number of the FIRST smoothing
 * group that triangle is in (0 if not in any group) */
int r3ds_first_smgroup( r3ds_triangle *tri )
{
	int g;

	for (g = 1; g <= 32; g++)
		if (R3DS_TRI_IN_SMGROUP(tri,g))
			return g;

	return 0;
}


/* Internal function-- rebuilds the r3ds_scene->objs array from the
 * separate ->lites, ->cams and ->tmeshes arrays */
void
r3ds_rebuild_scene_objs( r3ds_scene *scene )
{
	int n,o;

	n = scene->num_lites + scene->num_cams + scene->num_tmeshes;
	scene->num_objs = n;
	scene->objs = xrealloc( scene->objs, n * sizeof(r3ds_object *) );
	n = 0;
	for (o = 0; o < scene->num_lites; o++)
		scene->objs[n++] = R3DS_OBJECT(scene->lites[o]);
	for (o = 0; o < scene->num_cams; o++)
		scene->objs[n++] = R3DS_OBJECT(scene->cams[o]);
	for (o = 0; o < scene->num_tmeshes; o++)
		scene->objs[n++] = R3DS_OBJECT(scene->tmeshes[o]);
}


/* This function is mostly meant for use by r3ds_split_trimeshes( ); it takes a
 * pointer to an array of one trimesh, splits up the trimesh by smoothing groups
 * (message == -1) or materials (message == num_mats), places all the new
 * trimeshes into the array and returns the quantity created (or -1 on error)
 * Note: original trimesh always gets freed */
int
r3ds_split_trimesh( r3ds_trimesh ***trimeshes_ptr, int message )
{
	r3ds_trimesh **group_tmeshes;
	r3ds_trimesh *orig_tmesh, *tmesh;
	r3ds_triangle *orig_tri, *tri;
	int num_groups;
	int num_new_tmeshes = 0;
	int *tri_counts;
	int *index_remap;
	int *vertex_remap;
	char name_suffix[16];
	int len;
	int g, t, v, n;

	/* Create group array, triangle counters */
	if (message == -1) /* R3DS_SPLIT_BY_SMGROUP */
		num_groups = 33; /* Null group + smoothing groups 1-32 */
	else
		num_groups = message + 1; /* # of materials + default finish */
	group_tmeshes = xmalloc( num_groups * sizeof(r3ds_trimesh *) );
	tri_counts = xmalloc( num_groups * sizeof(int) );

	orig_tmesh = *trimeshes_ptr[0];

	/* See how many triangles are in each potential sub-trimesh group */
	for (g = 0; g < num_groups; g++)
		tri_counts[g] = 0; /* initialize counters first */

	for (t = 0; t < orig_tmesh->num_tris; t++) {
		orig_tri = &orig_tmesh->tris[t];
		if (message == -1) /* R3DS_SPLIT_BY_SMGROUP */
			g = r3ds_first_smgroup( orig_tri );
		else {
			g = orig_tri->mat_id;
			if (g == -1) /* default material */
				g += num_groups;
		}
		++tri_counts[g];
	}

	/* For each non-empty group, create a new sub-trimesh */
	for (g = 0; g < num_groups; g++) {
		n = tri_counts[g];
		if (n == 0) {
			group_tmeshes[g] = NULL;
			continue;
		}

		++num_new_tmeshes;

		/* tmesh = new sub-trimesh */
		tmesh = xmalloc( sizeof(r3ds_trimesh) );
		if (message == -1) /* R3DS_SPLIT_BY_SMGROUP */
			sprintf( name_suffix, " (smgrp %d)", g );
		else {
			if (g < (num_groups - 1))
				sprintf( name_suffix, " (mat=%d)", g );
			else
				sprintf( name_suffix, " (mat=default)" );
		}
		len = strlen( orig_tmesh->name ) + strlen( name_suffix ) + 1;
		tmesh->name = xmalloc( len * sizeof(char) );
		strcpy( tmesh->name, orig_tmesh->name );
		strcat( tmesh->name, name_suffix );
		tmesh->type = R3DS_OBJ_TRIMESH;
		tmesh->num_tris = n;
		tmesh->tris = xmalloc( n * sizeof(r3ds_triangle) );
		group_tmeshes[g] = tmesh;
	}

	/* Re-init triangle counters, need them for something else now */
	for (g = 0; g < num_groups; g++)
		tri_counts[g] = 0;

	/* Copy each triangle into the appropriate sub-trimesh */
	for (t = 0; t < orig_tmesh->num_tris; t++) {
		orig_tri = &orig_tmesh->tris[t];
		if (message == -1) /* R3DS_SPLIT_BY_SMGROUP */
			g = r3ds_first_smgroup( orig_tri );
		else {
			g = orig_tri->mat_id;
			if (g == -1) /* default material */
				g += num_groups;
		}
		tmesh = group_tmeshes[g];
		n = tri_counts[g]++;
		tri = &tmesh->tris[n];
		memcpy( tri, orig_tri, sizeof(r3ds_triangle) );
		if (message == -1) { /* R3DS_SPLIT_BY_SMGROUP */
			/* Set new smoothing groups property */
			if (g == 0)
				tri->smgroups = 0;
			else
				tri->smgroups = (uint32)1 << (g - 1);
		}
	}

	/* Done with triangle counters */
	xfree( tri_counts );

	/* Allocate vertex/index remapping tables (pseudo-functions,
	 * each the inverse of the other) */
	n = orig_tmesh->num_verts;
	if (n > 0) {
		vertex_remap = xmalloc( n * sizeof(int) );
		index_remap = xmalloc( n * sizeof(int) );
	}
	else {
		vertex_remap = NULL;
		index_remap = NULL;
	}

	/* Get vertices and remapped indices into each sub-trimesh */
	for (g = 0; g < num_groups; g++) {
		tmesh = group_tmeshes[g];
		if (tmesh == NULL)
			continue;

		/* Initialize index remap table for this object
		 * (vertex_remap doesn't need this) */
		for (v = 0; v < orig_tmesh->num_verts; v++)
			index_remap[v] = -1;

		/* Build tables */
		v = 0;
		for (t = 0; t < tmesh->num_tris; t++) {
			tri = &tmesh->tris[t];
			if (index_remap[tri->a] == -1) {
				vertex_remap[v] = tri->a;
				index_remap[tri->a] = v++;
			}
			if (index_remap[tri->b] == -1) {
				vertex_remap[v] = tri->b;
				index_remap[tri->b] = v++;
			}
			if (index_remap[tri->c] == -1) {
				vertex_remap[v] = tri->c;
				index_remap[tri->c] = v++;
			}
		}
		tmesh->num_verts = v;
		if (v > 0)
			tmesh->verts = xmalloc( v * sizeof(r3ds_point) );

		/* Copy the vertices needed */
		for (v = 0; v < tmesh->num_verts; v++) {
			n = vertex_remap[v];
			memcpy( &tmesh->verts[v], &orig_tmesh->verts[n], sizeof(r3ds_point) );
		}

		/* And lastly, remap the indices */
		for (t = 0; t < tmesh->num_tris; t++) {
			tri = &tmesh->tris[t];
			tri->a = index_remap[tri->a];
			tri->b = index_remap[tri->b];
			tri->c = index_remap[tri->c];
		}
	}

	/* Done with these */
	if (vertex_remap != NULL) {
		xfree( vertex_remap );
		xfree( index_remap );
	}
	r3ds_free_object( R3DS_OBJECT(orig_tmesh) );

	/* Place the new sub-trimeshes in the original array */
	n = num_new_tmeshes;
	if (n > 1)
		*trimeshes_ptr = xrealloc( *trimeshes_ptr, n * sizeof(r3ds_trimesh *) );
	if (n == 0)
		xfree( *trimeshes_ptr ); /* orig_tmesh was a dud :( */
	n = 0;
	for (g = 0; g < num_groups; g++) {
		tmesh = group_tmeshes[g];
		if (tmesh != NULL) {
			(*trimeshes_ptr)[n++] = tmesh;
		}
	}
	xfree( group_tmeshes );

	return num_new_tmeshes;
}


/* This runs through all the r3ds_trimeshes in the given scene and breaks each
 * down into smaller trimeshes, either on the basis of material (message ==
 * R3DS_SPLIT_BY_MATERIAL) or lowest-numbered smoothing group (message ==
 * R3DS_SPLIT_BY_SMGROUP)
 * Useful if you want to import 3DS geometry into a property-per-vertex 3D
 * system like OpenGL (which is exactly the reason I wrote this :) */
int
r3ds_split_scene_trimeshes( r3ds_scene *scene, int message )
{
	r3ds_trimesh **new_tmeshes = NULL;
	r3ds_trimesh **tmeshes;
	int num_new_tmeshes = 0;
	int n, o;

	for (o = 0; o < scene->num_tmeshes; o++) {
		tmeshes = xmalloc( sizeof(r3ds_trimesh *) );
		tmeshes[0] = scene->tmeshes[o];
		switch (message) {
		case R3DS_SPLIT_BY_MATERIAL:
			n = r3ds_split_trimesh( &tmeshes, scene->num_mats );
			break;

		case R3DS_SPLIT_BY_SMGROUP:
			n = r3ds_split_trimesh( &tmeshes, -1 );
			break;

		default:
			return -1;
		}
		num_new_tmeshes += n;
		if (n > 0) {
			new_tmeshes = xrealloc( new_tmeshes, num_new_tmeshes * sizeof(r3ds_trimesh *) );
			memcpy( &new_tmeshes[num_new_tmeshes - n], tmeshes, n * sizeof(r3ds_trimesh *) );
			xfree( tmeshes );
		}
	}
	xfree( scene->tmeshes );
	scene->num_tmeshes = num_new_tmeshes;
	scene->tmeshes = new_tmeshes;
	r3ds_rebuild_scene_objs( scene );

	return num_new_tmeshes;
}




/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




/* This next part is what does the down and dirty file reading. It is a modified
 * version of the 3DS reader by Jare/Iguana et. al. (see below for details).
 *
 * Main changes:
 *
 * 1) Replaced all the fread()'s with read_uintXX()/read_float() calls, to
 * allow explicit little-endian byte ordering. Meaning that file I/O is now
 * pretty much endian-agnostic.
 *
 * 2) Got rid of the #pragma pack directive, and changed the code to read in
 * the chunk header fields separately instead of the whole thing at once
 * (the unaligned double word TChunkHeader->len was giving bus errors when
 * accessed on MIPS hardware)
 *
 * 3) Added hooks for the r3ds_build() state machine all over the place.
 * This required adding a few extra XXXXXXReader() routines to generate some
 * additional control signals, but no big deal.
 *
 * 4) Enclosed all the printf()'s with #ifdef's, so they can be compiled out.
 * However, they can just as easily be compiled in, and their output compared
 * to that of the original reference code. As far as I've seen, the outputs
 * from both the modified and unmodified readers are identical. (yes!)
 *
 * 5) Formatting: Changed all the C++ //-style comments to slash-star C kind
 * (cc complains about the former) and ran the code through astyle as the
 * 4-space indents were getting on my nerves :-)
 *
 * 6) Oh, and added the all-important project scale chunk (CHUNK_SCALE).
 *
 * The code below has been successfully tested on SGI MIPS, Sun Sparc,
 * and Intel x86 hardware. (Alpha anyone?)
 *
 * Any further potential portability issues should be resolvable in the
 * uintXX typedefs, I think. BTW, if you're using libtiff, you'll have to get
 * rid of said declarations, else namespaces will collide.
 *
 * Because Jare and company have generously donated their code to the public
 * domain, so do I, in the hope that it will be useful to others. (Disclaimer:
 * There is no warranty, I assume no liability, use at your own risk, etc.)
 * Other than that, enjoy, and happy parsing!
 *
 *
 * --Straker
 * <skunk@mit.edu>
 *
 *
 */


/* --------------------------- 3DSRDR.C -------------------------------
 
    .3DS file format exerciser v1.4
    by Bezzegh D‚nes, aka. DiVeR
    e-mail: diver@inf.bme.hu
    http://www.inf.bme.hu/~diver
 
    If you are not interested in materials,
    then there is not much new for you. :-(
 
    Changes since v1.3:
 
    - Fixed SmooListReader's bug when group>15.
    - New data-chunk type: WORD /CHUNK_WRD/
    - New Material chunks:
	CHUNK_REFMAP	      CHUNK_REFMASK	     CHUNK_SHININESS
	CHUNK_SPECULARMAP     CHUNK_SPECULARMASK     CHUNK_SHSTRENGTH
	CHUNK_OPACITYMAP      CHUNK_OPACITYMASK      CHUNK_TRANSP
	CHUNK_TEXTURE2	      CHUNK_TEXTURE2MASK     CHUNK_TRFALLOFF
	CHUNK_SHININESSMAP    CHUNK_SHININESSMASK    CHUNK_REFLECTBL
	CHUNK_SELFILLUMMAP    CHUNK_SELFILLUMMASK    CHUNK_SELFILLUM
	CHUNK_TEXTUREMASK     CHUNK_BLUR	     CHUNK_SHADETYPE
	CHUNK_BUMPMASK	      CHUNK_TROTATION	     CHUNK_FLAG2SIDE
	CHUNK_USCALE	      CHUNK_VSCALE	     CHUNK_TFLAGS
	CHUNK_UOFFSET	      CHUNK_VOFFSET
	CHUNK_BLACKTINT       CHUNK_WHITETINT
	CHUNK_RTINT	      CHUNK_GTINT	     CHUNK_BTINT
    - New "not so useful" Material chunks:
	CHUNK_FACEMAP	      CHUNK_WIREFRAME	     CHUNK_WIRETHICK
	CHUNK_SOFTEN	      CHUNK_FLAGIN	     CHUNK_FLAGUNIT
	CHUNK_FLAGADD
    - New Light chunks:
	CHUNK_LIGHTR1	      CHUNK_LIGHTR2	     CHUNK_LIGHTMUL
    - Added CHUNK_SHP and CHUNK_LFT. Why not?
    - That's all!
 
    Did I leave something from material out?
 
   ------------------------------ v1.3 --------------------------------
 
    .3DS file format exerciser v1.3.
    by Mats Byggmastar, aka. MRI/Doomsday
    e-mail: mri@penti.sit.fi
 
    All credits goes to Jare/Iguana as this is just a modified
    version of v1.2 he released.
 
    You can use the following method to dig up new data:
	- Make a simple .3ds file and convert it with this reader
	  using -dump mode.
	- Go back into 3D Studio and add that little feature to the
	  scene and convert the .3ds file again using -dump mode.
	- Compare the output from both conversions and you'll
	  find the chunk where the data was modified or added.
 
    "The Unofficial 3dStudio 3DS File Format v1.0" by Jeff Lewis
    is also a great help. I found that file at:
 
	  www.viewpoint.com/avalon/format_specs.html
 
 
    .Mats Byggmastar  15.2.1997  Jakobstad, Finland
 
 
   --------------------------- 3DSRDR.C -------------------------------
 
    .3DS file format exerciser v1.2.
    Written by Javier Arevalo, AKA Jare/Iguana.
 
    I compile this with Watcom/32, but I guess it should work with
	any compiler and OS combination for which the typedefs are
	valid i.e. any that I know for PCs... Try it and see.
	Oh, and also check the #pragma pack() thing.
 
    - DISCLAIMER -
 
    I hope I have not broken any patents or trade secrets by releasing
	this info. This is purely a mind exercise to break into a file
	format that is quite useful to know. As far as I have been told
	a file format is not subject to anything such as copyright or
	patent, so I have done this because I believe I'm allowed to.
 
    I PLACE THIS FILE IN THE PUBLIC DOMAIN, SO EVERYTHING CONTAINED HERE
	IS TOTALLY FREE FOR YOU TO EXPLORE AND USE. I DISCLAIM ANY AND ALL
	EVENTS COMING OUT OF ANY POSSIBLE USE (OR LACK OF USE) OR EXISTANCE
	OF THIS FILE. I WON'T BE LIABLE FOR ANYTHING RELATED TO THIS FILE,
	OR ANY PRIOR OR FUTURE VERSION OF IT.
 
    All trademarks mentioned are property of their respective holders.
 
    - Merits -
 
    Heavily based on info on the file 3DS_08.TXT by Jim Pitts
      (jp5@ukc.ac.uk)
 
    Basic material-related stuff digged up by Jare.
    Track info stuff too.
 
    Thanks to the Egerter brothers of WGT fame and to Walken/Impact studios
	for extra info, Rex Deathstar for support. And definitely to
	Xanthome/Darkzone for you know why. And of course, respect to
	Avatar/Legend Design for being here before all of us.
 
    For a cool example of actual reading of 3DS files, look no
	further than 3DSCO20.ZIP by Mats Byggmastar aka. MRI. I
	personally prefer using a table-driven modification of this
	code, but both approaches are quite ok and his is much faster
	to write and follow.
 
    Now only lack is someone to explain how to make use of all this
	stuff i.e. how exactly is data stored, how spline interpolations
	are performed, what are those things called quaternions, etc. And
	also, maybe, dig the rest of the chunks until we are actually able
	to write 3DS files instead of just being bored reading. There's
	lots to do.
 
    If you decide to work on this further, please make your findings
	public like we have already done, ok? Upload it to
	x2ftp.oulu.fi, THE place for programming info, and/or to
	ftp.cdrom.com. But please PUBLISH it!
 
    - Change log -
 
    V 1.2:
	- Added change log to have some idea what's going on.
	- Added pivot point reading inside tracks stuff.
	- Info about spline flags on keyframes.
	- Added face edge visibility info.
	- Finally!! Those flags that mark when the texture is wrapping
	  around inside a face. This happens when you apply spherical
	  or cylindrical coordinates, the faces along the 0§ axis don't
	  get proper mapping coords. Someone describe how to fix this?
	- Added -quiet parm, only displays minimal chunk info.
	- Object parent number is stored in CHUNK_TRACKOBJNAME.
	  This makes reference to the node number in CHUNK_OBJNUMBER.
	- Object number changed to unsigned. Parent 65535 means none.
	- Added CHUNK_PRJ and CHUNK_MLI to allow inspecting .PRJ and
	  .MLI files (they're basically the same chunks as .3DS).
	- Added banner to identify myself, and disclaimer for "just in
	  case" possibilities.
	- Corrected possible bug when chunklen == 0 (it was not a
	  chunk).
	- Added several name descriptions of chunks. Use diff to find
	  all the new chunks.
*/


#ifndef PI
#define PI 3.14159265358979323846264338327
#endif

typedef struct {
	uint16    id;
	uint32   len;
} TChunkHeader;

enum {
    CHUNK_RGBF      = 0x0010,
    CHUNK_RGBB      = 0x0011,
/*  CHUNK_RBGB2     = 0x0012,    ?? NOT HLS */
    CHUNK_WRD       = 0x0030,

    CHUNK_PRJ       = 0xC23D,
    CHUNK_MLI       = 0x3DAA,
    CHUNK_SHP       = 0x2D2D,
    CHUNK_LFT       = 0x2D3D,

    CHUNK_MAIN      = 0x4D4D,
	CHUNK_SCALE     = 0xC420,
	CHUNK_OBJMESH   = 0x3D3D,
	    CHUNK_BKGCOLOR  = 0x1200,
	    CHUNK_AMBCOLOR  = 0x2100,
	    CHUNK_OBJBLOCK  = 0x4000,
		CHUNK_TRIMESH   = 0x4100,
		    CHUNK_VERTLIST  = 0x4110,
		    CHUNK_VERTFLAGS = 0x4111,
		    CHUNK_FACELIST  = 0x4120,
			CHUNK_FACEMAT   = 0x4130,
		    CHUNK_MAPLIST   = 0x4140,
		    CHUNK_SMOOLIST  = 0x4150,
		    CHUNK_TRMATRIX  = 0x4160,
		    CHUNK_MESHCOLOR = 0x4165,
		    CHUNK_TXTINFO   = 0x4170,
		CHUNK_LIGHT     = 0x4600,
		    CHUNK_LIGHTR1  = 0x4659,
		    CHUNK_LIGHTR2  = 0x465A,
		    CHUNK_LIGHTMUL = 0x465B,
		    CHUNK_SPOTLIGHT = 0x4610,
		CHUNK_CAMERA    = 0x4700,
		CHUNK_HIERARCHY = 0x4F00,
	CHUNK_VIEWPORT = 0x7001,
	CHUNK_MATERIAL         = 0xAFFF,
	    CHUNK_MATNAME      = 0xA000,
	    CHUNK_AMBIENT      = 0xA010,
	    CHUNK_DIFFUSE      = 0xA020,
	    CHUNK_SPECULAR     = 0xA030,
	    CHUNK_SHININESS    = 0xA040,
	    CHUNK_SHSTRENGTH   = 0xA041,
	    CHUNK_TRANSP       = 0xA050,
	    CHUNK_TRFALLOFF    = 0xA052,
	    CHUNK_REFLECTBL    = 0xA053,
	    CHUNK_FLAG2SIDE    = 0xA081,
	    CHUNK_FLAGADD      = 0xA083,
	    CHUNK_SELFILLUM    = 0xA084,
	    CHUNK_WIREFRAME    = 0xA085, /* Flag */
	    CHUNK_WIRETHICK    = 0xA087, /* Float */
	    CHUNK_FACEMAP      = 0xA088, /* Flag */
	    CHUNK_FLAGIN       = 0xA08A,
	    CHUNK_SOFTEN       = 0xA08C, /* Flag */
	    CHUNK_FLAGUNIT     = 0xA08E, /* Very useless */
	    CHUNK_SHADETYPE    = 0xA100,
	    CHUNK_TEXTURE      = 0xA200,
	    CHUNK_TEXTUREMASK  = 0xA33E,
	    CHUNK_TEXTURE2     = 0xA33A,
	    CHUNK_TEXTUREMASK2 = 0xA340,
	    CHUNK_REFMAP       = 0xA220,
	    CHUNK_REFMASK      = 0xA34C,
	    CHUNK_OPACITYMAP   = 0xA210,
	    CHUNK_OPACITYMASK  = 0xA342,
	    CHUNK_SPECULARMAP  = 0xA204,
	    CHUNK_SPECULARMASK = 0xA348,
	    CHUNK_SHININESSMAP = 0xA33C,
	    CHUNK_SHININESSMASK= 0xA346,
	    CHUNK_SELFILLUMMAP = 0xA33D,
	    CHUNK_SELFILLUMMASK= 0xA34A,
	    CHUNK_BUMPMAP      = 0xA230,
	    CHUNK_BUMPMASK     = 0xA344,
	    CHUNK_MAPFILE      = 0xA300,
		CHUNK_TFLAGS     = 0xA351, /* Flags */
		CHUNK_BLUR       = 0xA353, /* Float */
		CHUNK_USCALE     = 0xA354, /* Float */
		CHUNK_VSCALE     = 0xA356, /* Float */
		CHUNK_UOFFSET    = 0xA358, /* Float */
		CHUNK_VOFFSET    = 0xA35A, /* Float */
		CHUNK_TROTATION  = 0xA35C, /* Float */
		CHUNK_BLACKTINT  = 0xA360, /* Float */
		CHUNK_WHITETINT  = 0xA362, /* Float */
		CHUNK_RTINT      = 0xA364, /* Float */
		CHUNK_GTINT      = 0xA366, /* Float */
		CHUNK_BTINT      = 0xA368, /* Float */
	CHUNK_KEYFRAMER = 0xB000,
	    CHUNK_AMBIENTKEY  = 0xB001,
	    CHUNK_TRACKINFO   = 0xB002,
		CHUNK_TRACKOBJNAME  = 0xB010,
		CHUNK_TRACKPIVOT    = 0xB013,
		CHUNK_TRACKPOS      = 0xB020,
		CHUNK_TRACKROTATE   = 0xB021,
		CHUNK_TRACKSCALE    = 0xB022,
		CHUNK_TRACKMORPH    = 0xB026,
		CHUNK_TRACKHIDE     = 0xB029,
		CHUNK_OBJNUMBER     = 0xB030,
	    CHUNK_TRACKCAMERA = 0xB003,
		CHUNK_TRACKFOV  = 0xB023,
		CHUNK_TRACKROLL = 0xB024,
	    CHUNK_TRACKCAMTGT = 0xB004,
	    CHUNK_TRACKLIGHT  = 0xB005,
	    CHUNK_TRACKLIGTGT = 0xB006,
	    CHUNK_TRACKSPOTL  = 0xB007,
	    CHUNK_FRAMES    = 0xB008
};


/* ------------------------------------ */

static void SkipReader( FILE *f, int ind, int p )
{
	/* Do nothing! */
}


static void RGBFReader (FILE *f, int ind, int p) {
	float c[3];

	if (!r3ds_read_float( c, 3, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s    Red: %f, Green: %f, Blue: %f\n", ind, "", c[0], c[1], c[2]);
#endif
}


static void RGBBReader (FILE *f, int ind, int p) {
	uint8 c[3];
	int rgb[3];
	int i;

	if (!r3ds_read_uint8( c, 3, f )) return;
	for (i = 0; i < 3; i++)
		rgb[i] = c[i];
	r3ds_build( R3DS_COLOR24, rgb );
#ifdef R3DS_ORIG_CODE
	printf("%*s    Red: %d, Green: %d, Blue: %d\n", ind, "", c[0], c[1], c[2]);
#endif
}


static void WRDReader (FILE *f, int ind, int p) {
	uint16 value;

	if (!r3ds_read_uint16( &value, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s    Value: %d\n", ind, "", value);
#endif
}


static void FloatReader (FILE *f, int ind, int p) {
	float value;

	if (!r3ds_read_float( &value, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s    Value: %f\n", ind, "", value);
#endif
}


static void TextFlagReader (FILE *f, int ind, int p) {
	uint8 flag1, flag2;

#ifdef R3DS_ORIG_CODE
	printf("%*s  ", ind, "");
#endif
	if (!r3ds_read_uint8( &flag1, 1, f )) return;
	if (!r3ds_read_uint8( &flag2, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("Source:");
	if ((flag1 & 0xc0)==0x80) printf("RGB luma tint\n");
	else {
		if ((flag1 & 0xc0)==0xc0) printf("Alpha tint\n");
		else {
			if ((flag2 & 0x02)==0x02) printf("RGB tint\n");
			else printf("RGB\n");
		}
	}
	printf("%*s  ", ind, "");
	printf("Filtering:");
	if (flag1 & 0x40) printf("Summed arial\n");
	else printf("Pyramidal\n");
	printf("%*s  ", ind, "");
	printf("Other parameters:");
	switch (flag1 & 0x11) {
	case 0x11: printf("Decal"); break;
	case 0x00: printf("Tile"); break;
	case 0x10:
	case 0x01: printf("Tile & Decal");
	}
	if (flag2 & 1) printf(" - Ignoge map alpha");
	if (flag1 & 8) printf(" - Negative");
	if (flag1 & 2) printf(" - Mirror");
	printf("\n%*s  ", ind, "");
	printf("%02X ", flag1); printf("%02X ", flag2);
	printf("\n");
#endif
}


static void SHTReader (FILE *f, int ind, int p) {
	uint16 value;
	if (!r3ds_read_uint16( &value, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	switch (value) {
	case 1: printf("%*s    Flat\n", ind, ""); break;
	case 2: printf("%*s    Gouraud\n", ind, ""); break;
	case 3: printf("%*s    Phong\n", ind, ""); break;
	case 4: printf("%*s    Metal\n", ind, ""); break;
	}
#endif
}


static char *ASCIIZReader (FILE *f, int ind, int p) {
	static char inbuf[256];
	int c;
	int i = 0;

	/* Read ASCIIZ name */
	while (((c = fgetc(f)) != EOF) && (c != '\0')) {
		if (i < 255)
			inbuf[i++] = c;
#ifdef R3DS_ORIG_CODE
		putchar(c);
#endif
	}
#ifdef R3DS_ORIG_CODE
	printf("\"\n");
#endif
	inbuf[i] = '\0';

	return inbuf;
}


static void ProjScaleReader( FILE *f, int ind, int p )
{
	float unit_size;
	float inches_per_unit;
	uint16 unknown;

	if (!r3ds_read_float( &unit_size, 1, f )) return;
	if (!r3ds_read_float( &inches_per_unit, 1, f )) return;
	r3ds_build( R3DS_PROJECT_SCALE, &inches_per_unit );
	if (!r3ds_read_uint16( &unknown, 1, f )) return;
#if 0
	printf( "%*sProject scale: 1 unit == %.3f real units (unknown)\n", ind, "", unit_size );
	printf( "%*s                      == %.3f inches\n", ind, "", inches_per_unit );
#endif
}


static void ObjBlockReader (FILE *f, int ind, int p) {
	char *name;

	/* Read ASCIIZ object name */
#ifdef R3DS_ORIG_CODE
	printf("%*sObject name \"", ind, "");
#endif
	name = ASCIIZReader(f, ind, p);
	r3ds_build( R3DS_NEW_OBJECT, name );
	/* Read rest of chunks inside this one */
	r3ds_ChunkReader(f, ind, p);
}


static void TriMeshReader( FILE *f, int ind, int p)
{
	r3ds_build( R3DS_DEF_TRIMESH, NULL );
	r3ds_ChunkReader( f, ind, p );
}


static void VertListReader (FILE *f, int ind, int p) {
	uint16 nv;
	float c[3];
	int num_verts;

	if (!r3ds_read_uint16( &nv, 1, f )) return;
	num_verts = nv;
	r3ds_build( R3DS_NUM_VERTS, &num_verts );
#ifdef R3DS_ORIG_CODE
	printf("%*sVertices: %d\n", ind, "", nv);
#endif
	while (nv-- > 0) {
		if (!r3ds_read_float( c, 3, f )) return;
		r3ds_build( R3DS_VERT, c );
#ifdef R3DS_ORIG_CODE
		printf("%*s    X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
#endif
	}
}


static void FaceListReader (FILE *f, int ind, int p) {
	uint16 nv;
	uint16 c[3];
	uint16 flags;
	int tri_def[4];
	int num_tris;
	int i;

	if (!r3ds_read_uint16( &nv, 1, f )) return;
	num_tris = nv;
	r3ds_build( R3DS_NUM_TRIS, &num_tris );
#ifdef R3DS_ORIG_CODE
	printf("%*sFaces: %d\n", ind, "", nv);
#endif
	while (nv-- > 0) {
		if (!r3ds_read_uint16( c, 3, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
		for (i = 0; i < 3; i++)
			tri_def[i] = c[i];
		tri_def[3] = flags;
		r3ds_build( R3DS_TRI_FACE, tri_def );
#ifdef R3DS_ORIG_CODE
		printf("%*s  A %d, B %d, C %d, 0x%X:",
		       ind, "", c[0], c[1], c[2], flags);
		printf(" AB %d BC %d CA %d UWrap %d VWrap %d\n",
		       (flags & 0x04) != 0, (flags & 0x02) != 0, (flags & 0x01) != 0,
		       (flags & 0x08) != 0, (flags & 0x10) != 0);
#endif
	}
	/* Read rest of chunks inside this one */
	r3ds_ChunkReader(f, ind, p);
}


static void FaceMatReader (FILE *f, int ind, int p) {
	uint16 n, nf;
	int tri_num;
	char *mat_name;

	/* Read ASCIIZ material name */
#ifdef R3DS_ORIG_CODE
	printf("%*sMaterial name for faces: \"", ind, "");
#endif
	mat_name = ASCIIZReader(f, ind, p);
	r3ds_build( R3DS_TRI_MATERIAL_CURRENT, mat_name );

	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sFaces with this material: %d\n", ind, "", n);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		tri_num = nf;
		r3ds_build( R3DS_TRI_MATERIAL, &tri_num );
#ifdef R3DS_ORIG_CODE
		printf("%*s    Face %d\n", ind, "", nf);
#endif
	}
}


static void MapListReader (FILE *f, int ind, int p) {
	uint16 nv;
	float c[2];

	if (!r3ds_read_uint16( &nv, 1, f )) return;
	r3ds_build( R3DS_NUM_VERT_MAPPINGS, &nv );
#ifdef R3DS_ORIG_CODE
	printf("%*sVertices: %d\n", ind, "", nv);
#endif
	while (nv-- > 0) {
		if (!r3ds_read_float( c, 2, f )) return;
		r3ds_build( R3DS_VERT_MAPPING, c );
#ifdef R3DS_ORIG_CODE
		printf("%*s    U: %f, V: %f\n", ind, "", c[0], c[1]);
#endif
	}
}


static void SmooListReader (FILE *f, int ind, int p) {
	uint32 s;
	int i;

	r3ds_build( R3DS_BEGIN_TRI_SMOOTH, NULL );
	while (ftell(f) < p) {
		if (!r3ds_read_uint32( &s, 1, f )) return;
		r3ds_build( R3DS_TRI_SMOOTH, &s );
#ifdef R3DS_ORIG_CODE
		printf("%*sSmoothing groups: ", ind, "");
#endif
		for (i = 0; i < 32; i++)
			if (s & ((uint32)1 << i)) {
#ifdef R3DS_ORIG_CODE
				printf("%d, ", i + 1);
#endif
			}
#ifdef R3DS_ORIG_CODE
		printf("\n");
#endif
	}
}


static void TrMatrixReader(FILE *f, int ind, int p) {
	float rot[9];
	float trans[3];

	if (!r3ds_read_float( rot, 9, f )) return;
	r3ds_build( R3DS_ROT_MATRIX, rot );
#ifdef R3DS_ORIG_CODE
	printf("%*sRotation matrix:\n", ind, "");
	printf("%*s    %f, %f, %f\n", ind, "", rot[0], rot[1], rot[2]);
	printf("%*s    %f, %f, %f\n", ind, "", rot[3], rot[4], rot[5]);
	printf("%*s    %f, %f, %f\n", ind, "", rot[6], rot[7], rot[8]);
#endif
	if (!r3ds_read_float( trans, 3, f )) return;
	r3ds_build( R3DS_TRANS_MATRIX, trans );
#ifdef R3DS_ORIG_CODE
	printf("%*sTranslation matrix: %f, %f, %f\n",
	       ind, "", trans[0], trans[1], trans[2]);
#endif
}


static void LightReader(FILE *f, int ind, int p) {
	float c[3];
	if (!r3ds_read_float( c, 3, f )) return;
	r3ds_build( R3DS_DEF_LIGHT, c );
#ifdef R3DS_ORIG_CODE
	printf("%*s    X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
#endif
	/* Read rest of chunks inside this one */
	r3ds_ChunkReader(f, ind, p);
}


static void SpotLightReader(FILE *f, int ind, int p) {
	float c[5];
	if (!r3ds_read_float( c, 5, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s    Target X: %f, Y: %f, Z: %f; Hotspot %f, Falloff %f\n",
	       ind, "", c[0], c[1], c[2], c[3], c[4]);
#endif
}


static void CameraReader(FILE *f, int ind, int p) {
	float c[8];
	if (!r3ds_read_float( c, 8, f )) return;
	r3ds_build( R3DS_DEF_CAMERA, c );
#ifdef R3DS_ORIG_CODE
	printf("%*s    Position: X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
	printf("%*s    Target: X: %f, Y: %f, Z: %f\n", ind, "", c[3], c[4], c[5]);
	printf("%*s    Bank: %f, Lens: %f\n", ind, "", c[6], c[7]);
#endif
}


static void MatNameReader (FILE *f, int ind, int p) {
	char *matname;

	/* Read ASCIIZ object name */
#ifdef R3DS_ORIG_CODE
	printf("%*sMaterial name \"", ind, "");
#endif
	matname = ASCIIZReader(f, ind, p);
	r3ds_build( R3DS_NEW_MATERIAL, matname );
}


static void MatAmbColorReader( FILE *f, int ind, int p )
{
	r3ds_build( R3DS_MAT_AMBIENT_COLOR, NULL );
	r3ds_ChunkReader( f, ind, p );
}


static void MatDiffColorReader( FILE *f, int ind, int p )
{
	r3ds_build( R3DS_MAT_DIFFUSE_COLOR, NULL );
	r3ds_ChunkReader( f, ind, p );
}


static void MatSpecColorReader( FILE *f, int ind, int p )
{
	r3ds_build( R3DS_MAT_SPECULAR_COLOR, NULL );
	r3ds_ChunkReader( f, ind, p );
}


static void TwoSidedFlagReader( FILE *f, int ind, int p )
{
	r3ds_build( R3DS_MAT_2SIDED, NULL );
}


static void MapFileReader(FILE *f, int ind, int p) {
	/* Read ASCIIZ filename */
#ifdef R3DS_ORIG_CODE
	printf("%*sMap filename \"", ind, "");
#endif
	ASCIIZReader(f, ind, p);
}


static void FramesReader(FILE *f, int ind, int p) {
	uint32 c[2];
	if (!r3ds_read_uint32( c, 2, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s    Start: %d, End: %d\n",
	       ind, "", c[0], c[1]);
#endif
}


static void TrackObjNameReader(FILE *f, int ind, int p) {
	uint16 w[2];
	uint16 parent;

	/* Read ASCIIZ name */
#ifdef R3DS_ORIG_CODE
	printf("%*sTrack object name \"", ind, "");
#endif
	ASCIIZReader(f, ind, p);
	if (!r3ds_read_uint16( w, 2, f )) return;
	if (!r3ds_read_uint16( &parent, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sObject name data: Flags 0x%X, 0x%X, Parent %d\n",
	       ind, "", w[0], w[1], parent);
#endif
}


static void PivotPointReader(FILE *f, int ind, int p) {
	float pos[3];

	if (!r3ds_read_float( pos, 3, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*s  Pivot at X: %f, Y: %f, Z: %f\n",
	       ind, "", pos[0], pos[1], pos[2]);
#endif
}

/* Key info flags for position, rotation and scaling:
Until I know the meaning of each bit in flags I assume all mean
a following float data.
*/

/* NOTE THIS IS NOT A CHUNK, but A PART OF SEVERAL CHUNKS */
static void SplineFlagsReader(FILE *f, int ind, uint16 flags) {
	int i;
	float dat;
#ifdef R3DS_ORIG_CODE
	static const char *flagnames[] = {
		                                 "Tension",
		                                 "Continuity",
		                                 "Bias",
		                                 "Ease To",
		                                 "Ease From",
	                                 };
#endif

	for (i = 0; i < 16; i++) {
		if (flags & (1 << i)) {
			if (!r3ds_read_float( &dat, 1, f )) return;
#ifdef R3DS_ORIG_CODE
			if (i < sizeof(flagnames)/sizeof(*flagnames)) {
				printf("%*s		%-15s = %f\n",
				       ind, "", flagnames[i], dat);
			}
			else {
				printf("%*s		%-15s = %f\n",
				       ind, "", "Unknown", dat);
			}
#endif
		}
	}
}


static void TrackPosReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	float pos[3];
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sPosition keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
		if (!r3ds_read_float( pos, 3, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s		X: %f, Y: %f, Z: %f\n",
		       ind, "", pos[0], pos[1], pos[2]);
#endif
	}
}


static void TrackRotReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	float pos[4];
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sRotation keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
		if (!r3ds_read_float( pos, 4, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s		Angle: %f§, X: %f, Y: %f, Z: %f\n",
		       ind, "", pos[0]*180.0/PI, pos[1], pos[2], pos[3]);
#endif
	}
}


static void TrackScaleReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	float pos[3];
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sScale keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
		if (!r3ds_read_float( pos, 3, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s	       X: %f, Y: %f, Z: %f\n",
		       ind, "", pos[0], pos[1], pos[2]);
#endif
	}
}


static void TrackFovReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	float fov;
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sFOV keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
		if (!r3ds_read_float( &fov, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s	       FOV: %f\n", ind, "", fov);
#endif
	}
}


static void TrackRollReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	float roll;
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sRoll keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
		if (!r3ds_read_float( &roll, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s	       Roll: %f\n", ind, "", roll);
#endif
	}
}


static void TrackMorphReader(FILE *f, int ind, int p) {
	uint16 n, nf;
	uint16 unkown;
	uint16 flags;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sMorph keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	while (n-- > 0) {
		if (!r3ds_read_uint16( &nf, 1, f )) return;
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
		if (!r3ds_read_uint16( &flags, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
#endif
		SplineFlagsReader(f, ind, flags);
#ifdef R3DS_ORIG_CODE
		printf("%*s		Object name: \"", ind, "");
#endif
		ASCIIZReader(f, ind, p);
	}
}


static void TrackHideReader(FILE *f, int ind, int p) {
	uint16 n;
	uint16 frame;
	uint16 unkown;
	int i;

	for(i=0; i<5; i++) {
		if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sUnknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
	}
	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sHide keys: %d\n", ind, "", n);
#endif
	if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sUnknown: 0x%x\n", ind, "", unkown);
#endif
	ind += 2;
	while (n-- > 0) {
		if (!r3ds_read_uint16( &frame, 1, f )) return;
#ifdef R3DS_ORIG_CODE
		printf("%*sFrame: %d\n", ind, "", (uint32) frame);
#endif
		for(i=0; i<2; i++) {
			if (!r3ds_read_uint16( &unkown, 1, f )) return;
#ifdef R3DS_ORIG_CODE
			printf("%*s  Unknown #%d: 0x%x\n", ind, "", i, unkown);
#endif
		}
	}
}


static void ObjNumberReader(FILE *f, int ind, int p) {
	uint16 n;

	if (!r3ds_read_uint16( &n, 1, f )) return;
#ifdef R3DS_ORIG_CODE
	printf("%*sObject number: %d\n", ind, "", n);
#endif
}


/* ------------------------------------ */

struct {
	uint16 id;
	const char *name;
	void (*func)(FILE *f, int ind, int p);
} ChunkNames[] = {
    {CHUNK_RGBF,	"RGB float",	    RGBFReader},
    {CHUNK_RGBB,	"RGB byte",	    RGBBReader},
    {CHUNK_WRD, 	"WORD", 	    WRDReader},

    {CHUNK_PRJ, 	"Project",	    NULL},
    {CHUNK_MLI, 	"Material Library", NULL},
    {CHUNK_SHP, 	"Shape file", NULL},
    {CHUNK_LFT, 	"Loft file", NULL},

    {CHUNK_MAIN,	"Main", 	    NULL},
    {CHUNK_SCALE,	"Project scale",    ProjScaleReader},
    {CHUNK_OBJMESH,	"Object Mesh",	    NULL},
    {CHUNK_BKGCOLOR,	"Background color", NULL},
    {CHUNK_AMBCOLOR,	"Ambient color",    NULL},
    {CHUNK_OBJBLOCK,	"Object Block",     ObjBlockReader},
    {CHUNK_TRIMESH,	"Tri-Mesh",	    TriMeshReader},
    {CHUNK_VERTLIST,	"Vertex list",	    VertListReader},
    {CHUNK_VERTFLAGS,	"Vertex flag list", SkipReader},
    {CHUNK_FACELIST,	"Face list",	    FaceListReader},
    {CHUNK_MESHCOLOR,	"Mesh color",	    SkipReader},
    {CHUNK_FACEMAT,	"Face material",    FaceMatReader},
    {CHUNK_MAPLIST,	"Mappings list",    MapListReader},
    {CHUNK_TXTINFO,	"Texture info",     SkipReader},
    {CHUNK_SMOOLIST,	"Smoothings",	    SmooListReader},
    {CHUNK_TRMATRIX,	"Matrix",	    TrMatrixReader},
    {CHUNK_LIGHT,	"Light",	    LightReader},
    {CHUNK_LIGHTR1,	"Light Range radius1",	       FloatReader},
    {CHUNK_LIGHTR2,	"Light Range radius2",	       FloatReader},
    {CHUNK_LIGHTMUL,	"Light Multyplity",	       FloatReader},
    {CHUNK_SPOTLIGHT,	"Spotlight",	    SpotLightReader},
    {CHUNK_CAMERA,	"Camera",	    CameraReader},
    {CHUNK_HIERARCHY,	"Hierarchy",	    NULL},

    {CHUNK_VIEWPORT,	"Viewport info",    SkipReader},

    {CHUNK_MATERIAL,	 "Material",		 NULL},
    {CHUNK_MATNAME,	 "Material name",	 MatNameReader},
    {CHUNK_AMBIENT,	 "Ambient color",	 MatAmbColorReader},
    {CHUNK_DIFFUSE,	 "Diffuse color",	 MatDiffColorReader},
    {CHUNK_SPECULAR,	 "Specular color",	 MatSpecColorReader},
    {CHUNK_SHININESS,	 "Shininess",		 NULL},
    {CHUNK_SHSTRENGTH,	 "Shininess strength",	 NULL},
    {CHUNK_TRANSP,	 "Transparency",	 NULL},
    {CHUNK_TRFALLOFF,	 "Transparency falloff", NULL},
    {CHUNK_REFLECTBL,	 "Reflection blur",	 NULL},
    {CHUNK_SELFILLUM,	 "Self Illumination",	 NULL},
    {CHUNK_FLAG2SIDE,	 "2Side: ON /flag/",	 TwoSidedFlagReader},
    {CHUNK_FLAGADD,	 "Transparency: Add /flag/",		    SkipReader},
    {CHUNK_WIREFRAME,	 "WireFrame: ON /flag/", SkipReader},
    {CHUNK_WIRETHICK,	 "WireFrame thickness",  FloatReader},
    {CHUNK_FLAGIN,	 "Transparency falloff: In /flag/",	    SkipReader},
    {CHUNK_FLAGUNIT,	 "WireFrame thickness: Unit /flag/",	    SkipReader},
    {CHUNK_FACEMAP,	 "Self Illumination: FaceMap /flag/",	    SkipReader},
    {CHUNK_SOFTEN,	 "Shininess: Soften /flag/",	    SkipReader},
    {CHUNK_SHADETYPE,	 "Shadeing type",		    SHTReader},
    {CHUNK_TEXTURE,	 "Texture map", 		    NULL},
    {CHUNK_SPECULARMAP,  "Specular map",		    NULL},
    {CHUNK_OPACITYMAP,	 "Opacity map", 		    NULL},
    {CHUNK_TEXTURE2,	 "Texture map 2",		    NULL},
    {CHUNK_SHININESSMAP, "Shininess map",		    NULL},
    {CHUNK_SELFILLUMMAP, "Self Illumination map",	    NULL},
    {CHUNK_REFMAP,	 "Reflection map",		    NULL},
    {CHUNK_BUMPMAP,	 "Bump map",			    NULL},
    {CHUNK_TEXTUREMASK,  "Texture mask",		    NULL},
    {CHUNK_SPECULARMASK, "Specular mask",		    NULL},
    {CHUNK_OPACITYMASK,  "Opacity mask",		    NULL},
    {CHUNK_TEXTUREMASK2, "Texture2 mask",		    NULL},
    {CHUNK_SHININESSMASK,"Shininess mask",		    NULL},
    {CHUNK_SELFILLUMMASK,"Self Illumination mask",	    NULL},
    {CHUNK_REFMASK,	 "Reflection mask",		    NULL},
    {CHUNK_BUMPMASK,	 "Bump mask",			    NULL},
    {CHUNK_TFLAGS,	 "Texture flags",		    TextFlagReader},
    {CHUNK_BLUR,	 "Texture Blur",		    FloatReader},
    {CHUNK_USCALE,	 "U scale (1/x)",		    FloatReader},
    {CHUNK_VSCALE,	 "V scale (1/x)",		    FloatReader},
    {CHUNK_UOFFSET,	 "U offset",			    FloatReader},
    {CHUNK_VOFFSET,	 "V offset",			    FloatReader},
    {CHUNK_TROTATION,	 "Texture Rotation angle",	    FloatReader},
    {CHUNK_BLACKTINT,	 "Black Tint",			    RGBBReader},
    {CHUNK_WHITETINT,	 "White Tint",			    RGBBReader},
    {CHUNK_RTINT,	 "Red Tint",			    RGBBReader},
    {CHUNK_GTINT,	 "Green Tint",			    RGBBReader},
    {CHUNK_BTINT,	 "Blue Tint",			    RGBBReader},
    {CHUNK_MAPFILE,	 "Map filename",		    MapFileReader},

    {CHUNK_KEYFRAMER,	"Keyframer data",   NULL},
    {CHUNK_AMBIENTKEY,	"Ambient key",	    NULL},
    {CHUNK_TRACKINFO,	"Track info",	    NULL},
    {CHUNK_FRAMES,	"Frames",	    FramesReader},
    {CHUNK_TRACKOBJNAME,"Track Obj. Name",  TrackObjNameReader},
    {CHUNK_TRACKPIVOT,	"Pivot point",	    PivotPointReader},
    {CHUNK_TRACKPOS,	"Position keys",    TrackPosReader},
    {CHUNK_TRACKROTATE, "Rotation keys",    TrackRotReader},
    {CHUNK_TRACKSCALE,	"Scale keys",	    TrackScaleReader},
    {CHUNK_TRACKMORPH,	"Morph keys",	    TrackMorphReader},
    {CHUNK_TRACKHIDE,	"Hide keys",	    TrackHideReader},
    {CHUNK_OBJNUMBER,	"Object number",    ObjNumberReader},

    {CHUNK_TRACKCAMERA, "Camera track", 	    NULL},
    {CHUNK_TRACKCAMTGT, "Camera target track",	    NULL},
    {CHUNK_TRACKLIGHT,	"Pointlight track",	    NULL},
    {CHUNK_TRACKLIGTGT, "Pointlight target track",  NULL},
    {CHUNK_TRACKSPOTL,	"Spotlight track",	    NULL},
    {CHUNK_TRACKFOV,	"FOV track",	    TrackFovReader},
    {CHUNK_TRACKROLL,	"Roll track",	    TrackRollReader},
};


static int FindChunk(uint16 id) {
	int i;
	for (i = 0; i < sizeof(ChunkNames)/sizeof(ChunkNames[0]); i++)
		if (id == ChunkNames[i].id)
			return i;
	return -1;
}

/* ------------------------------------ */

#ifdef R3DS_ORIG_CODE
int Verbose = 1;
int Quiet   = 0;
int Dump    = 1;
#endif

void r3ds_ChunkReader(FILE *f, int ind, int p) {
	TChunkHeader h;
	int n;
#ifdef R3DS_ORIG_CODE
	int i;
	uint8 d;
#endif
	int pc;

	while (ftell(f) < p) {
		pc = ftell(f);
		if (!r3ds_read_uint16( &h.id, 1, f )) return;
		if (!r3ds_read_uint32( &h.len, 1, f )) return;
		if (h.len == 0) return;
		n = FindChunk(h.id);
		if (n < 0) {
#ifdef R3DS_ORIG_CODE
			if (Verbose) {
				printf("%*sUnknown chunk: 0x%04X, offset 0x%X, size: %d bytes.",
				       ind, "", h.id, pc, h.len);
				if (!Dump)
					printf("\n");
			}
			if (Dump) {
				fseek(f, pc + 6, SEEK_SET);
				for (i=0; i<h.len-6; i++) {
					if ((i & 0xf) == 0) printf("\n%*s  ", ind, "");
					if (!r3ds_read_uint8( &d, 1, f )) return;
					printf("%02X ", d);
				}
				printf("\n");
			} else
				fseek(f, pc + (int)h.len, SEEK_SET);
#else
			fseek( f, pc + (int)h.len, SEEK_SET );
#endif
		} else {
#ifdef R3DS_ORIG_CODE
			if (!Quiet || ChunkNames[n].func == NULL)
				printf("%*sChunk type \"%s\", offset 0x%X, size %d bytes\n",
				       ind, "", ChunkNames[n].name, pc, h.len);
#endif
			pc = pc + h.len;
			if (ChunkNames[n].func != NULL)
				ChunkNames[n].func(f, ind + 2, pc);
			else
				r3ds_ChunkReader(f, ind + 2, pc);
			fseek(f, pc, SEEK_SET);
		}
		if (ferror(f))
			break;
	}
}


/* ------------------------------------ */

/* The remaining code are the vestiges of 3DSRDR.C's days as a plain-vanilla
 * file dumper. I leave it intact for historical purposes */

#if 0

void Banner(void) {
	printf("3DSRDR, 3D Studio formats reader v1.2 by Javier Arevalo AKA Jare/Iguana.\n"
	       "Modified by Mats Byggmastar aka. MRI/Doomsday (v1.3)\n"
	       "& Modified again by Bezzegh D‚nes aka. DiVeR /diver@inf.bme.hu/ (v1.4)\n");
}

int main(int argc, char *argv[]) {
	FILE *f;
	int p;

	if (argc < 2) {
		Banner();
		printf("Usage: 3DSRDR file.3DS (or .PRJ or .MLI) [-quiet, -verbose or -dump]\n");
		exit(1);
	}

	f = fopen(argv[1], "rb");
	if (f == NULL) {
		Banner();
		printf("Can't open %s!\n", argv[1]);
		exit(1);
	}

	if (argc > 2 && strcmp(argv[2], "-quiet") == 0)
		Quiet = 1;
	if (argc > 2 && strcmp(argv[2], "-verbose") == 0)
		Verbose = 1;
	if (argc > 2 && strcmp(argv[2], "-dump") == 0) {
		Verbose = 1;
		Dump = 1;
	}

	if (!Quiet)
		Banner();

	/* Find file size */
	fseek(f, 0, SEEK_END);
	p = ftell(f);
	fseek(f, 0, SEEK_SET);
	/* Go! */
	r3ds_ChunkReader(f, 0, p);

	return 0;
}

#endif /* 0 */

/* end read3ds.c */
