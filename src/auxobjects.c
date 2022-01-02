/* auxobjects.c */

/* Auxiliary objects / reference geometry drawing routines */

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
static void draw_coord_axes( float percent, point *cam_pos );
static void draw_floating_grid( float percent );
static void draw_floating_grid_AUX( float unit_size, int i_min, int i_max, int j_min, int j_max, float vis, float side );
static void draw_bounding_box( float percent );


/* Auxiliary objects control */
void
auxiliary_objects( int message1, int message2 )
{
	static float percent_axes = 0.0;
	static float percent_grid = 1.0;
	static float percent_bbox = 0.0;
	static int axes_active = FALSE;
	static int grid_active = TRUE;
	static int bbox_active = FALSE;
	camera *cam;

	switch (message1) {
	case AUXOBJS_DRAW:
		cam = usr_cams[message2];
		break;

	case AUXOBJS_SET_AXES:
		axes_active = message2;
		if (axes_active)
			transition(  &percent_axes, FALSE, TRANS_RAMP, 2, 1.0, -1 );
		else
			transition( &percent_axes, FALSE, TRANS_QTR_SIN, 1, 0.0, -1 );
		return;

	case AUXOBJS_SET_BBOX:
		bbox_active = message2;
		if (bbox_active)
			transition(  &percent_bbox, FALSE, TRANS_RAMP, 2, 1.0, -1 );
		else
			transition( &percent_bbox, FALSE, TRANS_QTR_SIN, 1, 0.0, -1 );
		return;

	case AUXOBJS_SET_GRID:
		grid_active = message2;
		if (grid_active)
			transition(  &percent_grid, FALSE, TRANS_QTR_SIN, 2, 1.0, -1 );
		else {
			percent_grid = -1.0;
			transition( &percent_grid, FALSE, TRANS_RAMP, 1, 0.0, -1 );
		}
		return;

	default:
#ifdef DEBUG
		crash( "aux_objects( ): invalid message" );
#endif
		return;
	}

	glDisable( GL_LIGHTING );

	if (percent_axes != 0.0)
		draw_coord_axes( percent_axes, &cam->pos );

	if (percent_bbox != 0.0)
		draw_bounding_box( percent_bbox );

	if (percent_grid != 0.0)
		draw_floating_grid( percent_grid );

	glEnable( GL_LIGHTING );
}


