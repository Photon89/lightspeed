/* importobjs.c */

/* Imports 3DS/LWO objects to our native format */

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

/* 3D Studio reader routines */
#include "read3ds.h"

/* LightWave reader routines */
#define lwPoint point
#include "readlwo.h"


/* Used by tessellate_object( ) */
struct face_info {
	int adj_faces[3];
	int adj_face_edges[3];
int edgewise : 1;
};


/* Forward declarations */
static int import_3ds_file( const char *filename );
static int import_3ds( r3ds_scene *scene );
static int import_lwo_file( const char *filename );
static void generate_normals( ogl_object *obj, int face_type );
static void tessellate_object( ogl_object *obj, int tess );
static unsigned int tessellate_face( ogl_object *obj, struct face_info **facei_list_ptr, int f, int e, unsigned int ind_mid );
static void triangulate_polygon( r3ds_triangle *out_tris, int num_edges, int *indices, point *vertices );


int
import_objects( const char *filename )
{
	int rs = -1;
	int len;
	char *file_ext;
	char *error_str;

	/* Confirm that the file exists */
	if (!file_exists( filename )) {
		len = strlen( STR_MSG_no_object_file_ARG ) + strlen( filename ) + 16;
		error_str = xmalloc( len * sizeof(char) );
		sprintf( error_str, STR_MSG_no_object_file_ARG, filename );
		message_window( STR_DLG_Error, error_str );
		xfree( error_str );
		return -1;
	}

	file_ext = strrchr( filename, '.' );
	if (file_ext == NULL)
		file_ext = "";

	/* Dispatch a file loader, based on the file's extension */
	if (!strcasecmp( file_ext, ".3DS" )) {
		if (r3ds_file_is_3ds( filename ))
			rs = import_3ds_file( filename );
		else {
			message_window( STR_DLG_Error, STR_MSG_not_3ds_file );
			return -1;
		}
	}
	else if (!strcasecmp( file_ext, ".PRJ" )) {
		if (r3ds_file_is_prj( filename ))
			rs = import_3ds_file( filename );
		else {
			message_window( STR_DLG_Error, STR_MSG_not_prj_file );
			return -1;
		}
	}
	else if (!strcasecmp( file_ext, ".LWO" )) {
		if (lw_is_lwobject( filename ))
			rs = import_lwo_file( filename );
		else {
			message_window( STR_DLG_Error, STR_MSG_not_lwo_file );
			return -1;
		}
	}
	else {
		/* Looks like we don't have an extension to help us
		 * Try file magic numbers */
		if (r3ds_file_is_3ds( filename ))
			rs = import_3ds_file( filename );
		else if (r3ds_file_is_prj( filename ))
			rs = import_3ds_file( filename );
		else if (lw_is_lwobject( filename ))
			rs = import_lwo_file( filename );
		else {
			/* Whatever it is, we can't read it */
			message_window( STR_DLG_Error, STR_MSG_unknown_obj_format );
			return -1;
		}
	}

	return rs;
}


static int
import_3ds_file( const char *filename )
{
	r3ds_scene *scene;

	/* Load .3DS */
	scene = read3ds( filename );
	if (scene == NULL) {
		message_window( STR_DLG_Error, STR_MSG_bad_3ds_file );
		return -1;
	}

	return import_3ds( scene );
}


