/* warp.c */

/* Relativistic distortion engine */

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
static void doppler_shift( rgb_color *color, float freq_ratio );
static void doppler_shift_ref( rgb_color *color, float freq_ratio );


/* This is where it all happens! */
int
warp( int message, void *data )
{
	static int do_lorentz_contraction = TRUE;
	static int do_optical_deformation = TRUE;
	static int do_headlight_effect = TRUE;
	static int do_doppler_shift = TRUE;
	static float percent_contraction = 1.0;
	static float percent_deformation = 1.0;
	static float percent_dopplershift = 1.0;
	static float percent_headlight = 1.0;
#if USE_LOOKUP_TABLES
	static float sqrt01_lut[LUT_RES + 1];
	static float normal_interp_lut1[LUT_RES + 1];
	static float normal_interp_lut2[LUT_RES + 1];
#endif
	ogl_object *obj;
	ogl_point *pnt;
	point *cam_pos;
	point vertex;
	point normal;
	rgb_color color;
	rgb_color in_ray;
	double LC_gamma;
	double OD_v, OD_v2, OD_v2_min_C2;
	double DS_v_over_C, DS_gamma;
	double HE_v_over_C, HE_gamma;
	double d_vertex_x;
	double dx,dy,dz;
	double dyz2, dist2;
	double t;
	float cos_alpha_n, cos_alpha_c;
	float freq_ratio;
	float inten_ratio;
	float intensity;
	float len2, len;
	float k;
	int message2 = 0;
	int o, vn, i;

	if ((message != WARP_DISTORT) && (message != INITIALIZE))
		message2 = *((int *)data); /* several methods use this value */

	switch (message) {
	case WARP_DISTORT:
		cam_pos = (point *)data;
		break;

	case WARP_LORENTZ_CONTRACTION:
		do_lorentz_contraction = message2;
		if (do_lorentz_contraction)
			transition( &percent_contraction, FALSE, TRANS_SIGMOID, 4.0, 1.0, -1 );
		else
			transition( &percent_contraction, FALSE, TRANS_SIGMOID, 4.0, 0.0, -1 );
		return 0;

	case WARP_OPTICAL_DEFORMATION:
		do_optical_deformation = message2;
		if (do_optical_deformation)
			transition( &percent_deformation, FALSE, TRANS_SIGMOID, 4.0, 1.0, -1 );
		else
			transition( &percent_deformation, FALSE, TRANS_SIGMOID, 4.0, 0.0, -1 );
		return 0;

	case WARP_HEADLIGHT_EFFECT:
		do_headlight_effect = message2;
		if (do_headlight_effect)
			transition( &percent_headlight, FALSE, TRANS_SIGMOID, 4.0, 1.0, -1 );
		else
			transition( &percent_headlight, FALSE, TRANS_SIGMOID, 4.0, 0.0, -1 );
		return 0;

	case WARP_DOPPLER_SHIFT:
		do_doppler_shift = message2;
		if (do_doppler_shift)
			transition( &percent_dopplershift, FALSE, TRANS_SIGMOID, 4.0, 1.0, -1 );
		else
			transition( &percent_dopplershift, FALSE, TRANS_SIGMOID, 4.0, 0.0, -1 );
		return 0;

	case QUERY:
		switch (message2) {
		case WARP_LORENTZ_CONTRACTION:
			return do_lorentz_contraction;

		case WARP_OPTICAL_DEFORMATION:
			return do_optical_deformation;

		case WARP_HEADLIGHT_EFFECT:
			return do_headlight_effect;

		case WARP_DOPPLER_SHIFT:
			return do_doppler_shift;
		}
		return 0;

	case INITIALIZE:
#if USE_LOOKUP_TABLES
		/* Initialize look-up tables */
		for (i = 0; i <= LUT_RES; i++) {
			/* Normal interpolation factor tables
			 * Table 1 handles [0, 1] input range */
			normal_interp_lut1[i] = DEG(atan( (double)i / LUT_RES )) / 90.0;
			/* Table 2 handles (1, inf.) (via reciprocal) */
			if (i > 0)
				normal_interp_lut2[i] = DEG(atan( LUT_RES / (double)i )) / 90.0;
			/* Square root table for [0, 1] range */
			sqrt01_lut[i] = sqrt( (double)i / LUT_RES );
		}
		normal_interp_lut2[0] = 1.0;
#endif /* USE_LOOKUP_TABLES */
		return 0;

	default:
#ifdef DEBUG
		crash( "warp( ): invalid message" );
#endif
		return 0;
	}

	/* Variables for Lorentz contraction */
	LC_gamma = lorentz_factor( velocity * percent_contraction );

	/* Variables for optical deformation */
	OD_v = MAX(1.0, velocity * percent_deformation);
	OD_v2 = SQR(OD_v);
	OD_v2_min_C2 = OD_v2 - C2;

	/* Simulation time (and x-location) depend on effective velocity
	 * used for optical deformation */
	warp_time( NIL, NIL, OD_v, WARP_UPDATE_TIME_T );
	vehicle_real_x = OD_v * cur_time_t;

	/* Variables for Doppler shift */
	DS_v_over_C = velocity * percent_dopplershift / C;
	DS_gamma = lorentz_factor( velocity * percent_dopplershift );

	/* Variables for headlight effect */
	HE_v_over_C = velocity * percent_headlight / C;
	HE_gamma = lorentz_factor( velocity * percent_headlight );

	for (o = 0; o < num_vehicle_objs; o++) {
		obj = vehicle_objs[o];
		/* "vn" is the counter instead of "v", to avoid
		 * possible confusion with velocity variables */
		for (vn = 0; vn < obj->num_vertices; vn++) {
			/* Load vertex location */
			vertex.x = obj->vertices0[vn].x;
			vertex.y = obj->vertices0[vn].y;
			vertex.z = obj->vertices0[vn].z;
			/* Load vertex normal direction */
			normal.x = obj->normals0[vn].x;
			normal.y = obj->normals0[vn].y;
			normal.z = obj->normals0[vn].z;
			/* (Re)load base RGB color */
			color.r = obj->color0.r;
			color.g = obj->color0.g;
			color.b = obj->color0.b;

			/**** RELATIVISTIC GEOMETRY TRANSFORMS ****/

			/* Do x-coordinate work in double precision */
			d_vertex_x = vertex.x;

			/** Lorentz contraction **/
			d_vertex_x /= LC_gamma;

			/* Adjust normal accordingly */
			dx = normal.x;
			dy = normal.y / LC_gamma;
			dz = normal.z / LC_gamma;

			/* Renormalize the normal */
			len2 = SQR(dx) + SQR(dy) + SQR(dz);
#if USE_LOOKUP_TABLES
#ifdef DEBUG
			if (len2 > 1.001) {
				printf( "ERROR: warp( ): normal length > 1.0 !!!\n" );
				fflush( stdout );
				len2 = 1.0;
			}
#endif /* DEBUG */
			len = sqrt01_lut[(int)(len2 * LUT_RES)];
#else
			len = sqrt( len2 );
#endif /* not USE_LOOKUP_TABLES */
			if (len < 1E-6)
				len = 1.0;
			normal.x = dx / len;
			normal.y = dy / len;
			normal.z = dz / len;

			/* Move object to its "real" x-position */
			d_vertex_x += vehicle_real_x;

			/** Optical deformation **/

			/* Obtain xyz deltas (camera to vertex) */
			dx = d_vertex_x - cam_pos->x;
			dy = vertex.y - cam_pos->y;
			dz = vertex.z - cam_pos->z;

			/* square, add */
			dyz2 = SQR(dy) + SQR(dz); /* lump y & z together */
			dist2 = SQR(dx) + dyz2;

			/* Calculate t and adjust vertex accordingly */
			t = (dx*OD_v - sqrt( C2*dist2 - dyz2*OD_v2 )) / OD_v2_min_C2;
			d_vertex_x -= OD_v * t;

			/* done with double-precision math */
			vertex.x = d_vertex_x;

			/* Note: dx and dist2 do NOT get updated!
			 * alpha_c below is calculated w.r.t. the
			 * vertex's "actual" position */

			/* Need two angles for the color transforms:
			 * alpha_n = angle between direction of
			 * travel and vertex normal;
			 * alpha_c = angle between direction of
			 * travel and vertex-to-camera vector */
			cos_alpha_n = normal.x;
			if (dist2 > 1E-6)
				cos_alpha_c = - dx / sqrt( dist2 );
			else
				cos_alpha_c = 0.0;

			/**** RELATIVISTIC COLOR/INTENSITY TRANSFORMS ****/

			/* Incoming light ray */
			/* in_ray.r = 1.0; */
			/* in_ray.g = 1.0; */
			/* in_ray.b = 1.0; */

			/** Doppler frequency shift (incoming light) **/
			k = 1.0 + (HE_v_over_C * cos_alpha_n);
			inten_ratio = SQR(k) * HE_gamma;
			/* in_ray.r *= inten_ratio; */
			/* in_ray.g *= inten_ratio; */
			/* in_ray.b *= inten_ratio; */
			in_ray.r = inten_ratio;
			in_ray.g = inten_ratio;
			in_ray.b = inten_ratio;

			/** Headlight effect (incoming light) **/
			freq_ratio = (1.0 + (DS_v_over_C * cos_alpha_n)) * DS_gamma;
			doppler_shift( &in_ray, freq_ratio );

			/* Illuminative color interaction */
			color.r *= SQR(in_ray.r);
			color.g *= SQR(in_ray.g);
			color.b *= SQR(in_ray.b);

			/** Doppler frequency shift (outgoing light) **/
			freq_ratio = (1.0 + (DS_v_over_C * cos_alpha_c)) * DS_gamma;
			doppler_shift_ref( &color, freq_ratio );

			/** Headlight effect (outgoing light) **/
			k = 1.0 + (HE_v_over_C * cos_alpha_c);
			inten_ratio = SQR(k) * HE_gamma;
			color.r *= inten_ratio;
			color.g *= inten_ratio;
			color.b *= inten_ratio;

			/* If intensity exceeds I(1,1,1),
			 * rotate normal toward camera */
			intensity = (color.r * RED_STRENGTH) + (color.g * GREEN_STRENGTH) + (color.b * BLUE_STRENGTH);
			if (intensity > 1.0) {
				/* Normal interpolation factor, range [0, 1)
				 * 0 == unchanged, 1 == pointing toward camera */
#if USE_LOOKUP_TABLES
				if (intensity <= 2.0) {
					i = (int)((intensity - 1.0) * LUT_RES);
					k = normal_interp_lut1[i];
				}
				else {
					i = (int)(LUT_RES / (intensity - 1.0));
					k = normal_interp_lut2[i];
				}
#else
				k = DEG(atan( intensity - 1.0 )) / 90.0;
#endif /* not USE_LOOKUP_TABLES */
				/* Interpolate between normal vector and
				 * vertex-to-camera vector */
				normal.x -= k * (dx + normal.x);
				normal.y -= k * (dy + normal.y);
				normal.z -= k * (dz + normal.z);
				/* Renormalize */
				len = sqrt( SQR(normal.x) + SQR(normal.y) + SQR(normal.z) );
				if (len < 1E-6)
					len = 1.0;
				normal.x /= len;
				normal.y /= len;
				normal.z /= len;
			}

			/* Done with relativistic transforms */

			/* Clamp color components to legal range */
			color.r = MIN(1.0, color.r);
			color.g = MIN(1.0, color.g);
			color.b = MIN(1.0, color.b);

			/* Lastly, perform display gamma correction if needed */
			if (dgamma_correct) {
				color.r = dgamma_lut[(int)(color.r * LUT_RES)];
				color.g = dgamma_lut[(int)(color.g * LUT_RES)];
				color.b = dgamma_lut[(int)(color.b * LUT_RES)];
			}

			pnt = &obj->iarrays[vn];
			/* Store processed vertex location */
			pnt->x = vertex.x;
			pnt->y = vertex.y;
			pnt->z = vertex.z;
			/* Store processed vertex normal */
			pnt->nx = normal.x;
			pnt->ny = normal.y;
			pnt->nz = normal.z;
			/* Store processed vertex color */
			pnt->r = color.r;
			pnt->g = color.g;
			pnt->b = color.b;
		}
	}

	return 0;
}


