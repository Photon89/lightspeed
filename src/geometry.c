/* geometry.c */

/* Various geometrical and geometry-related routines */

/*
 *  ``The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is the "Light Speed!" relativistic simulator.
 *
 *  The Initial Developer of the Original Code is Daniel Richard G.
 *  Portions created by the Initial Developer are Copyright (C) 1999
 *  Daniel Richard G. <skunk@mit.edu> All Rights Reserved.
 *
 *  Contributor(s): ______________________________________.''
 */


#include "lightspeed.h"


/* Allocates an ogl_object */
ogl_object *
alloc_ogl_object( int num_vertices, int num_indices )
{
	ogl_object *new_obj;
	int i;

	new_obj = xmalloc( sizeof(ogl_object) );
	new_obj->num_vertices = num_vertices;
	new_obj->vertices0 = xmalloc( num_vertices * sizeof(point) );
	new_obj->normals0 = xmalloc( num_vertices * sizeof(point) );
	new_obj->iarrays = xmalloc( num_vertices * sizeof(ogl_point) );
	/* Initialize "a" fields in iarrays, since we don't really use them */
	for (i = 0; i < num_vertices; i++)
		new_obj->iarrays[i].a = 1.0;
	new_obj->num_indices = num_indices;
	new_obj->indices = xmalloc( num_indices * sizeof(unsigned int) );
	new_obj->pre_dlist = 0; /* null display list */
	new_obj->post_dlist = 0; /* ditto */

	return new_obj;
}


/* Returns the size in memory (bytes) used by an ogl_object having
 * so many vertices and so many indices */
int
calc_ogl_object_memusage( int num_vertices, int num_indices )
{
	int num_bytes;

	/* The vertices0 and normals0 arrays */
	num_bytes = 2 * num_vertices * sizeof(point);
	/* The C4F+N3F+V3F superarray */
	num_bytes += num_vertices * sizeof(ogl_point);
	/* The indices array */
	num_bytes += num_indices * sizeof(unsigned int);
	/* The ogl_object structure itself */
	num_bytes += sizeof(ogl_object);

	return num_bytes;
}


/* Deallocates an ogl_object */
void
free_ogl_object( ogl_object *obj )
{
	xfree( obj->vertices0 );
	xfree( obj->normals0 );
	xfree( obj->iarrays );
	xfree( obj->indices );
	if (obj->pre_dlist != 0)
		glDeleteLists( obj->pre_dlist, 1 );
	if (obj->post_dlist != 0)
		glDeleteLists( obj->post_dlist, 1 );
	xfree( obj );
}


/* Gets rid of all current objects */
void
clear_all_objects( void )
{
	int i;

	for (i = 0; i < num_vehicle_objs; i++)
		free_ogl_object( vehicle_objs[i] );
	xfree( vehicle_objs );
	num_vehicle_objs = 0;
}


void
rotate_all_objects( int direction )
{
	ogl_object *obj;
	float x0,y0,z0;
	float x,y,z;
	float xmax = -1E6, ymax = -1E6, zmax = -1E6;
	float xmin = 1E6, ymin = 1E6, zmin = 1E6;
	int o, v;

	for (o = 0; o < num_vehicle_objs; o++) {
		obj = vehicle_objs[o];
		for (v = 0; v < obj->num_vertices; v++) {
			/* Rotate vertex */
			x0 = obj->vertices0[v].x;
			y0 = obj->vertices0[v].y;
			z0 = obj->vertices0[v].z;
			rotate_xyz( direction, &x, &y, &z, x0, y0, z0 );
			obj->vertices0[v].x = x;
			obj->vertices0[v].y = y;
			obj->vertices0[v].z = z;

			/* Update vehicle extents */
			xmin = MIN(x, xmin);
			xmax = MAX(x, xmax);
			ymin = MIN(y, ymin);
			ymax = MAX(y, ymax);
			zmin = MIN(z, zmin);
			zmax = MAX(z, zmax);

			/* Rotate normal too */
			x0 = obj->normals0[v].x;
			y0 = obj->normals0[v].y;
			z0 = obj->normals0[v].z;
			rotate_xyz( direction, &x, &y, &z, x0, y0, z0 );
			obj->normals0[v].x = x;
			obj->normals0[v].y = y;
			obj->normals0[v].z = z;
		}
	}

	/* Reset vehicle_extents */
	vehicle_extents.xmin = xmin;
	vehicle_extents.xmax = xmax;
	vehicle_extents.ymin = ymin;
	vehicle_extents.ymax = ymax;
	vehicle_extents.zmin = zmin;
	vehicle_extents.zmax = zmax;
	vehicle_extents.avg = ((xmax - xmin) + (ymax - ymin) + (zmax - zmin)) / 3;

	queue_redraw( -1 );
}