static int
import_3ds( r3ds_scene *scene )
{
	r3ds_trimesh **trimeshes;
	r3ds_trimesh *tmesh;
	ogl_object *obj;
	point *vert_a, *vert_b, *vert_c;
	point *tri_cent;
	point centroid = { 0.0, 0.0, 0.0 };
	r3ds_color24 *mat_color24;
	rgb_color color;
	float scale_factor;
	float x0,y0,z0;
	float x,y,z;
	float tri_area;
	float tris_total_area = 0.0;
	float xmax = -1E6, ymax = -1E6, zmax = -1E6;
	float xmin = 1E6, ymin = 1E6, zmin = 1E6;
	int num_trimeshes;
	int num_bad_trimeshes = 0;
	int num_vertices;
	int num_indices;
	int num_bad_tris;
	int cur_index;
	int a,b,c;
	int o, v, i, t; /* Mike, you have a call */

	/* Break objects apart by material
	 * (each ogl_object ultimately has one base color) */
	r3ds_split_scene_trimeshes( scene, R3DS_SPLIT_BY_MATERIAL );

	/* We can't really handle objects with >1 smoothing group,
	 * so break those apart as necessary */
	r3ds_split_scene_trimeshes( scene, R3DS_SPLIT_BY_SMGROUP );

	trimeshes = scene->tmeshes;
	num_trimeshes = scene->num_tmeshes;
	vehicle_objs = xmalloc( num_trimeshes * sizeof(ogl_object *) );

	/* Make model metric (from inches) */
	scale_factor = scene->inches_per_unit * 0.0254;

	/* Now, convert each [good] r3ds_trimesh into an ogl_object */
	for (o = 0; o < num_trimeshes; o++) {
		tmesh = trimeshes[o];
		if ((tmesh->num_verts < 3) || (tmesh->num_tris < 1)) {
			++num_bad_trimeshes;
			continue;
		}

		/* Material of 1st triangle == material of entire object */
		i = tmesh->tris[0].mat_id;
		if (i >= 0) {
			mat_color24 = &scene->mats[i].diffuse;
			color.r = (float)(mat_color24->red) / 255.0;
			color.g = (float)(mat_color24->green) / 255.0;
			color.b = (float)(mat_color24->blue) / 255.0;
		}
		else {
			color.r = 0.5;
			color.g = 0.5;
			color.b = 0.5;
		}

		num_vertices = tmesh->num_verts;
		num_indices = 3 * tmesh->num_tris;
		obj = alloc_ogl_object( num_vertices, num_indices );
		obj->type = GL_TRIANGLES;
		obj->color0.r = color.r;
		obj->color0.g = color.g;
		obj->color0.b = color.b;

		for (v = 0; v < num_vertices; v++) {
			x0 = tmesh->verts[v].x;
			y0 = tmesh->verts[v].y;
			z0 = tmesh->verts[v].z;

			/* Scale object, and rotate to x-axial alignment */
			x = - y0 * scale_factor;
			y = x0 * scale_factor;
			z = z0 * scale_factor;

			obj->vertices0[v].x = x;
			obj->vertices0[v].y = y;
			obj->vertices0[v].z = z;

			/* Normals will be taken care of shortly... */
		}

		/* Copy triangles, and at the same time sum up the weighed
		 * triangle centroids to get an overall centroid later on */
		cur_index = 0;
		num_bad_tris = 0;
		for (t = 0; t < tmesh->num_tris; t++) {
			a = tmesh->tris[t].a;
			b = tmesh->tris[t].b;
			c = tmesh->tris[t].c;

			vert_a = &obj->vertices0[a];
			vert_b = &obj->vertices0[b];
			vert_c = &obj->vertices0[c];
			tri_cent = calc_tri_centroid( vert_a, vert_b, vert_c );
			tri_area = calc_tri_area( vert_a, vert_b, vert_c );
			centroid.x += tri_cent->x * tri_area;
			centroid.y += tri_cent->y * tri_area;
			centroid.z += tri_cent->z * tri_area;
			tris_total_area += tri_area;

			if (tri_area > 1E-8) {
				obj->indices[cur_index++] = a;
				obj->indices[cur_index++] = b;
				obj->indices[cur_index++] = c;
			}
			else
				++num_bad_tris; /* very VERY tiny triangle */
		}
		if (num_bad_tris > 0) {
			/* Readjust index array size */
			obj->num_indices -= 3 * num_bad_tris;
			obj->indices = xrealloc( obj->indices, obj->num_indices * sizeof(int) );
		}

		/* No longer need the trimesh
		 * Partially free the big arrays to economize on memory
		 * r3ds_free_scene( ) will finish off the rest, shortly */
		tmesh->verts = xrealloc( tmesh->verts, sizeof(r3ds_point) );
		if (tmesh->num_tris > 1)
			tmesh->tris = xrealloc( tmesh->tris, sizeof(r3ds_triangle) );

		generate_normals( obj, GL_TRIANGLES );

		/* This ogl_object is ready */
		vehicle_objs[o - num_bad_trimeshes] = obj;
	}

	/* Deallocate scene */
	r3ds_free_scene( scene );

	num_vehicle_objs = num_trimeshes - num_bad_trimeshes;
	if (num_vehicle_objs == 0) {
		message_window( STR_DLG_Error, STR_MSG_empty_3ds_file );
		return -1;
	}

	/* Calculate centroid */
	centroid.x /= tris_total_area;
	centroid.y /= tris_total_area;
	centroid.z /= tris_total_area;

	/* Center the model (make world origin and centroid coincide) */
	for (o = 0; o < num_vehicle_objs; o++) {
		obj = vehicle_objs[o];
		for (v = 0; v < obj->num_vertices; v++) {
			obj->vertices0[v].x -= centroid.x;
			obj->vertices0[v].y -= centroid.y;
			obj->vertices0[v].z -= centroid.z;

			/* Update xyz extents */
			x = obj->vertices0[v].x;
			y = obj->vertices0[v].y;
			z = obj->vertices0[v].z;
			xmin = MIN(x, xmin);
			xmax = MAX(x, xmax);
			ymin = MIN(y, ymin);
			ymax = MAX(y, ymax);
			zmin = MIN(z, zmin);
			zmax = MAX(z, zmax);
		}
	}

	vehicle_extents.xmin = xmin;
	vehicle_extents.xmax = xmax;
	vehicle_extents.ymin = ymin;
	vehicle_extents.ymax = ymax;
	vehicle_extents.zmin = zmin;
	vehicle_extents.zmax = zmax;
	vehicle_extents.avg = ((xmax - xmin) + (ymax - ymin) + (zmax - zmin)) / 3;

	/* Finally, tessellate the objects so that they deform nicely */
	for (o = 0; o < num_vehicle_objs; o++)
		tessellate_object( vehicle_objs[o], 8 );

	return 0;
}


