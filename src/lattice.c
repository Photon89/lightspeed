/* lattice.c */

/* Lattice generator */

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


/* Forward declarations */
static void add_ball( ogl_object *parent_obj, float x0, float y0, float z0, int smoothness );
static void add_stick( ogl_object *parent_obj, float arg1, float arg2, float arg3, float arg4, int num_segs, int smoothness, int alignment );
static int add_point( ogl_object *obj, float x, float y, float z, float normal_x, float normal_y, float normal_z );
static void add_index( ogl_object *obj, int index );


/* Creates a 3D lattice of specified size, with sticks properly segmented
 * in the y and z directions for our purposes (i.e. these will be bent) */
void
make_lattice( int size_x, int size_y, int size_z, int smoothness )
{
	ogl_object *lattice_balls;
	ogl_object *lattice_sticks;
	ogl_object dummy;
	float xc, yc, zc;
	float x, y, z;
	float sign = 1.0;
	int nodes_x, nodes_y, nodes_z;
	int num_balls, num_sticks;
	int segs_x, segs_y, segs_z;
	int num_vertices, num_indices;
	int i, j, k;
	int i_inc, j_inc, k_inc;

#ifdef DEBUG
	printf( "Building %dx%dx%d/%d lattice...", size_x, size_y, size_z, smoothness );
	fflush( stdout );
#endif

	nodes_x = size_x + 1;
	nodes_y = size_y + 1;
	nodes_z = size_z + 1;

	/* Half the size of each lattice dimension
	 * (disregarding radii of sticks/balls) */
	xc = LATTICE_UNIT_SIZE * (float)size_x / 2;
	yc = LATTICE_UNIT_SIZE * (float)size_y / 2;
	zc = LATTICE_UNIT_SIZE * (float)size_z / 2;

	/* NODE BALLS */

	/* Determine how many vertices/indices the balls will need */
	dummy.type = -1;
	dummy.num_vertices = 0;
	dummy.num_indices = 0;
	add_ball( &dummy, 0, 0, 0, smoothness );
	num_balls = nodes_x * nodes_y * nodes_z;
	num_vertices = num_balls * dummy.num_vertices;
	num_indices = num_balls * dummy.num_indices;

	lattice_balls = alloc_ogl_object( num_vertices, num_indices );
	lattice_balls->type = GL_QUAD_STRIP;
	lattice_balls->color0.r = BALL_R;
	lattice_balls->color0.g = BALL_G;
	lattice_balls->color0.b = BALL_B;
	lattice_balls->num_vertices = 0;
	lattice_balls->num_indices = 0;

	/* Make the balls */

	/* j and k are in "ping-pong" loops */
	j = 0;
	k = 0;
	j_inc = 1;
	k_inc = 1;
	for (i = 0; i < nodes_x; i++) {
		for (; (j < nodes_y) && (j >= 0); j += j_inc) {
			for (; (k < nodes_z) && (k >= 0); k += k_inc) {
				x = LATTICE_UNIT_SIZE * (float)i - xc;
				y = LATTICE_UNIT_SIZE * (float)j - yc;
				z = LATTICE_UNIT_SIZE * (float)k - zc;
				add_ball( lattice_balls, x, y, z, smoothness );
			}
			k -= k_inc;
			k_inc = - k_inc;
		}
		j -= j_inc;
		j_inc = - j_inc;
	}

#ifdef DEBUG
	if (lattice_balls->num_vertices != num_vertices)
		printf( "ERROR: Lattice balls vertices: %d expected, %d actual\n",
		        num_vertices, lattice_balls->num_vertices );
	if (lattice_balls->num_indices != num_indices)
		printf( "ERROR: Lattice balls indices: %d expected, %d actual\n",
		        num_indices, lattice_balls->num_indices );
	fflush( stdout );
#endif

	/* INTERCONNECTING STICKS */

	/* Number of segments along segmented y- & z-axis-aligned sticks */
	segs_x = size_x * 2;
	segs_y = size_y * smoothness;
	segs_z = size_z * smoothness;

	/* Determine how many vertices/indices the sticks will need */
	num_vertices = 0;
	num_indices = 0;
	dummy.type = -1;
	/* x-aligned sticks */
	dummy.num_vertices = 0;
	dummy.num_indices = 0;
	add_stick( &dummy, 0, 0, 0, 0, segs_x, smoothness, X_ALIGN );
	num_sticks = nodes_y * nodes_z;
	num_vertices += num_sticks * dummy.num_vertices;
	num_indices += num_sticks * dummy.num_indices;
	/* y-aligned sticks */
	dummy.num_vertices = 0;
	dummy.num_indices = 0;
	add_stick( &dummy, 0, 0, 0, 0, segs_y, smoothness, Y_ALIGN );
	num_sticks = nodes_z * nodes_x;
	num_vertices += num_sticks * dummy.num_vertices;
	num_indices += num_sticks * dummy.num_indices;
	/* z-aligned sticks */
	dummy.num_vertices = 0;
	dummy.num_indices = 0;
	add_stick( &dummy, 0, 0, 0, 0, segs_z, smoothness, Z_ALIGN );
	num_sticks = nodes_x * nodes_y;
	num_vertices += num_sticks * dummy.num_vertices;
	num_indices += num_sticks * dummy.num_indices;

	lattice_sticks = alloc_ogl_object( num_vertices, num_indices );
	lattice_sticks->type = GL_QUAD_STRIP;
	lattice_sticks->color0.r = STICK_R;
	lattice_sticks->color0.g = STICK_G;
	lattice_sticks->color0.b = STICK_B;
	lattice_sticks->num_vertices = 0;
	lattice_sticks->num_indices = 0;

	/* Make the sticks */

	/* Make the x-aligned sticks */
	k = 0;
	k_inc = 1;
	for (j = 0; j < nodes_y; j++) {
		y = LATTICE_UNIT_SIZE * (float)j - yc;
		for (; (k < nodes_z) && (k >= 0); k += k_inc) {
			z = LATTICE_UNIT_SIZE * (float)k - zc;
			add_stick( lattice_sticks, - sign * xc, sign * xc, y, z,
			           segs_x, smoothness, X_ALIGN );
			sign = - sign;
		}
		k -= k_inc;
		k_inc = - k_inc;
	}

	/* Make the y-aligned sticks */
	if (sign < 0) {
		i = 0;
		i_inc = 1;
	}
	else {
		i = nodes_x - 1;
		i_inc = -1;
	}
	sign = -1.0; /* as j_inc == 1 */
	for (; (k < nodes_z) && (k >= 0); k += k_inc) {
		z = LATTICE_UNIT_SIZE * (float)k - zc;
		for (; (i < nodes_x) && (i >= 0); i += i_inc) {
			x = LATTICE_UNIT_SIZE * (float)i - xc;
			add_stick( lattice_sticks, x, - sign * yc, sign * yc, z,
			           segs_y, smoothness, Y_ALIGN );
			sign = - sign;
		}
		i -= i_inc;
		i_inc = - i_inc;
	}

	/* Make the z-aligned sticks */
	if (sign > 0) {
		j = 0;
		j_inc = 1;
	}
	else {
		j = nodes_y - 1;
		j_inc = -1;
	}
	sign = (float)k_inc;
	for (; (i < nodes_x) && (i >= 0); i += i_inc) {
		x = LATTICE_UNIT_SIZE * (float)i - xc;
		for (; (j < nodes_y) && (j >= 0); j += j_inc) {
			y = LATTICE_UNIT_SIZE * (float)j - yc;
			add_stick( lattice_sticks, x, y, - sign * zc, sign * zc,
			           segs_z, smoothness, Z_ALIGN );
			sign = - sign;
		}
		j -= j_inc;
		j_inc = - j_inc;
	}

#ifdef DEBUG
	if (lattice_sticks->num_vertices != num_vertices)
		printf( "ERROR: Lattice sticks vertices: %d expected, %d actual\n",
		        num_vertices, lattice_sticks->num_vertices );
	if (lattice_sticks->num_indices != num_indices)
		printf( "ERROR: Lattice sticks indices: %d expected, %d actual\n",
		        num_indices, lattice_sticks->num_indices );
	fflush( stdout );
#endif

	vehicle_objs = xmalloc( 2 * sizeof(ogl_object *) );
	vehicle_objs[0] = lattice_balls;
	vehicle_objs[1] = lattice_sticks;
	num_vehicle_objs = 2;

	/* Define dimensional extents */
	xc += BALL_RADIUS;
	yc += BALL_RADIUS;
	zc += BALL_RADIUS;
	vehicle_extents.xmin = - xc;
	vehicle_extents.xmax = xc;
	vehicle_extents.ymin = - yc;
	vehicle_extents.ymax = yc;
	vehicle_extents.zmin = - zc;
	vehicle_extents.zmax = zc;
	vehicle_extents.avg = 2 * (xc + yc + zc) / 3;

#ifdef DEBUG
	printf( "done (%db+%ds vertices, %db+%ds indices).\n",
	        lattice_balls->num_vertices, lattice_sticks->num_vertices,
	        lattice_balls->num_indices, lattice_sticks->num_indices );
	fflush( stdout );
#endif
}