/* Rotates an XYZ triplet whichever way specified
 * The ?_ALIGN actions realign a point from the x-axis to the indicated axis */
void
rotate_xyz( int action, float *x, float *y, float *z, float x0, float y0, float z0 )
{
	switch (action) {
	case X_ALIGN:
		*x = x0;
		*y = y0;
		*z = z0;
		break;

	case Y_ALIGN:
		*x = - y0;
		*y = x0;
		*z = z0;
		break;

	case Z_ALIGN:
		*x = - z0;
		*y = y0;
		*z = x0;
		break;

	case Z_ROTATE_CW:
		*x = y0;
		*y = - x0;
		*z = z0;
		break;

	case Z_ROTATE_CCW:
		*x = - y0;
		*y = x0;
		*z = z0;
		break;

	default:
#ifdef DEBUG
		crash( "rotate_xyz( ): invalid action" );
#endif
		return;
	}
}


/* Calculate unit normal vector for a given triangle */
point *
calc_tri_normal( point *a, point *b, point *c )
{
	static point normal;
	point vec_ab;
	point vec_ac;
	float d;

	/* Obtain A->B vector */
	vec_ab.x = b->x - a->x;
	vec_ab.y = b->y - a->y;
	vec_ab.z = b->z - a->z;
	/* Obtain A->C vector */
	vec_ac.x = c->x - a->x;
	vec_ac.y = c->y - a->y;
	vec_ac.z = c->z - a->z;
	/* Obtain cross product (A->B x A->C) to get normal vector */
	normal.x = vec_ab.y * vec_ac.z - vec_ab.z * vec_ac.y;
	normal.y = vec_ab.z * vec_ac.x - vec_ab.x * vec_ac.z;
	normal.z = vec_ab.x * vec_ac.y - vec_ab.y * vec_ac.x;
	/* Scale it by own length to get unit normal vector */
	d = sqrt( SQR(normal.x) + SQR(normal.y) + SQR(normal.z) );
	if (d < 1E-6)
		d = 1.0;
	normal.x /= d;
	normal.y /= d;
	normal.z /= d;

	return &normal;
}


/* Calculate the centroid of a given triangle */
point *
calc_tri_centroid( point *a, point *b, point *c )
{
	static point centroid;

	centroid.x = (a->x + b->x + c->x) / 3.0;
	centroid.y = (a->y + b->y + c->y) / 3.0;
	centroid.z = (a->z + b->z + c->z) / 3.0;

	return &centroid;
}


/* Calculate area of a given triangle */
float
calc_tri_area( point *a, point *b, point *c )
{
	point vec_ab, vec_ac;
	point xprod;

	/* Obtain A->B vector */
	vec_ab.x = b->x - a->x;
	vec_ab.y = b->y - a->y;
	vec_ab.z = b->z - a->z;
	/* Obtain A->C vector */
	vec_ac.x = c->x - a->x;
	vec_ac.y = c->y - a->y;
	vec_ac.z = c->z - a->z;
	/* Magnitude of cross product == 2 * area of ascribed triangle */
	xprod.x = vec_ab.y * vec_ac.z - vec_ab.z * vec_ac.y;
	xprod.y = vec_ab.z * vec_ac.x - vec_ab.x * vec_ac.z;
	xprod.z = vec_ab.x * vec_ac.y - vec_ab.y * vec_ac.x;

	return sqrt( SQR(xprod.x) + SQR(xprod.y) + SQR(xprod.z) ) / 2.0;
}


void
show_geometry_stats( void )
{
	ogl_object *obj;
	float r,g,b;
	float ex,ey,ez;
	int vnum, inum;
	int vnum_total = 0, inum_total = 0;
	int i;

	printf( "=========== Light Speed! geometry stats ===========\n" );
	printf( "Object    Vertices    Indices    RGB base color\n" );
	printf( "------    --------    -------    ------------------\n" );
	for (i = 0; i < num_vehicle_objs; i++) {
		obj = vehicle_objs[i];
		vnum = obj->num_vertices;
		vnum_total += vnum;
		inum = obj->num_indices;
		inum_total += inum;
		r = obj->color0.r;
		g = obj->color0.g;
		b = obj->color0.b;
		printf( "%6d%12d%11d    (%.2f, %.2f, %.2f)\n", i, vnum, inum, r, g, b );
	}
	printf( "------    --------    -------    ------------------\n" );
	printf( "Object    Vertices    Indices    RGB base color\n" );
	printf( "------    --------    -------\n" );
	printf( " Total%12d%11d\n", vnum_total, inum_total );
	ex = vehicle_extents.xmax - vehicle_extents.xmin;
	ey = vehicle_extents.ymax - vehicle_extents.ymin;
	ez = vehicle_extents.zmax - vehicle_extents.zmin;
	printf( " Size:  %.3f x %.3f x %.3f\n", ex, ey, ez );
	printf( "===================================================\n" );
	fflush( stdout );
}