/* This next importer basically converts a LightWave object into 3DS data using
 * the same r3ds_build( ) of read3ds. The only additional thing it needs is
 * a polygon triangulator, as LW objects can have arbitrary n-sided faces */
static int
import_lwo_file( const char *filename )
{
	r3ds_scene *scene;
	lwObject *lwo;
	lwFace *face;
	r3ds_triangle poly_tris_buffer[256];
	r3ds_triangle *poly_tris;
	r3ds_triangle *tri;
	int abcf[4];
	int rgb[3];
	float xyz[3];
	int num_tris;
	int num_poly_tris;
	int mat_id = -1;
	int i, j;

	/* Read LWO file */
	lwo = lw_object_read( filename );
	if (lwo == NULL) {
		message_window( STR_DLG_Error, STR_MSG_bad_lwo_file );
		return -1;
	}

	/* Initialize read3ds scene builder */
	r3ds_build( R3DS_INITIALIZE, NULL );

	/* Input materials */
	for (i = 0; i < lwo->material_cnt; i++) {
		r3ds_build( R3DS_NEW_MATERIAL, lwo->materials[i].name );
		r3ds_build( R3DS_MAT_DIFFUSE_COLOR, NULL );
		rgb[0] = (int)(lwo->materials[i].r * 255.0);
		rgb[1] = (int)(lwo->materials[i].g * 255.0);
		rgb[2] = (int)(lwo->materials[i].b * 255.0);
		r3ds_build( R3DS_COLOR24, rgb );
	}

	/* Input object mesh */

	r3ds_build( R3DS_NEW_OBJECT, filename );
	r3ds_build( R3DS_DEF_TRIMESH, NULL );

	/* Input vertices */
	r3ds_build( R3DS_NUM_VERTS, &lwo->vertex_cnt );
	for (i = 0; i < lwo->vertex_cnt; i++) {
		/* Rotate coordinate system to 3DS standard */
		xyz[0] = lwo->vertices[i].x;
		xyz[1] = - lwo->vertices[i].z;
		xyz[2] = lwo->vertices[i].y;
		r3ds_build( R3DS_VERT, xyz );
	}

	/* Count how many triangles there will be */
	num_tris = 0;
	for (i = 0; i < lwo->face_cnt; i++) {
		face = &lwo->faces[i];
		/* (Quickly double-check that the indices are within bounds) */
		for (j = 0; j < face->index_cnt; j++)
			if (face->indices[j] >= lwo->vertex_cnt)
				face->indices[j] = 0;
		/* Faces with <3 sides are not faces */
		if (face->index_cnt < 3)
			continue;
		/* Otherwise, an n-sided polygon will become (n-2) triangles */
		num_tris += (face->index_cnt - 2);
	}
	r3ds_build( R3DS_NUM_TRIS, &num_tris );

	/* Process faces and input triangles */
	num_tris = 0;
	for (i = 0; i < lwo->face_cnt; i++) {
		face = &lwo->faces[i];
		if (face->index_cnt < 3)
			continue;
		if (mat_id != face->mat_id) {
			mat_id = face->mat_id;
			r3ds_build( R3DS_TRI_MATERIAL_CURRENT, lwo->materials[mat_id].name );
		}
		if (face->index_cnt == 3) {
			abcf[0] = face->indices[0];
			abcf[1] = face->indices[1];
			abcf[2] = face->indices[2];
			abcf[3] = 0x07; /* Flags: all edges visible */
			r3ds_build( R3DS_TRI_FACE, abcf );
			r3ds_build( R3DS_TRI_MATERIAL, &num_tris );
			++num_tris;
		}
		else {
			num_poly_tris = face->index_cnt - 2;
			/* Avoid small malloc()'s at all costs (well, almost) */
			if (num_poly_tris <= 256)
				poly_tris = poly_tris_buffer;
			else
				poly_tris = xmalloc( num_poly_tris * sizeof(r3ds_triangle) );
			triangulate_polygon( poly_tris, face->index_cnt, face->indices, lwo->vertices );
			for (j = 0; j < num_poly_tris; j++) {
				tri = &poly_tris[j];
				abcf[0] = tri->a;
				abcf[1] = tri->b;
				abcf[2] = tri->c;
				abcf[3] = 0x07;
				r3ds_build( R3DS_TRI_FACE, abcf );
				r3ds_build( R3DS_TRI_MATERIAL, &num_tris );
				++num_tris;
			}
			if (poly_tris != poly_tris_buffer)
				xfree( poly_tris );
		}
	}

	lw_object_free( lwo );

	scene = xmalloc( sizeof(r3ds_scene) );
	r3ds_build( R3DS_GET_SCENE, scene );

	import_3ds( scene );

	return 0;
}


