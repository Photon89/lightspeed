/* readlwo.c */

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
 * included in the GtkGLArea distribution (gtkglarea-1.0/examples/lw.c)
 */


#include <stdio.h>
#include <math.h>
#include "readlwo.h"

#define MK_ID(a,b,c,d) ((((guint32)(a))<<24)| \
(((guint32)(b))<<16)| \
(((guint32)(c))<< 8)| \
(((guint32)(d))    ))

#define ID_FORM MK_ID('F','O','R','M')
#define ID_LWOB MK_ID('L','W','O','B')
#define ID_PNTS MK_ID('P','N','T','S')
#define ID_SRFS MK_ID('S','R','F','S')
#define ID_SURF MK_ID('S','U','R','F')
#define ID_POLS MK_ID('P','O','L','S')
#define ID_COLR MK_ID('C','O','L','R')

static gint32
read_char(FILE *f)
{
	gint32 c;
	c = fgetc(f);
	g_return_val_if_fail(c != EOF, 0);
	return c;
}

static gint32
read_short(FILE *f)
{
	guint8 bytes[2];
	bytes[0] = read_char(f);
	bytes[1] = read_char(f);
	return (bytes[0] << 8) | bytes[1];
}

static gint32
read_long(FILE *f)
{
	guint8 bytes[4];
	bytes[0] = read_char(f);
	bytes[1] = read_char(f);
	bytes[2] = read_char(f);
	bytes[3] = read_char(f);
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

static float
read_float(FILE *f)
{
	gint32 x;
	x = read_long(f);
	return *(float *)&x;
}

static gint
read_string(FILE *f, char *str)
{
	gint c;
	gint cnt = 0;
	do {
		c = read_char(f);
		if (cnt < LW_MAX_NAME_LEN)
			str[cnt] = c;
		else
			str[LW_MAX_NAME_LEN - 1] = 0;
		cnt++;
	} while (c != 0);
	/* if length of string (including \0) is odd skip another byte */
	if (cnt % 2) {
		read_char(f);
		cnt++;
	}
	return cnt;
}

static void
read_srfs(FILE *f, gint nbytes, lwObject *lwo)
{
	int guess_cnt = lwo->material_cnt;

	while (nbytes > 0) {
		lwMaterial *material;

		/* allocate more memory for materials if needed */
		if (guess_cnt <= lwo->material_cnt) {
			guess_cnt += guess_cnt / 2 + 4;
			lwo->materials = xrealloc(lwo->materials, sizeof(lwMaterial) * guess_cnt);
		}
		material = lwo->materials + lwo->material_cnt++;

		/* read name */
		nbytes -= read_string(f, material->name);

		/* defaults */
		material->r = 0.5;
		material->g = 0.5;
		material->b = 0.5;
	}
	lwo->materials = xrealloc(lwo->materials, sizeof(lwMaterial) * lwo->material_cnt);
}


static void
read_surf(FILE *f, gint nbytes, lwObject *lwo)
{
	int i;
	char name[LW_MAX_NAME_LEN];
	lwMaterial *material = NULL;

	/* read surface name */
	nbytes -= read_string(f, name);

	/* find material */
	for (i = 0; i < lwo->material_cnt; i++) {
		if (strcmp(lwo->materials[i].name, name) == 0) {
			material = &lwo->materials[i];
			break;
		}
	}
	g_return_if_fail(material != NULL);

	/* read values */
	while (nbytes > 0) {
		gint id = read_long(f);
		gint len = read_short(f);
		nbytes -= 6 + len + (len % 2);

		switch (id) {
		case ID_COLR:
			material->r = read_char(f) / 255.0;
			material->g = read_char(f) / 255.0;
			material->b = read_char(f) / 255.0;
			read_char(f);	/* dummy */
			break;

		default:
			fseek(f, len + (len % 2), SEEK_CUR);
		}
	}
}


static void
read_pols(FILE *f, int nbytes, lwObject *lwo)
{
	int guess_cnt = lwo->face_cnt;
	size_t n;

	while (nbytes > 0) {
		lwFace *face;
		int i;

		/* allocate more memory for polygons if necessary */
		if (guess_cnt <= lwo->face_cnt) {
			guess_cnt += guess_cnt + 4;
			lwo->faces = xrealloc(lwo->faces, sizeof(lwFace) * guess_cnt);
		}
		face = lwo->faces + lwo->face_cnt++;

		/* number of points in this face */
		face->index_cnt = read_short(f);
		nbytes -= 2;

		/* allocate space for points */
		n = sizeof(int) * face->index_cnt;
		face->indices = xmalloc(n);

		/* read points in */
		for (i = 0; i < face->index_cnt; i++) {
			face->indices[i] = read_short(f);
			nbytes -= 2;
		}

		/* read surface material */
		face->mat_id = read_short(f);
		nbytes -= 2;

		/* skip over detail  polygons */
		if (face->mat_id < 0) {
			int det_cnt;
			face->mat_id = - face->mat_id;
			det_cnt = read_short(f);
			nbytes -= 2;
			while (det_cnt-- > 0) {
				int cnt = read_short(f);
				fseek(f, cnt * 2 + 2, SEEK_CUR);
				nbytes -= cnt * 2 + 2;
			}
		}
		face->mat_id -= 1;
	}
	/* readjust to true size */
	lwo->faces = xrealloc(lwo->faces, sizeof(lwFace) * lwo->face_cnt);
}



static void
read_pnts(FILE *f, gint nbytes, lwObject *lwo)
{
	int i;
	lwo->vertex_cnt = nbytes / 12;
	lwo->vertices = xmalloc( lwo->vertex_cnt * sizeof(lwPoint) );
	for (i = 0; i < lwo->vertex_cnt; i++) {
		lwo->vertices[i].x = read_float(f);
		lwo->vertices[i].y = read_float(f);
		lwo->vertices[i].z = read_float(f);
	}
}






gint
lw_is_lwobject(const char *lw_file)
{
	FILE *f = fopen(lw_file, "rb");
	if (f) {
		gint32 form = read_long(f);
		gint32 nlen = read_long(f);
		gint32 lwob = read_long(f);
		fclose(f);
		if ((form == ID_FORM) && (nlen != 0) && (lwob == ID_LWOB))
			return TRUE;
	}
	return FALSE;
}


lwObject *
lw_object_read(const char *lw_file)
{
	FILE *f = NULL;
	lwObject *lw_object = NULL;
	size_t n;

	gint32 form_bytes = 0;
	gint32 read_bytes = 0;

	/* open file */
	f = fopen(lw_file, "rb");
	if (f == NULL) {
		/* g_warning("can't open file %s", lw_file); */
		return NULL;
	}

	/* check for headers */
	if (read_long(f) != ID_FORM) {
		/* g_warning("file %s is not an IFF file", lw_file); */
		fclose(f);
		return NULL;
	}
	form_bytes = read_long(f);
	read_bytes += 4;

	if (read_long(f) != ID_LWOB) {
		/* g_warning("file %s is not a LWOB file", lw_file); */
		fclose(f);
		return NULL;
	}

	/* create new lwObject */
	n = sizeof(lwObject);
	lw_object = xmalloc(n);
	memset(lw_object, 0, n);

	/* read chunks */
	while (read_bytes < form_bytes) {
		gint32 id = read_long(f);
		gint32 nbytes = read_long(f);
		read_bytes += 8 + nbytes + (nbytes % 2);

		switch (id) {
		case ID_PNTS:
			read_pnts(f, nbytes, lw_object);
			break;

		case ID_POLS:
			read_pols(f, nbytes, lw_object);
			break;

		case ID_SRFS:
			read_srfs(f, nbytes, lw_object);
			break;

		case ID_SURF:
			read_surf(f, nbytes, lw_object);
			break;

		default:
			fseek(f, nbytes + (nbytes % 2), SEEK_CUR);
		}
	}

	fclose(f);
	return lw_object;
}







void
lw_object_free(lwObject *lw_object)
{
	g_return_if_fail(lw_object != NULL);

	if (lw_object->faces != NULL) {
		int i;
		for (i = 0; i < lw_object->face_cnt; i++)
			xfree(lw_object->faces[i].indices);
		xfree(lw_object->faces);
	}
	xfree(lw_object->materials);
	xfree(lw_object->vertices);
	xfree(lw_object);
}




#if 0 /* don't need this next bit */

#define PX(i) (lw_object->vertices[face->indices[i]].x)
#define PY(i) (lw_object->vertices[face->indices[i]].y)
#define PZ(i) (lw_object->vertices[face->indices[i]].z)
void
lw_object_show(const lwObject *lw_object)
{
	int i, j;
	int prev_index_cnt = -1;
	int prev_material = -1;
	float prev_nx = 0;
	float prev_ny = 0;
	float prev_nz = 0;

	g_return_if_fail(lw_object != NULL);

	for (i = 0; i < lw_object->face_cnt; i++) {
		float ax, ay, az, bx, by, bz, nx, ny, nz, r;
		const lwFace *face = lw_object->faces + i;

		/* ignore faces with less than 3 points */
		if (face->index_cnt < 3)
			continue;

		/* calculate normal */
		ax = PX(1) - PX(0);
		ay = PY(1) - PY(0);
		az = PZ(1) - PZ(0);

		bx = PX(face->index_cnt - 1) - PX(0);
		by = PY(face->index_cnt - 1) - PY(0);
		bz = PZ(face->index_cnt - 1) - PZ(0);

		nx = ay * bz - az * by;
		ny = az * bx - ax * bz;
		nz = ax * by - ay * bx;

		r = sqrt(nx * nx + ny * ny + nz * nz);
		if (r < 0.000001)	/* avoid division by zero */
			continue;
		nx /= r;
		ny /= r;
		nz /= r;

		/* glBegin/glEnd */
		if (prev_index_cnt != face->index_cnt || prev_index_cnt > 4) {
			if (prev_index_cnt > 0)
				glEnd();
			prev_index_cnt = face->index_cnt;
			switch (face->index_cnt) {
			case 3:
				glBegin(GL_TRIANGLES);
				break;

			case 4:
				glBegin(GL_QUADS);
				break;

			default:
				glBegin(GL_POLYGON);
			}
		}

		/* update material if necessary */
		if (prev_material != face->mat_id) {
			prev_material = face->mat_id;
			glColor3f(lw_object->materials[face->mat_id].r,
			          lw_object->materials[face->mat_id].g,
			          lw_object->materials[face->mat_id].b);
		}

		/* update normal if necessary */
		if (nx != prev_nx || ny != prev_ny || nz != prev_nz) {
			prev_nx = nx;
			prev_ny = ny;
			prev_nz = nz;
			glNormal3f(nx, ny, nz);
		}

		/* draw polygon/triangle/quad */
		for (j = 0; j < face->index_cnt; j++)
			glVertex3f(PX(j), PY(j), PZ(j));

	}

	/* if glBegin was called call glEnd */
	if (prev_index_cnt > 0)
		glEnd();
}


float
lw_object_radius(const lwObject *lwo)
{
	int i;
	double max_radius = 0.0;

	g_return_val_if_fail(lwo != NULL, 0.0);

	for (i = 0; i < lwo->vertex_cnt; i++) {
		point *v = &lwo->vertices[i];
		double r = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
		if (r > max_radius)
			max_radius = r;
	}
	return sqrt(max_radius);
}

void
lw_object_scale(lwObject *lwo, float scale)
{
	int i;

	g_return_if_fail(lwo != NULL);

	for (i = 0; i < lwo->vertex_cnt; i++) {
		lwo->vertices[i].x *= scale;
		lwo->vertices[i].y *= scale;
		lwo->vertices[i].z *= scale;
	}
}

#endif /* 0 */

/* end readlwo.c */