#ifdef WITH_SRS_EXPORTER

/* Write out an .srs (Special Relativity Scene) file, for use with the
 * BACKLIGHT relativistic raytracer */
int
export_srs( const char *filename, int width, int height, int stereo_view, int visible_faces_only )
{
	FILE *srs;
	camera eye_cam;
	point p;
	rgb_color *bg;
	float dx,dy;
	float eye_dx, eye_dy;
	float t;
	int flag, i;

	srs = fopen( filename, "w" );

	/* Initialize output camera */
	memcpy( &out_cam, usr_cams[0], sizeof(camera) );
	out_cam.width = width;
	out_cam.height = height;

	/* Header and global settings */
	fprintf( srs, "// %s\n", file_basename( filename, NULL ) );
	fprintf( srs, "// File generated by the Light Speed! SRS exporter\n" );
	fprintf( srs, "// for use with the BACKLIGHT relativistic raytracer\n\n" );
	fprintf( srs, "Width = %d\n", width );
	fprintf( srs, "Height = %d\n\n", height );
	fprintf( srs, "Antialias = 1\n" );
	/* Doppler shift switch */
	flag = warp( QUERY, MESG_(WARP_DOPPLER_SHIFT) );
	i = flag ? 1 : 0;
	fprintf( srs, "Doppler = %d\n", i );
	/* Headlight effect (intensity) switch */
	flag = warp( QUERY, MESG_(WARP_HEADLIGHT_EFFECT) );
	i = flag ? 1 : 0;
	fprintf( srs, "Intensity = %d\n\n", i );
	fprintf( srs, "Output_File_Name = \"%s\"\n\n", file_basename( filename, ".srs" ) );

	/* Background color */
	fprintf( srs, "background { colour rgb " );
	bg = &background;
	fprintf( srs, "< %.2f, %.2f, %.2f > }\n\n", bg->r, bg->g, bg->b );

	/* Stationary (observer) frame */
	fprintf( srs, "frame {\n" );
	fprintf( srs, "\t// Observer frame (stationary)\n" );

	/* Camera(s) */
	t = C * cur_time_t;
	if (stereo_view) {
		/* Stereoscopic view */
		dx = out_cam.target.x - out_cam.pos.x;
		dy = out_cam.target.y - out_cam.pos.y;
		eye_dx = (EYE_SPACING / 2.0) * dy / sqrt( SQR(dx) + SQR(dy) );
		eye_dy = - (EYE_SPACING / 2.0) * dx / sqrt( SQR(dx) + SQR(dy) );

		fprintf( srs, "\t// Left eye view\n" );
		memcpy( &eye_cam, &out_cam, sizeof(camera) );
		eye_cam.pos.x -= eye_dx;
		eye_cam.pos.y -= eye_dy;
		write_srs_camera( srs, &eye_cam, t );

		fprintf( srs, "\t// Right eye view\n" );
		memcpy( &eye_cam, &out_cam, sizeof(camera) );
		eye_cam.pos.x += eye_dx;
		eye_cam.pos.y += eye_dy;
		write_srs_camera( srs, &eye_cam, t );
	}
	else {
		/* Normal view */
		write_srs_camera( srs, &out_cam, t );
	}

	/* Light source */
	fprintf( srs, "\tlight_source {\n" );
	convert_to_srs_cs( &p, &out_cam.pos );
	fprintf( srs, "\t\t< %.3f, %.3f, %.3f >\n", p.x, p.y, p.z );
	fprintf( srs, "\t\tcolour rgb " );
	fprintf( srs, "< 1, 1, 1 >\n" );
	fprintf( srs, "\t}\n" );
	fprintf( srs, "}\n\n" );

	/* Moving (object) frame */
	fprintf( srs, "frame {\n" );
	fprintf( srs, "\t// Object frame (moving)\n" );
	fprintf( srs, "\t// v = %s\n", velocity_string( velocity, TRUE ) );
	fprintf( srs, "\tvelocity " );
	fprintf( srs, "< %.6f, 0, 0 >\n", velocity / C );

	/* Export lattice or arbitrary mesh geometry */
	fprintf( srs, "\n" );
	if (object_mode == MODE_LATTICE)
		write_srs_lattice( srs );
	else
		write_srs_mesh( srs, &out_cam.pos, visible_faces_only );
	fprintf( srs, "\n" );

	fprintf( srs, "}\n" );
	fclose( srs );

	return 0;
}