/* Produce normals for an arbitrary triangle or quad mesh
 * (either is referred to as a generalized "face") */
static void
generate_normals( ogl_object *obj, int face_type )
{
	struct obj_vertex {
		int num_faces;		/* # of faces using this vertex */
		point normal_sum;	/* Vector sum of normals of said faces */
	} *overtices, *overtex;
	point *plane[3];
	point *normal;
	float x, y, z;
	float d;
	int face_size;
	int num_faces;
	int num_vertices;
	int base;
	int f, v, i;

	switch (face_type) {
	case GL_TRIANGLES:
		face_size = 3;
		break;

	case GL_QUADS:
		face_size = 4;
		break;

	default:
#ifdef DEBUG
		crash( "generate_normals( ): invalid face type" );
#endif
		return;
	}

	num_faces = obj->num_indices / face_size;

	num_vertices = obj->num_vertices;
	overtices = xmalloc( num_vertices * sizeof(struct obj_vertex) );
	/* Initialize vertex records */
	for (v = 0; v < num_vertices; v++) {
		overtex = &overtices[v];
		overtex->num_faces = 0;
		overtex->normal_sum.x = 0.0;
		overtex->normal_sum.y = 0.0;
		overtex->normal_sum.z = 0.0;
	}

	/* Step 1: Get vector sum of [face] normals for each vertex */
	for (f = 0; f < num_faces; f++) {
		base = f * face_size; /* # of first index describing face */

		/* Calculate face normal (need only 3 points for this) */
		for (v = 0; v < 3; v++) {
			i = obj->indices[base + v];
			plane[v] = &obj->vertices0[i];
		}
		normal = calc_tri_normal( plane[0], plane[1], plane[2] );

		/* Update the 3 or 4 involved vertices */
		for (v = 0; v < face_size; v++) {
			i = obj->indices[base + v];
			overtex = &overtices[i];
			++overtex->num_faces;
			overtex->normal_sum.x += normal->x;
			overtex->normal_sum.y += normal->y;
			overtex->normal_sum.z += normal->z;
		}
	}

	/* Step 2: Vertex normal = normalized sum of associated face normals */
	for (v = 0; v < num_vertices; v++) {
		overtex = &overtices[v];

		x = overtex->normal_sum.x;
		y = overtex->normal_sum.y;
		z = overtex->normal_sum.z;

		d = sqrt( SQR(x) + SQR(y) + SQR(z) );
		if (d < 1E-6)
			d = 1.0;
		obj->normals0[v].x = x / d;
		obj->normals0[v].y = y / d;
		obj->normals0[v].z = z / d;
	}

	xfree( overtices );
}