/* This performs the relativistic geometrical transform, but for a single point,
 * with the camera at the specified location */
void
warp_point( point *vertex, point *normal, point *cam_pos )
{
	double gamma;
	double d_vertex_x;
	double dx,dy,dz;
	double len;
	double dyz2, dist2;
	double v, v2, t;

	gamma = lorentz_factor( velocity );

	/* Perform calculations in double precision */
	d_vertex_x = vertex->x;

	/** Lorentz contraction **/
	d_vertex_x /= gamma;

	/* Adjust normal, if we have a normal to adjust */
	if (normal != NULL) {
		dx = normal->x;
		dy = normal->y / gamma;
		dz = normal->z / gamma;

		/* Renormalize */
		len = sqrt( SQR(dx) + SQR(dy) + SQR(dz) );
		if (len < 1E-6)
			len = 1.0;
		normal->x = dx / len;
		normal->y = dy / len;
		normal->z = dz / len;
	}

	/* Move object to its "real" x-position */
	d_vertex_x += velocity * cur_time_t;

	dx = d_vertex_x - cam_pos->x;
	dy = vertex->y - cam_pos->y;
	dz = vertex->z - cam_pos->z;

	dyz2 = SQR(dy) + SQR(dz);
	dist2 = SQR(dx) + SQR(dy) + SQR(dz);
	v = velocity;
	v2 = SQR(velocity);

	/* Calculate t and adjust vertex accordingly */
	t = (dx*v - sqrt( C2*dist2 - dyz2*v2 )) / (v2 - C2);
	d_vertex_x -= velocity * t;

	vertex->x = d_vertex_x;
}