void
write_srs_mesh( FILE *srs, point *cam_pos, int visible_faces_only )
{
	ogl_object *obj;
	point *face_verts[4];
	point *face_norms[4];
	point *vert_a, *vert_b;
	point tri_verts[3];
	point *norm, *cent;
	float dx,dy,dz;
	float fdir;
	int checks[6][2] = { {0,1}, {1,2}, {2,0}, {0,3}, {1,3}, {2,3} };
	int face_size;
	int num_faces;
	int num_checks;
	int *face_flags;
	int base, ind, ind_a, ind_b;
	int bad_face, at_least_one_good_face;
	int o, f, i, a, b;

	fprintf( srs, "\t// **** Begin mesh definition ****\n" );

	for (o = 0; o < num_vehicle_objs; o++) {
		obj = vehicle_objs[o];
		switch (obj->type) {
		case GL_TRIANGLES:
			face_size = 3;
			num_checks = 3;
			break;

		case GL_QUADS:
			face_size = 4;
			num_checks = 6;
			break;

		default:
#ifdef DEBUG
			crash( "write_srs_mesh( ): invalid object type" );
#endif
			return;
		}
		num_faces = obj->num_indices / face_size;
		face_flags = xmalloc( num_faces * sizeof(int) );

		/* First, check for good faces in this object
		 * (good = no coincident vertices, no zero normals) */
		at_least_one_good_face = FALSE;
		for (f = 0; f < num_faces; f++) {
			base = f * face_size;
			bad_face = FALSE; /* benefit of the doubt */

			for (i = 0; i < num_checks; i++) {
				/* Check between vertex pair */
				a = checks[i][0];
				b = checks[i][1];
				ind_a = obj->indices[base + a];
				ind_b = obj->indices[base + b];
				vert_a = &obj->vertices0[ind_a];
				vert_b = &obj->vertices0[ind_b];
				dx = ABS(vert_a->x - vert_b->x);
				dy = ABS(vert_a->y - vert_b->y);
				dz = ABS(vert_a->z - vert_b->z);
				if ((dx < 1E-4) && (dy < 1E-4) && (dz < 1E-4))
					bad_face = TRUE;
			}

			/* Check face normals */
			for (i = 0; i < face_size; i++) {
				ind = obj->indices[base + i];
				norm = &obj->normals0[ind];
				dx = ABS(norm->x);
				dy = ABS(norm->y);
				dz = ABS(norm->z);
				if ((dx < 1E-2) && (dy < 1E-2) && (dz < 1E-2))
					bad_face = TRUE;
			}

			/* If only visible faces are desired, check if face
			 * is facing the camera (dropping it if not) */
			if (visible_faces_only && !bad_face) {
				for (i = 0; i < 3; i++) {
					ind = obj->indices[base + i];
					memcpy( &tri_verts[i], &obj->vertices0[ind], sizeof(point) );
					warp_point( &tri_verts[i], NULL, cam_pos );
				}
				/* Calculate (warped) flat triangle normal
				 * (for quads: 4th vertex is coplanar anyway) */
				norm = calc_tri_normal( &tri_verts[0], &tri_verts[1], &tri_verts[2] );
				/* Calculate (warped) centroid and then the
				 * triangle-to-camera (reverse view) vector */
				cent = calc_tri_centroid( &tri_verts[0], &tri_verts[1], &tri_verts[2] );
				dx = cam_pos->x - cent->x;
				dy = cam_pos->y - cent->y;
				dz = cam_pos->z - cent->z;
				/* Get dot product of normal and rview vectors */
				fdir = (norm->x * dx) + (norm->y * dy) + (norm->z * dz);
				if (fdir < 0)
					bad_face = TRUE; /* not facing camera */
			}

			if (bad_face)
				face_flags[f] = FALSE;
			else {
				face_flags[f] = TRUE;
				at_least_one_good_face = TRUE;
			}
		}

		if (!at_least_one_good_face) {
			xfree( face_flags );
			continue;
		}

		/* Write out the good faces */
		fprintf( srs, "\tunion {\n" );
		for (f = 0; f < num_faces; f++) {
			if (!face_flags[f])
				continue; /* skip bad face */
			base = f * face_size;
			for (i = 0; i < face_size; i++) {
				ind = obj->indices[base + i];
				face_verts[i] = &obj->vertices0[ind];
				face_norms[i] = &obj->normals0[ind];
			}
			write_srs_smooth_triangle( srs, face_verts, face_norms );
			if (face_size == 4) {
				/* Output a 2nd triangle to make a quad
				 * Shift 3rd and 4th vertices down */
				face_verts[1] = face_verts[2];
				face_norms[1] = face_norms[2];
				face_verts[2] = face_verts[3];
				face_norms[2] = face_norms[3];
				write_srs_smooth_triangle( srs, face_verts, face_norms );
			}
		}

		xfree( face_flags );

		/* Write out face group color */
		write_srs_pigment( srs, &obj->color0 );
		fprintf( srs, "\t}\n" );
	}

	fprintf( srs, "\t// **** End mesh definition ****\n" );
}