static void
add_ball( ogl_object *parent_obj, float x0, float y0, float z0, int smoothness )
{
	float theta;
	float phi;
	float x, y, z;
	int p, t;
	int range_t, range_p;
	int center_v, pole_v, begin_v = 0;
	int obj_v;

	range_t = smoothness;
	range_p = 2 * smoothness;

	/* Center vertex */
	center_v = add_point( parent_obj, x0, y0, z0, 0.0, 0.0, -1.0 );
	add_index( parent_obj, center_v );
	add_index( parent_obj, center_v );

	/* Front of ball (North Pole to Arctic Circle) */

	/* Pole vertex */
	pole_v = add_point( parent_obj, x0 + BALL_RADIUS, y0, z0, 1.0, 0.0, 0.0 );

	theta = RAD(90.0 - 180.0 / (float)range_t);
	x = BALL_RADIUS * sin( theta );
	for (p = 0; p < range_p; p++) {
		phi = RAD(360.0 * (float)p / (float)range_p);
		y = BALL_RADIUS * cos( theta ) * cos( phi );
		z = BALL_RADIUS * cos( theta ) * sin( phi );
		obj_v = add_point( parent_obj, x0 + x, y0 + y, z0 + z, x, y, z );
		add_index( parent_obj, pole_v );
		add_index( parent_obj, obj_v );
		if (p == 0)
			begin_v = obj_v;
	}

	/* Complete the circle */
	add_index( parent_obj, pole_v );
	add_index( parent_obj, begin_v );

	/* Midsection of ball (Arctic Circle to Antarctic Circle) */

	for (t = 2; t <  range_t; t++) {
		theta = RAD(90.0 - 180.0 * (float)t / (float)range_t);
		x = BALL_RADIUS * sin( theta );
		for (p = 0; p < range_p; p++) {
			phi = RAD(360.0 * (float)p / (float)range_p);
			y = BALL_RADIUS * cos( theta ) * cos( phi );
			z = BALL_RADIUS * cos( theta ) * sin( phi );
			obj_v = add_point( parent_obj, x0 + x, y0 + y, z0 + z, x, y, z );
			add_index( parent_obj, obj_v - range_p );
			add_index( parent_obj, obj_v );
			if (p == 0)
				begin_v = obj_v;
		}

		/* Complete the circle */
		add_index( parent_obj, begin_v - range_p );
		add_index( parent_obj, begin_v );
	}

	/* Back of ball (Antarctic Circle to South Pole) */

	/* Pole vertex */
	pole_v = add_point( parent_obj, x0 - BALL_RADIUS, y0, z0, -1.0, 0.0, 0.0 );

	theta = RAD(180.0 / (float)range_t - 90.0);
	x = BALL_RADIUS * sin( theta );
	for (p = 0; p < range_p; p++) {
		phi = RAD(360.0 * (float)p / (float)range_p);
		y = BALL_RADIUS * cos( theta ) * cos( phi );
		z = BALL_RADIUS * cos( theta ) * sin( phi );
		obj_v = add_point( parent_obj, x0 + x, y0 + y, z0 + z, x, y, z );
		add_index( parent_obj, obj_v );
		add_index( parent_obj, pole_v );
		if (p == 0)
			begin_v = obj_v;
	}

	/* Once more, complete the circle */
	add_index( parent_obj, begin_v );
	add_index( parent_obj, pole_v );

	/* and tie quad strip back to center */
	add_index( parent_obj, center_v );
	add_index( parent_obj, center_v );
}