/* This reduces the face edge size (in the yz-plane) of a GL_TRIANGLES object
 * to below a certain threshold specified by tess (1/tess times the y- and
 * z-extents), with the exception that any edges shared between faces mostly
 * edgewise to the yz-plane are not split. This is what keeps, for example,
 * the tops and sides of a cube from being as heavily tessellated as the front
 * and back
 * NOTE: tess must be a power of 2 */
static void
tessellate_object( ogl_object *obj, int tess )
{
	struct edge_info {
		/* Offset into edgei_list == ind_a */
		unsigned int ind_b;
		int face;
		int edge;
		struct edge_info *next;
	} **edgei_list, *prev_edgei, *edgei;
	struct face_info *facei_list;
	struct face_info *facei, *facei_adj, *facei_new, *facei_adj_new;
	point *vert_a, *vert_b;
	point *p[3], *norm;
	float max_ylen, max_zlen;
	float long_ylen = 0.0, long_zlen = 0.0;
	float yzlen2, long_yzlen2;
	float dy, dz;
	unsigned int ind_a, ind_b, ind_c, ind_d, ind_mid;
	unsigned int a, b, i;
	int num_faces_orig;
	int num_faces;
	int tess_level;
	int base, base_adj;
	int f, f_adj, f_new, f_adj_new;
	int e, e_adj, e_long;
	int v;

#ifdef DEBUG
	printf( "Tessellating object..." );
	fflush( stdout );
#endif

	num_faces_orig = obj->num_indices / 3;
	num_faces = num_faces_orig;

	/* Initialize face info list */
	facei_list = xmalloc( num_faces * sizeof(struct face_info) );
	for (f = 0; f < num_faces; f++) {
		facei = &facei_list[f];

		/* No adjacent faces (yet) */
		for (e = 0; e < 3; e++) {
			facei->adj_faces[e] = -1;
			facei->adj_face_edges[e] = -1;
		}

		/* Check x-component of face normal to see if face is
		 * edgewise to the yz-plane or not */
		base = 3 * f;
		for (v = 0; v < 3; v++) {
			i = obj->indices[base + v];
			p[v] = &obj->vertices0[i];
		}
		norm = calc_tri_normal( p[0], p[1], p[2] );
		if (ABS(norm->x) < 0.125)
			facei->edgewise = TRUE; /* Edgewise */
		else
			facei->edgewise = FALSE; /* Not edgewise */
	}

	/* Initialize edge info list */
	edgei_list = xmalloc( obj->num_vertices * sizeof(struct edge_info *) );
	for (e = 0; e < obj->num_vertices; e++)
		edgei_list[e] = NULL;

	/* Find all face adjacencies by searching for coincident edges */
	for (f = 0; f < num_faces; f++) {
		base = 3 * f;
		for (e = 0; e < 3; e++) {
			a = obj->indices[base + e];
			b = obj->indices[base + ((e + 1) % 3)];
			ind_a = MIN(a, b);
			ind_b = MAX(a, b);

			/* Search for matching edge */
			prev_edgei = NULL;
			edgei = edgei_list[ind_a];
			while (edgei != NULL) {
				if (edgei->ind_b == ind_b)
					break; /* found */
				else {
					prev_edgei = edgei;
					edgei = edgei->next; /* check next one */
				}
			}
			if (edgei != NULL) {
				/* Coincident edge found; update adjacency info */
				f_adj = edgei->face;
				e_adj = edgei->edge;
				/* First face */
				facei = &facei_list[f];
				facei->adj_faces[e] = f_adj;
				facei->adj_face_edges[e] = e_adj;
				/* Second face */
				facei_adj = &facei_list[f_adj];
				facei_adj->adj_faces[e_adj] = f;
				facei_adj->adj_face_edges[e_adj] = e;
				/* and free edge record
				 * (b/c at most two faces can share an edge)
				 * (barring bad geometry, anyway) */
				if (prev_edgei != NULL)
					prev_edgei->next = edgei->next;
				else
					edgei_list[ind_a] = edgei->next;
				xfree( edgei );
			}
			else {
				/* Add new edge record */
				edgei = xmalloc( sizeof(struct edge_info) );
				edgei->ind_b = ind_b;
				edgei->face = f;
				edgei->edge = e;
				edgei->next = NULL;
				if (prev_edgei != NULL)
					prev_edgei->next = edgei;
				else
					edgei_list[ind_a] = edgei;
			}

		}

	}

	/* Free the edge info list, no longer need it */
	for (v = 0; v < obj->num_vertices; v++) {
		edgei = edgei_list[v];
		while (edgei != NULL) {
			prev_edgei = edgei;
			edgei = edgei->next;
			xfree( prev_edgei );
		}
	}
	xfree( edgei_list );

	/* Perform tessellation incrementally, reducing threshold size
	 * one step at a time (else we get non-clean results) */
	for (tess_level = 2; tess_level <= tess; tess_level *= 2) {
		max_ylen = 1.01 * (vehicle_extents.ymax - vehicle_extents.ymin) / (float)tess_level;
		max_zlen = 1.01 * (vehicle_extents.zmax - vehicle_extents.zmin) / (float)tess_level;

		for (f = 0; f < num_faces; f++) {
			/* Find which edge is most eligible to be split. This
			 * is the longest one, as measured in the yz-plane,
			 * that is not shared between two edgewise faces */
			facei = &facei_list[f];
			base = 3 * f;
			e_long = -1;
			long_yzlen2 = -1.0;
			for (e = 0; e < 3; e++) {
				/* First, check for exception case */
				f_adj = facei->adj_faces[e];
				if (f_adj >= 0) {
					facei_adj = &facei_list[f_adj];
					if (facei->edgewise && facei_adj->edgewise) {
						/* Both faces are edgewise
						 * This edge must not be split */
						continue;
					}
				}

				a = obj->indices[base + e];
				b = obj->indices[base + ((e + 1) % 3)];
				ind_a = MIN(a, b);
				ind_b = MAX(a, b);
				vert_a = &obj->vertices0[ind_a];
				vert_b = &obj->vertices0[ind_b];
				dy = ABS(vert_a->y - vert_b->y);
				dz = ABS(vert_a->z - vert_b->z);
				yzlen2 = SQR(dy) + SQR(dz);
				if (yzlen2 > long_yzlen2) {
					e_long = e;
					long_ylen = dy;
					long_zlen = dz;
					long_yzlen2 = yzlen2;
				}
			}

			if (e_long == -1)
				continue; /* Can't split this face */

			/* See if long edge doesn't exceed split threshold */
			if ((long_ylen <= max_ylen) && (long_zlen <= max_zlen))
				continue;

			/* Tessellate the face and update face info */
			ind_mid = tessellate_face( obj, &facei_list, f, e_long, 0 );
			facei = &facei_list[f]; /* facei_list might have moved! */
			f_new = num_faces;
			++num_faces;

			/* If another face is sharing the same [long] edge,
			 * tessellate it too */
			f_adj = facei->adj_faces[e_long];
			if (f_adj >= 0) {
				base_adj = 3 * f_adj;
				e_adj = facei->adj_face_edges[e_long];
				tessellate_face( obj, &facei_list, f_adj, e_adj, ind_mid );
				facei = &facei_list[f];
				f_adj_new = num_faces;
				++num_faces;

				/* Check for crossed adjacencies */
				a = obj->indices[base + e_long];
				b = obj->indices[base + ((e_long + 1) % 3)];
				ind_a = MIN(a, b);
				ind_b = MAX(a, b);
				a = obj->indices[base_adj + e_adj];
				b = obj->indices[base_adj + ((e_adj + 1) % 3)];
				ind_c = MIN(a, b);
				ind_d = MAX(a, b);
				if ((ind_a == ind_c) && (ind_b == ind_d)) {
					/* The old faces are adjacent--
					 * this shouldn't happen!
					 * (means the normals disagree, often
					 * the case with sloppy geometry) */
					facei_new = &facei_list[f_new];
					facei_adj_new = &facei_list[f_adj_new];
					/* Correct adjacencies of the new faces */
					facei_new->adj_faces[e_long] = f_adj_new;
					facei_adj_new->adj_faces[e_adj] = f_new;
				}
				else {
					/* The old and new faces are staggered
					 * (as they should be) */
					facei_adj = &facei_list[f_adj];
					/* Correct adjacencies of the old faces */
					facei->adj_faces[e_long] = f_adj_new;
					facei_adj->adj_faces[e_adj] = f_new;
				}
			}

			/* Might have to split current face again */
			--f;
		}
	}

	xfree( facei_list );

#ifdef DEBUG
	printf( "done. (%d face splits)\n", num_faces - num_faces_orig );
	fflush( stdout );
#endif
}


