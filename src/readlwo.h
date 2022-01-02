/* readlwo.h */

/*
 * Copyright (C) 1998 Janne Löf <jlof@mail.student.oulu.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This is a heavily modified version of Janne's LWO reader library, as
 * included in the GtkGLArea distribution (gtkglarea-1.0/examples/lw.h)
 */


#include <string.h>
#include <glib.h>

#define LW_MAX_NAME_LEN 64

typedef struct {
	char name[LW_MAX_NAME_LEN];
	float r, g, b;
} lwMaterial;

#ifndef lwPoint
typedef struct {
	float x;
	float y;
	float z;
} lwPoint;
#endif

typedef struct {
	int mat_id;		/* material of this face */
	int index_cnt;		/* number of vertices */
	int *indices;		/* index to vertex */
	float *texcoords;	/* u,v texture coordinates (not used) */
} lwFace;

typedef struct {
	int face_cnt;
	lwFace *faces;

	int material_cnt;
	lwMaterial *materials;

	int vertex_cnt;
	lwPoint *vertices;

} lwObject;

/* Wrappers for the system memory management functions */
extern void *xmalloc( size_t size );
extern void *xrealloc( void *block, size_t size );
extern char *xstrdup( const char *str );
extern void xfree( void *ptr );

gint      lw_is_lwobject(const char *lw_file);
lwObject *lw_object_read(const char *lw_file);
void      lw_object_free(lwObject *lw_object);
void      lw_object_show(const lwObject *lw_object);

float lw_object_radius(const lwObject *lw_object);
void  lw_object_scale (lwObject *lw_object, float scale);

/* end readlwo.h */