static void
add_stick( ogl_object *parent_obj, float arg1, float arg2, float arg3, float arg4,
           int num_segs, int smoothness, int alignment )
{
	float phi;
	float a0, a1, b0, c0;
	float a, b, c;
	float x0, y0, z0;
	float xn, yn, zn;
	float x, y, z;
	int s, p;
	int range_p;
	int reverse = FALSE;
	int end_v, begin_v = 0;
	int obj_v;

	/* Interpret args differently depending on alignment */
	switch (alignment) {
	case X_ALIGN:
		a0 = arg1;
		a1 = arg2;
		b0 = arg3;
		c0 = arg4;
		break;

	case Y_ALIGN:
		b0 = arg1;
		a0 = - arg2;
		a1 = - arg3;
		c0 = arg4;
		break;

	case Z_ALIGN:
		c0 = arg1;
		b0 = arg2;
		a0 = - arg3;
		a1 = - arg4;
		break;

	default:
#ifdef DEBUG
		crash( "add_stick( ): invalid alignment" );
#endif
		return;
	}

	range_p = 2 * smoothness;
	if (a0 > a1)
		reverse = TRUE; /* to keep the normals right */

	/* Endcap vertex (tie point) */
	rotate_xyz( alignment, &x0, &y0, &z0, a0, b0, c0 );
	end_v = add_point( parent_obj, x0, y0, z0, 0.0, 0.0, -1.0 );
	add_index( parent_obj, end_v );
	add_index( parent_obj, end_v );

	for (s = 0; s <= num_segs; s++) {
		a = (a1 - a0) * (float)s / (float)num_segs;
		for (p = 0; p < range_p; p++) {
			phi = RAD(360.0 * (float)p / (float)range_p);
			b = STICK_RADIUS * cos( phi );
			c = STICK_RADIUS * sin( phi );
			rotate_xyz( alignment, &x, &y, &z, a, b, c );
			rotate_xyz( alignment, &xn, &yn, &zn, 0, b, c );
			obj_v = add_point( parent_obj, x0 + x, y0 + y, z0 + z, xn, yn, zn );
			if (s > 0) {
				if (reverse) {
					add_index( parent_obj, obj_v - range_p );
					add_index( parent_obj, obj_v );
				}
				else {
					add_index( parent_obj, obj_v );
					add_index( parent_obj, obj_v - range_p );
				}
			}
			if (p == 0)
				begin_v = obj_v;
		}
		if (s == 0)
			continue;

		/* Complete the circle */
		if (reverse) {
			add_index( parent_obj, begin_v - range_p );
			add_index( parent_obj, begin_v );
		}
		else {
			add_index( parent_obj, begin_v );
			add_index( parent_obj, begin_v - range_p );
		}
	}

	/* The other endcap vertex (tie point) */
	rotate_xyz( alignment, &x, &y0, &z0, a1, b0, c0 );
	end_v = add_point( parent_obj, x, y0, z0, 0.0, 0.0, -1.0 );
	add_index( parent_obj, end_v );
	add_index( parent_obj, end_v );
}