/* Turns a specified face (f) in an object into two, bisecting the specified
 * edge (e), using the specified midpoint vertex (ind_mid) if it is >0
 * Note: edges are numbered 0-2, *not* on basis of opposite vertex but in
 * same order as vertex enumeration */
static unsigned int
tessellate_face( ogl_object *obj, struct face_info **facei_list_ptr, int f, int e, unsigned int ind_mid )
{
	struct face_info *facei, *facei_new;
	struct face_info *facei_other;
	point *vert_a, *vert_b;
	point *norm_a, *norm_b;
	point *vert_mid, *norm_mid;
	unsigned int ind_a, ind_b;
	unsigned int ind_split, ind_move;
	int num_faces0;
	int base, base_new;
	int v_split, v_move, v_third;
	int f_other, e_other;
	int n;

	num_faces0 = obj->num_indices / 3; /* this value will not be incremented */
	base = 3 * f;

	/* A value of 0 for index_mid means no midpoint vertex exists
	 * Thus we create it here */
	if (ind_mid == 0) {
		/* Indices of endpoints and new midpoint vertex */
		ind_a = obj->indices[base + e];
		ind_b = obj->indices[base + ((e + 1) % 3)];
		ind_mid = obj->num_vertices;

		/* Add new vertex and normal */
		++obj->num_vertices;
		n = obj->num_vertices;
		obj->vertices0 = xrealloc( obj->vertices0, n * sizeof(point) );
		obj->normals0 = xrealloc( obj->normals0, n * sizeof(point) );
		obj->iarrays = xrealloc( obj->iarrays, n * sizeof(ogl_point) );

		/* New vertex location (midpoint of long edge) */
		vert_a = &obj->vertices0[ind_a];
		vert_b = &obj->vertices0[ind_b];
		vert_mid = &obj->vertices0[ind_mid];
		vert_mid->x = (vert_a->x + vert_b->x) / 2.0;
		vert_mid->y = (vert_a->y + vert_b->y) / 2.0;
		vert_mid->z = (vert_a->z + vert_b->z) / 2.0;

		/* New vertex normal (average of endpoint normals) */
		norm_a = &obj->normals0[ind_a];
		norm_b = &obj->normals0[ind_b];
		norm_mid = &obj->normals0[ind_mid];
		norm_mid->x = (norm_a->x + norm_b->x) / 2.0;
		norm_mid->y = (norm_a->y + norm_b->y) / 2.0;
		norm_mid->z = (norm_a->z + norm_b->z) / 2.0;
	}

	/* # and index of vertex from which split begins
	 * (i.e. the vertex opposite the bisected edge) */
	v_split = (e + 2) % 3;
	ind_split = obj->indices[base + v_split];
	/* # and index of vertex to move to midpoint of long edge */
	v_move = e;
	ind_move = obj->indices[base + v_move];
	/* # of the other (third) vertex */
	v_third = (e + 1) % 3;

	/* First triangle half (modify original face) */
	obj->indices[base + v_move] = ind_mid;

	/* Second triangle half (create new face) */
	base_new = obj->num_indices;
	obj->num_indices += 3;
	obj->indices = xrealloc( obj->indices, obj->num_indices * sizeof(unsigned int) );
	obj->indices[base_new + v_split] = ind_split;
	obj->indices[base_new + v_move] = ind_move;
	obj->indices[base_new + v_third] = ind_mid;

	/* Make new entry in the face info list, and update the adjacencies */

	*facei_list_ptr = xrealloc( *facei_list_ptr, (num_faces0 + 1) * sizeof(struct face_info) );
	facei = &(*facei_list_ptr)[f];
	facei_new = &(*facei_list_ptr)[num_faces0];
	memcpy( facei_new, facei, sizeof(struct face_info) );

	/* The old face is next to the new face... */
	facei->adj_faces[v_split] = num_faces0;
	facei->adj_face_edges[v_split] = v_third;
	/* ...and the new face is next to the old face */
	facei_new->adj_faces[v_third] = f;
	facei_new->adj_face_edges[v_third] = v_split;

	/* If the new face has a neighbor (that isn't sharing the splitted edge),
	 * it will need updating too */
	f_other = facei_new->adj_faces[v_split];
	if (f_other >= 0) {
		e_other = facei_new->adj_face_edges[v_split];
		facei_other = &(*facei_list_ptr)[f_other];
		facei_other->adj_faces[e_other] = num_faces0;
		/* facei_other->adj_face_edges[e_other] = v_split; */
		/* (last line unnecessary, by splitting convention used) */
	}

	return ind_mid;
}