static void
draw_coord_axes( float percent, point *cam_pos )
{
	float space, scale;
	float x_dist;
	float neg_x0, neg_y0, neg_z0;
	float neg_x1, neg_y1, neg_z1;
	float pos_x0, pos_y0, pos_z0;
	float pos_x1, pos_y1, pos_z1;
	float rot_x, rot_y, rot_z;
	float arrow1, arrow2;
	float label1, label2;
	float label2_x;

	/* These will be used to draw the axis arrowheads */
	arrow1 = vehicle_extents.avg * percent / 8;
	arrow2 = arrow1 * SQR(MAGIC_NUMBER);

	/* These will be used to draw the X, Y and Z labels */
	label1 = vehicle_extents.avg * percent / 8;
	label2 = label1 * MAGIC_NUMBER;

	if (percent > 0.01)
		space = vehicle_extents.avg / 4 / SQR(percent);
	else
		space = vehicle_extents.avg * 1000;
	scale = 1.0 + SQR(percent);

	x_dist = vehicle_extents.xmax + space;
	pos_x0 = x_dist;
	pos_x1 = x_dist * scale;
	x_dist = vehicle_extents.xmin - space;
	neg_x0 = x_dist;
	neg_x1 = x_dist * scale - arrow2 / 2.0;

	pos_y0 = vehicle_extents.ymax + space;
	pos_y1 = pos_y0 * scale;
	neg_y0 = vehicle_extents.ymin - space;
	neg_y1 = MIN(neg_y0 * scale, - pos_y1) - arrow2 / 2.0;

	pos_z0 = vehicle_extents.zmax + space;
	pos_z1 = pos_z0 * scale;
	neg_z0 = vehicle_extents.zmin - space;
	neg_z1 = MIN(neg_z0 * scale, - pos_z1) - arrow2 / 2.0;

	/* These will keep the arrowheads and labels facing the camera */
	rot_x = DEG(atan2( cam_pos->z, cam_pos->y ));
	rot_y = DEG(atan2( cam_pos->z, cam_pos->x ));
	rot_z = DEG(atan2( cam_pos->y, cam_pos->x ));

	/* label2_x is used to flip the Y if x-pos. of camera is negative */
	if (cam_pos->x < 0.0)
		label2_x = - label2;
	else
		label2_x = label2;

	/* Draw the terminating dots */
	glPointSize( 8 );
	glBegin( GL_POINTS );
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( neg_x0, 0, 0 );
	glVertex3f( pos_x0, 0, 0 );
	glColor3f( 1.0, 1.0, 0.0 );
	glVertex3f( 0, neg_y0, 0 );
	glVertex3f( 0, pos_y0, 0 );
	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( 0, 0, neg_z0 );
	glVertex3f( 0, 0, pos_z0 );
	glEnd( );

	/* Draw the axes */
	glLineWidth( 5 );
	glBegin( GL_LINES );
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( neg_x0, 0, 0 );
	glVertex3f( neg_x1, 0, 0 );
	glVertex3f( pos_x0, 0, 0 );
	glVertex3f( pos_x1, 0, 0 );
	glColor3f( 1.0, 1.0, 0.0 );
	glVertex3f( 0, neg_y0, 0 );
	glVertex3f( 0, neg_y1, 0 );
	glVertex3f( 0, pos_y0, 0 );
	glVertex3f( 0, pos_y1, 0 );
	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( 0, 0, neg_z0 );
	glVertex3f( 0, 0, neg_z1 );
	glVertex3f( 0, 0, pos_z0 );
	glVertex3f( 0, 0, pos_z1 );
	glEnd( );

	glPushMatrix( );
	glRotatef( rot_x, 1.0, 0.0, 0.0 );

	/* Draw the x-axis arrowhead */
	glBegin( GL_TRIANGLE_FAN );
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( pos_x1, 0, 0 );
	glVertex3f( pos_x1 - arrow1, 0, arrow1 / 2 );
	glVertex3f( pos_x1 + arrow2, 0, 0 ); /* tip */
	glVertex3f( pos_x1 - arrow1, 0, - arrow1 / 2 );
	glEnd( );

	/* Draw the X */
	glBegin( GL_LINES );
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( neg_x1 - label2 - label1, 0, label2 ); /* top left */
	glVertex3f( neg_x1 - label2 + label1, 0, - label2 ); /* bottom right */
	glVertex3f( neg_x1 - label2 - label1, 0, - label2 ); /* bottom left */
	glVertex3f( neg_x1 - label2 + label1, 0, label2 ); /* top right */
	glEnd( );

	glPopMatrix( );
	glPushMatrix( );
	glRotatef( rot_y, 0.0, -1.0, 0.0 );

	/* Draw the y-axis arrowhead */
	glBegin( GL_TRIANGLE_FAN );
	glColor3f( 1.0, 1.0, 0.0 );
	glVertex3f( 0, pos_y1, 0 );
	glVertex3f( 0, pos_y1 - arrow1, - arrow1 / 2 );
	glVertex3f( 0, pos_y1 + arrow2, 0 ); /* tip */
	glVertex3f( 0, pos_y1 - arrow1, arrow1 / 2 );
	glEnd( );

	/* Draw the Y */
	glBegin( GL_LINES );
	glColor3f( 1.0, 1.0, 0.0 );
	glVertex3f( 0, neg_y1 - label2 - label1, label2_x ); /* top left */
	glVertex3f( 0, neg_y1 - label2, 0 ); /* middle */
	glVertex3f( 0, neg_y1 - label2 + label1, label2_x ); /* top right */
	glVertex3f( 0, neg_y1 - label2, 0 ); /* middle */
	glVertex3f( 0, neg_y1 - label2, 0 ); /* middle */
	glVertex3f( 0, neg_y1 - label2, - label2_x ); /* bottom */
	glEnd( );

	glPopMatrix( );
	glPushMatrix( );
	glRotatef( rot_z, 0.0, 0.0, 1.0 );

	/* Draw the z-axis arrowhead */
	glBegin( GL_TRIANGLE_FAN );
	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( 0, 0, pos_z1 );
	glVertex3f( 0, arrow1 / 2, pos_z1 - arrow1 );
	glVertex3f( 0, 0, pos_z1 + arrow2 ); /* tip */
	glVertex3f( 0, - arrow1 / 2, pos_z1 - arrow1 );
	glEnd( );

	/* Draw the Z */
	glBegin( GL_LINE_STRIP );
	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( 0, - label1, neg_z1 - label2 ); /* top left */
	glVertex3f( 0, label1, neg_z1 - label2 );
	glVertex3f( 0, - label1, neg_z1 - 3 * label2 );
	glVertex3f( 0, label1, neg_z1 - 3 * label2 ); /* bottom right */
	glEnd( );

	glPopMatrix( );
}