static int
add_point( ogl_object *obj, float x, float y, float z, float norm_x, float norm_y, float norm_z )
{
	float d;
	int v;

	v = obj->num_vertices;

	if (obj->type != -1) {
		/* Set vertex location */
		obj->vertices0[v].x = x;
		obj->vertices0[v].y = y;
		obj->vertices0[v].z = z;

		/* Normalize and set vertex normal */
		d = sqrt( SQR(norm_x) + SQR(norm_y) + SQR(norm_z) );
		if (d < 1E-6)
			d = 1.0;
		obj->normals0[v].x = norm_x / d;
		obj->normals0[v].y = norm_y / d;
		obj->normals0[v].z = norm_z / d;
	}

	++obj->num_vertices;

	return v;
}


static void
add_index( ogl_object *obj, int index )
{
	int i;

	i = obj->num_indices++;
	if (obj->type != -1)
		obj->indices[i] = index;
}


#ifdef WITH_SRS_EXPORTER
void
write_srs_lattice( FILE *srs )
{
	point p1, p2;
	point p1_srs, p2_srs;
	rgb_color color;
	float xc,yc,zc;
	float x0,y0,z0;
	int i,j,k;

	/* Centering translation */
	xc = (float)lattice_size_x * LATTICE_UNIT_SIZE / 2.0;
	yc = (float)lattice_size_y * LATTICE_UNIT_SIZE / 2.0;
	zc = (float)lattice_size_z * LATTICE_UNIT_SIZE / 2.0;

	i = lattice_size_x;
	j = lattice_size_y;
	k = lattice_size_z;
	fprintf( srs, "\t// **** Begin %dx%dx%d lattice definition ****\n", i, j, k );
	fprintf( srs, "\tunion { // ** Balls **\n" );

	/* Make the balls */
	for (i = 0; i <= lattice_size_x; i++) {
		x0 = (float)i * LATTICE_UNIT_SIZE - xc;
		p1.x = x0;
		for (j = 0; j <= lattice_size_y; j++) {
			y0 = (float)j * LATTICE_UNIT_SIZE - yc;
			p1.y = y0;
			for (k = 0; k <= lattice_size_z; k++) {
				z0 = (float)k * LATTICE_UNIT_SIZE - zc;
				p1.z = z0;
				convert_to_srs_cs( &p1_srs, &p1 );
				write_srs_sphere( srs, &p1_srs, BALL_RADIUS );
			}
		}
	}

	/* Ball material */
	color.r = BALL_R;
	color.g = BALL_G;
	color.b = BALL_B;
	write_srs_pigment( srs, &color );

	fprintf( srs, "\t}\n" );
	fprintf( srs, "\tunion { // ** Sticks **\n" );

	/* Make x-sticks */
	for (j = 0; j <= lattice_size_y; j++) {
		y0 = (float)j * LATTICE_UNIT_SIZE - yc;
		p1.y = y0;
		p2.y = y0;
		for (k = 0; k <= lattice_size_z; k++) {
			z0 = (float)k * LATTICE_UNIT_SIZE - zc;
			p1.x = xc;
			p1.z = z0;
			convert_to_srs_cs( &p1_srs, &p1 );
			p2.x = - xc;
			p2.z = z0;
			convert_to_srs_cs( &p2_srs, &p2 );
			write_srs_cylinder( srs, &p1_srs, &p2_srs, STICK_RADIUS );
		}
	}

	/* Make y-sticks */
	for (i = 0; i <= lattice_size_x; i++) {
		x0 = (float)i * LATTICE_UNIT_SIZE - xc;
		p1.x = x0;
		p2.x = x0;
		for (k = 0; k <= lattice_size_z; k++) {
			z0 = (float)k * LATTICE_UNIT_SIZE - zc;
			p1.y = yc;
			p1.z = z0;
			convert_to_srs_cs( &p1_srs, &p1 );
			p2.y = - yc;
			p2.z = z0;
			convert_to_srs_cs( &p2_srs, &p2 );
			write_srs_cylinder( srs, &p1_srs, &p2_srs, STICK_RADIUS );
		}
	}

	/* Make z-sticks */
	for (i = 0; i <= lattice_size_x; i++) {
		x0 = (float)i * LATTICE_UNIT_SIZE - xc;
		p1.x = x0;
		p2.x = x0;
		for (j = 0; j <= lattice_size_y; j++) {
			y0 = (float)j * LATTICE_UNIT_SIZE - yc;
			p1.y = y0;
			p1.z = zc;
			convert_to_srs_cs( &p1_srs, &p1 );
			p2.y = y0;
			p2.z = - zc;
			convert_to_srs_cs( &p2_srs, &p2 );
			write_srs_cylinder( srs, &p1_srs, &p2_srs, STICK_RADIUS );
		}
	}

	/* Stick material */
	color.r = STICK_R;
	color.g = STICK_G;
	color.b = STICK_B;
	write_srs_pigment( srs, &color );

	fprintf( srs, "\t}\n" );
	fprintf( srs, "\t// **** End lattice definition ****\n" );
}
#endif /* WITH_SRS_EXPORTER */

/* end lattice.c */