/* Simple polygon triangulator, should be able to handle anything save for
 * self-intersecting faces (garbage_in == garbage_out) */
/* TODO: Still buggy, FIX THIS!!! */
static void
triangulate_polygon( r3ds_triangle *out_tris, int num_edges, int *indices, point *vertices )
{
	r3ds_triangle *tri;
	extents ext = { 1E6, -1E6, 1E6, -1E6, 1E6, -1E6, NIL };
	float x, y, z;
	int indices2_buffer[256];
	int *indices2;
	int index_xmax_num = 0, index_ymax_num = 0, index_zmax_num = 0;
	int ind, index_num;
	int i, j;

	/* Find face extents, and indices of extrema */
	for (i = 0; i < num_edges; i++) {
		ind = indices[i];
		x = vertices[ind].x;
		y = vertices[ind].y;
		z = vertices[ind].z;
		if (x < ext.xmin)
			ext.xmin = x;
		if (x > ext.xmax) {
			ext.xmax = x;
			index_xmax_num = i;
		}
		if (y < ext.ymin)
			ext.ymin = y;
		if (y > ext.ymax) {
			ext.ymax = y;
			index_ymax_num = i;
		}
		if (z < ext.zmin)
			ext.zmin = z;
		if (z > ext.zmax) {
			ext.zmax = z;
			index_zmax_num = i;
		}
	}

	/* Determine first vertex of triangle based on pricipal face alignment */
	x = ext.xmax - ext.xmin;
	y = ext.ymax - ext.ymin;
	z = ext.zmax - ext.zmin;
	index_num = index_xmax_num;
	if (y > x)
		index_num = index_ymax_num;
	if ((z > x) && (z > y))
		index_num = index_zmax_num;

	/* Create one output triangle, now knowing that index_num references
	 * a convex vertex of the polygon */
	tri = &out_tris[0];
	tri->a = indices[index_num];
	tri->b = indices[(index_num + 1) % num_edges];
	tri->c = indices[(index_num - 1 + num_edges) % num_edges];

	/* and recurse if there are more to do */
	if (num_edges > 3) {
		if (num_edges <= 256)
			indices2 = indices2_buffer;
		else
			indices2 = xmalloc( (num_edges - 1) * sizeof(int) );
		j = 0;
		for (i = 0; i < num_edges; i++)
			if (i != index_num)
				indices2[j++] = indices[i];
		triangulate_polygon( &out_tris[1], num_edges - 1, indices2, vertices );
		if (indices2 != indices2_buffer)
			xfree( indices2 );
	}
}

/* end importobjs.c */