/* Doppler frequency shift color transform
 * My deepest appreciation to Antony Searle for letting me use his code :-) */
static void
doppler_shift( rgb_color *color, float freq_ratio )
{
	float r0,g0,b0;
	float k;

	r0 = color->r;
	g0 = color->g;
	b0 = color->b;

	/* Convert frequency ratio into wavelength ratio */
	k = 1.0 / freq_ratio;

	/* Calculate red component */
	if (LAMBDA_RED < (k * LAMBDA_UV))
		color->r = LAMBDA_RED * ((0.5 * b0) + (0.25 * g0) + (0.125 * r0))/ (k * LAMBDA_UV);
	else if (LAMBDA_RED < (k * LAMBDA_BLUE))
		color->r = ((0.5 * b0) + (0.25 * g0) + (0.125 * r0)) + ((LAMBDA_RED - (k * LAMBDA_UV)) / ((k * LAMBDA_BLUE) - (k * LAMBDA_UV))) * ((0.5 * b0) - (0.25 * g0) - (0.125 * r0));
	else if (LAMBDA_RED < (k * LAMBDA_GREEN))
		color->r = b0 + ((LAMBDA_RED - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_RED < (k * LAMBDA_RED))
		color->r = g0 + ((LAMBDA_RED - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else if (LAMBDA_RED < (k * LAMBDA_IR))
		color->r = r0 + ((LAMBDA_RED - (k * LAMBDA_RED)) / ((k * LAMBDA_IR) - (k * LAMBDA_RED))) * ((0.125 * b0) + (0.25 * g0) - (0.5 * r0));
	else
		color->r = ((0.125 * b0) + (0.25 * g0) + (0.5 * r0)) * ((k * LAMBDA_IR) / LAMBDA_RED);

	/* Calculate green component */
	if (LAMBDA_GREEN < (k * LAMBDA_UV))
		color->g = LAMBDA_GREEN * ((0.5 * b0) + (0.25 * g0) + (0.125 * r0))/ (k * LAMBDA_UV);
	else if (LAMBDA_GREEN < (k * LAMBDA_BLUE))
		color->g = ((0.5 * b0) + (0.25 * g0) + (0.125 * r0)) + ((LAMBDA_GREEN - (k * LAMBDA_UV)) / ((k * LAMBDA_BLUE) - (k * LAMBDA_UV))) * ((0.5 * b0) - (0.25 * g0) - (0.125 * r0));
	else if (LAMBDA_GREEN < (k * LAMBDA_GREEN))
		color->g = b0 + ((LAMBDA_GREEN - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_GREEN < (k * LAMBDA_RED))
		color->g = g0 + ((LAMBDA_GREEN - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else if (LAMBDA_GREEN < (k * LAMBDA_IR))
		color->g = r0 + ((LAMBDA_GREEN - (k * LAMBDA_RED)) / ((k * LAMBDA_IR) - (k * LAMBDA_RED))) * ((0.125 * b0) + (0.25 * g0) - (0.5 * r0));
	else
		color->g = ((0.125 * b0) + (0.25 * g0) + (0.5 * r0)) * ((k * LAMBDA_IR) / LAMBDA_GREEN);

	/* Calculate blue component */
	if (LAMBDA_BLUE < (k * LAMBDA_UV))
		color->b = LAMBDA_BLUE * ((0.5 * b0) + (0.25 * g0) + (0.125 * r0))/ (k * LAMBDA_UV);
	else if (LAMBDA_BLUE < (k * LAMBDA_BLUE))
		color->b = ((0.5 * b0) + (0.25 * g0) + (0.125 * r0)) + ((LAMBDA_BLUE - (k * LAMBDA_UV)) / ((k * LAMBDA_BLUE) - (k * LAMBDA_UV))) * ((0.5 * b0) - (0.25 * g0) - (0.125 * r0));
	else if (LAMBDA_BLUE < (k * LAMBDA_GREEN))
		color->b = b0 + ((LAMBDA_BLUE - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_BLUE < (k * LAMBDA_RED))
		color->b = g0 + ((LAMBDA_BLUE - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else if (LAMBDA_BLUE < (k * LAMBDA_IR))
		color->b = r0 + ((LAMBDA_BLUE - (k * LAMBDA_RED)) / ((k * LAMBDA_IR) - (k * LAMBDA_RED))) * ((0.125 * b0) + (0.25 * g0) - (0.5 * r0));
	else
		color->b = ((0.125 * b0) + (0.25 * g0) + (0.5 * r0)) * ((k * LAMBDA_IR) / LAMBDA_BLUE);
}


/* Doppler frequency shift, for reflected light rays
 * Again, this is straight from BACKLIGHT */
static void
doppler_shift_ref( rgb_color *color, float freq_ratio )
{
	float r0,g0,b0;
	float k;

	r0 = color->r;
	g0 = color->g;
	b0 = color->b;

	/* Convert frequency ratio into wavelength ratio */
	k = 1.0 / freq_ratio;

	/* Calculate red component */
	if (LAMBDA_RED < (k * LAMBDA_BLUE))
		color->r = b0;
	else if (LAMBDA_RED < (k * LAMBDA_GREEN))
		color->r = b0 + ((LAMBDA_RED - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_RED < (k * LAMBDA_RED))
		color->r = g0 + ((LAMBDA_RED - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else
		color->r = r0;

	/* Calculate green component */
	if (LAMBDA_GREEN < (k * LAMBDA_BLUE))
		color->g = b0;
	else if (LAMBDA_GREEN < (k * LAMBDA_GREEN))
		color->g = b0 + ((LAMBDA_GREEN - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_GREEN < (k * LAMBDA_RED))
		color->g = g0 + ((LAMBDA_GREEN - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else
		color->g = r0;

	/* Calculate blue component */
	if (LAMBDA_BLUE < (k * LAMBDA_BLUE))
		color->b = b0;
	else if (LAMBDA_BLUE < (k * LAMBDA_GREEN))
		color->b = b0 + ((LAMBDA_BLUE - (k * LAMBDA_BLUE)) / ((k * LAMBDA_GREEN) - (k * LAMBDA_BLUE))) * (- b0 + g0);
	else if (LAMBDA_BLUE < (k * LAMBDA_RED))
		color->b = g0 + ((LAMBDA_BLUE - (k * LAMBDA_GREEN)) / ((k * LAMBDA_RED) - (k * LAMBDA_GREEN))) * (- g0 + r0);
	else
		color->b = r0;
}


/* This next function takes care of determining simulation time (cur_time_t).
 * If the vehicle is not being animated, warp_time( ) sets time t to keep the
 * image centered on the origin. If animation is active, time t is returned as
 * a percent between starting times t0 and t1, the times when the image appears
 * at the starting and ending x-positions x0 and x1, respectively (note that
 * while x0 and x1 stay constant, t0 and t1 will vary with camera position) */
void
warp_time( float x0, float x1, double value, int message )
{
	static float anim_x0 = 0.0;
	static float anim_x1 = 0.0;
	static float anim_percent = 0.0;
	camera *cam;
	double dx,dy,dz;
	double anim_t0;
	double anim_t1;
	double v, t;
	float loop_time = 0.0;

	/* Preliminary */
	if (message == WARP_BEGIN_ANIM) {
		anim_x0 = x0;
		anim_x1 = x1;
		/* Make SURE these two are not still in transition queue
		 * (i.e. if user hits Begin suddenly after Stop) */
		break_transition( &anim_x0 );
		break_transition( &anim_x1 );
		loop_time = value;
		v = velocity;
	}
	else
		v = value; /* effective velocity for calculations */

/* BUG (sort of): Actual velocity (not effective velocity) is used when
 * beginning an animation. This means that if the user begins an animation
 * while optical deformation is disabled, the object will leap back a small
 * distance before moving forward */

	/* Everything is done w.r.t. the primary camera */
	cam = usr_cams[0];

	/* Recalculate starting time t (image at x=x0) */
	dx = cam->pos.x - anim_x0;
	dy = cam->pos.y;
	dz = cam->pos.z;
	t = anim_x0 / v;
	anim_t0 = t + sqrt( SQR(dx) + SQR(dy) + SQR(dz) ) / C;

	/* Recalculate ending time t (image at x=x1) */
	dx = cam->pos.x - anim_x1;
	t = anim_x1 / v;
	anim_t1 = t + sqrt( SQR(dx) + SQR(dy) + SQR(dz) ) / C;

	switch (message) {
	case WARP_UPDATE_TIME_T:
		break;

	case WARP_BEGIN_ANIM:
		/* Set anim_percent so object doesn't initially JUMP to x0 */
		anim_percent = (cur_time_t - anim_t0) / (anim_t1 - anim_t0);
		animate( &anim_percent, FALSE, TRANS_LINEAR, loop_time, 0.0, 1.0, -1 );
		return;

	case WARP_STOP_ANIM:
		/* Move image back to origin */
		transition( &anim_x0, FALSE, TRANS_SIGMOID, 2.0, 0.0, -1 );
		transition( &anim_x1, FALSE, TRANS_SIGMOID, 2.0, 0.0, -1 );
		/* Get anim_percent out of the transition queue */
		transition( &anim_percent, FALSE, TRANS_LINEAR, 0.0, anim_percent, -1 );
		return;

	default:
#ifdef DEBUG
		crash( "warp_time( ): invalid message" );
#endif
		return;
	}

	/* Update current time t */
	cur_time_t = anim_t0 + anim_percent * (anim_t1 - anim_t0);
}


/* The Lorentz factor, a.k.a. the gamma factor
 * Dependent on velocity, with range [1, infinity) */
double
lorentz_factor( double v )
{
	return 1 / sqrt( 1 - SQR(v) / C2 );
}

/* end warp.c */