static void
draw_floating_grid( float percent )
{
	float angle;
	float vis;
	float unit_size;
	float x_length, y_length;
	float y_dist;
	int i_min, j_min;
	int i_max, j_max;

	/* Deployment rotation */
	angle = 180.0 * (percent - 1.0);

	/* Deployment visibility */
	if (percent > 0.0)
		vis = percent;
	else
		vis = sqrt( - percent );

	/* Determine unit_size suited to vehicle dimensions */
	unit_size = pow( 10, floor( log10( vehicle_extents.avg ) ) );

	/* and grid extents */
	x_length = vehicle_extents.xmax - vehicle_extents.xmin;
	y_length = vehicle_extents.ymax - vehicle_extents.ymin;
	y_dist = MAX(vehicle_extents.ymax, - vehicle_extents.ymin);
	y_dist += y_length / 4;

	i_max = (int)(ceil( 4 * x_length / unit_size )) / 2;
	i_min = - i_max;
	j_max = (int)(ceil( 4 * y_dist / unit_size ));
	j_min = (int)(ceil( y_dist / unit_size ));

	glPushMatrix( );
	glRotatef( angle, 0.0, 1.0, 0.0 );
	glEnable( GL_BLEND ); /* for fadein/fadeout */

	draw_floating_grid_AUX( unit_size, i_min, i_max, j_min, j_max, vis, 1.0 );

	glPopMatrix( );
	glPushMatrix( );
	glRotatef( - angle, 0.0, 1.0, 0.0 );

	draw_floating_grid_AUX( unit_size, i_min, i_max, j_min, j_max, vis, -1.0 );

	glDisable( GL_BLEND );
	glPopMatrix( );
}

/* ThisIsAHelperFunction */
static void
draw_floating_grid_AUX( float unit_size, int i_min, int i_max, int j_min, int j_max, float vis, float side )
{
	float x0, y0;
	float x1, y1;
	float x, y;
	int i, j;

	x0 = unit_size * (float)i_min;
	x1 = unit_size * (float)i_max;
	y0 = unit_size * (float)j_min;
	y1 = unit_size * (float)j_max;

	glLineWidth( 1 );
	glBegin( GL_LINES );
	glColor4f( 0.5, 0.5, 0.5, vis );

	/* x grid lines */
	for (j = j_min; j < j_max; j++) {
		y = unit_size * (float)j;
		glVertex3f( x0, side * y, 0 );
		glVertex3f( x1, side * y, 0 );
	}

	/* y grid lines */
	for (i = i_min + 1; i < i_max; i++) {
		x = unit_size * (float)i;
		glVertex3f( x, side * y0, 0 );
		glVertex3f( x, side * y1, 0 );
	}
	glEnd( );
	glLineWidth( 3 );

	/* Outer boundaries */
	glBegin( GL_LINE_STRIP );
	glColor4f( 0.75, 0.75, 0.75, vis );
	glVertex3f( x0, side * y0, 0 );
	glVertex3f( x0, side * y1, 0 );
	glVertex3f( x1, side * y1, 0 );
	glVertex3f( x1, side * y0, 0 );
	glEnd( );

	/* Floating grid determines world extents */
	world_extents.xmin = x0;
	world_extents.xmax = x1;
}


static void
draw_bounding_box( float percent )
{
	static int b[] = { 1, -1, -1, 2 }; /* Elements 0 and 3 are important */
	extents *exts;
	float bar_percent;
	float gamma;
	float xbar, ybar, zbar; /* Length of x, y, z bars */
	float bb_x[4], bb_y[4], bb_z[4]; /* x, y, z coords. of box/bars */
	int i, j, k;

	if (percent < 0.01)
		return;

	bar_percent = (0.25 / MAGIC_NUMBER) * percent;
	gamma = lorentz_factor( velocity );
	exts = &vehicle_extents; /* abbreviation */

	xbar = (exts->xmax - exts->xmin) * bar_percent;
	bb_x[0] = (exts->xmin / gamma + vehicle_real_x) / percent;
	bb_x[1] = ((exts->xmin + xbar) / gamma + vehicle_real_x) / percent;
	bb_x[2] = ((exts->xmax - xbar) / gamma + vehicle_real_x) / percent;
	bb_x[3] = (exts->xmax / gamma + vehicle_real_x) / percent;

	ybar = (exts->ymax - exts->ymin) * bar_percent;
	bb_y[0] = exts->ymin / percent;
	bb_y[1] = (exts->ymin + ybar) / percent;
	bb_y[2] = (exts->ymax - ybar) / percent;
	bb_y[3] = exts->ymax / percent;

	zbar = (exts->zmax - exts->zmin) * bar_percent;
	bb_z[0] = exts->zmin / percent;
	bb_z[1] = (exts->zmin + zbar) / percent;
	bb_z[2] = (exts->zmax - zbar) / percent;
	bb_z[3] = exts->zmax / percent;

	glLineWidth( 5 );
	glBegin( GL_LINES );
	glColor3f( 1.0, 1.0, 1.0 );
	for (i = 0; i <= 3; i += 3) {
		for (j = 0; j <= 3; j += 3) {
			for (k = 0; k <= 3; k += 3) {
				glVertex3f( bb_x[i], bb_y[j], bb_z[k] );
				glVertex3f( bb_x[b[i]], bb_y[j], bb_z[k] );

				glVertex3f( bb_x[i], bb_y[j], bb_z[k] );
				glVertex3f( bb_x[i], bb_y[b[j]], bb_z[k] );

				glVertex3f( bb_x[i], bb_y[j], bb_z[k] );
				glVertex3f( bb_x[i], bb_y[j], bb_z[b[k]] );
			}
		}
	}
	glEnd( );
}

/* end auxobjects.c */