void
write_srs_camera( FILE *srs, camera *cam, float t )
{
	point p;
	float aspect_ratio;

	aspect_ratio = (float)cam->width / (float)cam->height;

	fprintf( srs, "\tcamera {\n" );
	fprintf( srs, "\t\tlocation " );
	convert_to_srs_cs( &p, &cam->pos );
	fprintf( srs, "< %.3f, %.3f, %.3f >, %.3f\n", p.x, p.y, p.z, t );
	fprintf( srs, "\t\tup < 0, 1, 0 >\n" );
	fprintf( srs, "\t\tright " );
	fprintf( srs, "< %.3f, 0, 0 >\n", - aspect_ratio );
	fprintf( srs, "\t\tangle %.2f\n", cam->fov );
	fprintf( srs, "\t\tlook_at " );
	convert_to_srs_cs( &p, &cam->target );
	fprintf( srs, "< %.3f, %.3f, %.3f >\n", p.x, p.y, p.z );
	fprintf( srs, "\t}\n" );
}


void
write_srs_sphere( FILE *srs, point *center, float radius )
{
	fprintf( srs, "\t\tsphere {\n" );
	fprintf( srs, "\t\t\t< %.3f, %.3f, %.3f >,\n", center->x, center->y, center->z );
	fprintf( srs, "\t\t\t%.3f\n", radius );
	fprintf( srs, "\t\t}\n" );
}


void
write_srs_cylinder( FILE *srs, point *p1, point *p2, float radius )
{
	fprintf( srs, "\t\tcylinder {\n" );
	fprintf( srs, "\t\t\t< %.3f, %.3f, %.3f >,\n", p1->x, p1->y, p1->z );
	fprintf( srs, "\t\t\t< %.3f, %.3f, %.3f >,\n", p2->x, p2->y, p2->z );
	fprintf( srs, "\t\t\t%.3f\n", radius );
	fprintf( srs, "\t\t}\n" );
}


void
write_srs_smooth_triangle( FILE *srs, point **vertices, point **normals )
{
	point vert_srs, norm_srs;
	int i;

	fprintf( srs, "\t\tsmooth_triangle {\n" );
	for (i = 0; i < 3; i++) {
		convert_to_srs_cs( &vert_srs, vertices[i] );
		fprintf( srs, "\t\t\t< %.4f, %.4f, %.4f >,\n", vert_srs.x, vert_srs.y, vert_srs.z );
		convert_to_srs_cs( &norm_srs, normals[i] );
		fprintf( srs, "\t\t\t    < %.4f, %.4f, %.4f >", norm_srs.x, norm_srs.y, norm_srs.z );
		if (i < 2)
			fprintf( srs, ",\n" );
		else
			fprintf( srs, "\n" );
	}
	fprintf( srs, "\t\t}\n" );
}


void
write_srs_pigment( FILE *srs, rgb_color *color )
{
	float r,g,b;

	fprintf( srs, "\t\tpigment {\n" );
	fprintf( srs, "\t\t\tcolour rgb " );
	r = color->r;
	g = color->g;
	b = color->b;
	fprintf( srs, "< %.3f, %.3f, %.3f >\n", r, g, b );
	fprintf( srs, "\t\t}\n" );
}


/* Converts a point from our coordinate system (right-handed, z-axis up) to the
 * BACKLIGHT/SRS coordinate system (right-handed, y-axis up) */
void
convert_to_srs_cs( point *p_srs, point *p_ls )
{
	p_srs->x = p_ls->x;
	p_srs->y = p_ls->z;
	p_srs->z = - p_ls->y;
}

#endif /* WITH_SRS_EXPORTER */

/* end geometry.c */
