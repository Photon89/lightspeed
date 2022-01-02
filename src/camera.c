/* camera.c */

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


camera *
new_camera( void )
{
	int new_cam_id;

	new_cam_id = num_cams;
	++num_cams;
	usr_cams = xrealloc( usr_cams, num_cams * sizeof(camera *) );
	usr_cams[new_cam_id] = xmalloc( sizeof(camera) );
	cur_cam = new_cam_id;

	if (new_cam_id > 0) {
		/* Initialize camera with primary's state */
		memcpy( usr_cams[new_cam_id], usr_cams[0], sizeof(camera) );
	}
	else {
		/* This IS the primary */
		camera_reset( 0 );
	}

	/* Widget pointers will be assigned elsewhere
	 * (in main_window( ) for primary, in menu_Camera_Spawn( ) for others) */
	usr_cams[new_cam_id]->window_w = NULL;
	usr_cams[new_cam_id]->ogl_w = NULL;

	return usr_cams[new_cam_id];
}


/* Gets rid of a camera, along with accompanying window and GL widgets */
void
kill_camera( int cam_id )
{
	gtk_widget_destroy( usr_cams[cam_id]->ogl_w );
	gtk_widget_destroy( usr_cams[cam_id]->window_w );
	xfree( usr_cams[cam_id] );
	--num_cams;
	if (cam_id != num_cams) /* Not last camera? */
		memmove( &usr_cams[cam_id], &usr_cams[cam_id + 1], (num_cams - cam_id) * sizeof(camera *) );
	usr_cams = xrealloc( usr_cams, num_cams * sizeof(camera *) );
	cur_cam = 0;
#ifdef DEBUG
	printf( "Killed camera %d\n", cam_id );
#endif
}


/* Initialize camera with default initial settings */
void
camera_reset( int cam_id )
{
	camera *cam;

	cam = usr_cams[cam_id];
	/* Convert user phi/theta to struct camera phi/theta values */
	cam->phi = fmod( DEF_CAMERA_PHI + 180.0, 360.0) ;
	cam->theta = - DEF_CAMERA_THETA;
	cam->distance = 3 * vehicle_extents.avg;
	cam->target.x = 0;
	cam->target.y = 0;
	cam->target.z = 0;
	cam->fov = DEF_CAMERA_FOV;
	cam->near_clip = vehicle_extents.avg / 20;
	cam->far_clip = vehicle_extents.avg * 50;
	camera_calc_xyz( CAM_POSITION, cam );

	/* Update Camera Position dialog if it is active */
	dialog_Camera_Position( NULL, MESG_(RESET) );

	queue_redraw( cam_id );
}


/* This function returns the ID number of the camera associated
 * with the specified window or GL widget */
int
assoc_cam_id( void *widget )
{
	camera *cam;
	int i;

	for (i = 0; i < num_cams; i++) {
		cam = usr_cams[i];
		if ((widget == cam->window_w) || (widget == cam->ogl_w))
			return i;
	}

	return -1; /* No camera associated with this widget */
}


/* Focusing a window updates cur_cam */
int
camera_set_current( GtkWidget *widget, GdkEventFocus *ev_focus, void *nothing )
{
	if (num_cams > 1) {
		cur_cam = assoc_cam_id( widget );
#ifdef DEBUG
		printf( "Current camera: %d\n", cur_cam );
		fflush( stdout );
#endif
	}

	return FALSE;
}


/* Obtain a field of view (in degrees) from a lens length (in mm) */
float
camera_calc_fov( float lens_length )
{
	return DEG(2 * atan( 21.6663 / (float)lens_length ));
}


/* Obtain a lens length (in mm) from a field of view (in degrees) */
float
camera_calc_lens_length( float fov )
{
	return 21.6663 / tan( RAD(fov) / 2 );
}


/* Calculates a camera's XYZ position or target, given that the other point
 * (target or position) and phi + theta + distance are defined */
void
camera_calc_xyz( int point_type, camera *cam )
{
	float sin_phi, cos_phi, sin_theta, cos_theta;
	float dx, dy, dz;

	sin_phi = sin( RAD(cam->phi) );
	cos_phi = cos( RAD(cam->phi) );
	sin_theta = sin( RAD(cam->theta) );
	cos_theta = cos( RAD(cam->theta) );

	dx = cam->distance * cos_theta * cos_phi;
	dy = cam->distance * cos_theta * sin_phi;
	dz = cam->distance * sin_theta;

	switch (point_type) {
	case CAM_POSITION:
		cam->pos.x = cam->target.x + dx;
		cam->pos.y = cam->target.y + dy;
		cam->pos.z = cam->target.z + dz;
		break;

	case CAM_TARGET:
		cam->target.x = cam->pos.x - dx;
		cam->target.y = cam->pos.y - dy;
		cam->target.z = cam->pos.z - dz;
		break;

	default:
#ifdef DEBUG
		crash( "camera_calc_xyz( ): invalid type" );
#endif
		return;
	}
}


/* Calculate a target/distance for a camera which only has a location and a
 * phi/theta direction. The target point should be chosen such that it lies on
 * the camera's view ray, and is as close to the origin as possible */
void
camera_make_target( camera *cam )
{
	float sin_phi, sin_theta;
	float cos_phi, cos_theta;
	float ux, uy, uz;
	float dist;

	sin_phi = sin( RAD(cam->phi) );
	cos_phi = cos( RAD(cam->phi) );
	sin_theta = sin( RAD(cam->theta) );
	cos_theta = cos( RAD(cam->theta) );

	/* Calculate unit vector in direction of camera view */
	ux = cos_phi * cos_theta;
	uy = sin_phi * cos_theta;
	uz = sin_theta;

	/* Calculate distance to point on ray nearest the origin */
	dist = cam->pos.x * ux + cam->pos.y * uy + cam->pos.z * uz;

	/* If dist is negative, desired target is behind camera -- doh! */
	cam->distance = MAX(dist, vehicle_extents.avg / 8.0);
	camera_calc_xyz( CAM_TARGET, cam );
}


/* Read mouse input and move camera around accordingly */
/* Will be connected to mouse button & motion events */
int
camera_move( GtkWidget *widget, GdkEventAny *event, void *nothing )
{
	static camera *cam;
	static float mouse_prev_x;
	static float mouse_prev_y;
	static int cam_id;
	static int over_foreign = FALSE;
	GdkEventButton *ev_button;
	GdkEventMotion *ev_motion;
	float mouse_x, mouse_y;
	float mouse_dx, mouse_dy;
	float sin_phi, cos_phi, sin_theta, cos_theta;
	float k;
	int mouse_btn1 = FALSE, mouse_btn2 = FALSE, mouse_btn3 = FALSE;
	int shift_key = FALSE;

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		cam_id = assoc_cam_id( widget );
		cam = usr_cams[cam_id];
		cur_cam = cam_id;
		ev_button = (GdkEventButton *)event;
		/* Note click coordinates (for dragging) */
		mouse_prev_x = ev_button->x;
		mouse_prev_y = ev_button->y;
		/* Change cursor appropriately */
		if (ev_button->button == 1) {
			if (ev_button->state & GDK_SHIFT_MASK)
				set_cursor_glyph( GDK_CIRCLE );
			else
				set_cursor_glyph( GDK_FLEUR );
		}
		if (ev_button->button == 2)
			set_cursor_glyph( GDK_FLEUR );
		if (ev_button->button == 3)
			set_cursor_glyph( GDK_DOUBLE_ARROW );
		/* Finally, clear out any stale text in velocity entry */
		velocity_input( NULL, MESG_(RESET) );
		return FALSE;

		/* Double-click -- where are these events coming from? */
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		return FALSE;

	case GDK_BUTTON_RELEASE:
		set_cursor_glyph( GDK_LEFT_PTR );
		over_foreign = FALSE;
		return FALSE;

	case GDK_MOTION_NOTIFY:
		ev_motion = (GdkEventMotion *)event;
		mouse_x = ev_motion->x;
		mouse_y = ev_motion->y;
		if ((cam_id != assoc_cam_id( widget )) != over_foreign) {
			/* Fix for when pointer is dragged over another GL widget
			 * (x/y coords. jump to something else completely) */
			mouse_prev_x = mouse_x;
			mouse_prev_y = mouse_y;
			over_foreign = !over_foreign;
		}
		mouse_dx = mouse_sens * ( mouse_x - mouse_prev_x );
		mouse_dy = mouse_sens * ( mouse_y - mouse_prev_y );
		mouse_prev_x = mouse_x;
		mouse_prev_y = mouse_y;
		/* Get status of Shift key and buttons */
		shift_key = ev_motion->state & GDK_SHIFT_MASK;
		mouse_btn1 = ev_motion->state & GDK_BUTTON1_MASK;
		mouse_btn2 = ev_motion->state & GDK_BUTTON2_MASK;
		mouse_btn3 = ev_motion->state & GDK_BUTTON3_MASK;
		break;

	default:
#ifdef DEBUG
		crash( "camera_move( ): invalid message" );
#endif
		return FALSE;
	}

	/* Make sure cam->pos is not stale (for Shift + LeftClick) */
	camera_calc_xyz( CAM_POSITION, cam );

	/* Button 1: Revolve view around target or camera (Shift for latter) */
	if (mouse_btn1) {
		/* phi = heading, theta = elevation */
		cam->phi -= mouse_dx;
		cam->theta += mouse_dy;

		/* Keep angles within proper bounds */
		if (cam->phi < 0.0)
			cam->phi += 360.0;
		if (cam->phi > 360.0)
			cam->phi -= 360.0;
		if (cam->theta < -90.0)
			cam->theta = -90.0;
		if (cam->theta > 90.0)
			cam->theta = 90.0;
	}

	/* If Shift was pressed, need to update camera target */
	if (shift_key)
		camera_calc_xyz( CAM_TARGET, cam );

	sin_phi = sin( RAD(cam->phi) );
	cos_phi = cos( RAD(cam->phi) );
	sin_theta = sin( RAD(cam->theta) );
	cos_theta = cos( RAD(cam->theta) );

	/* Button 2: Dolly camera (up/down/left/right) */
	if (mouse_btn2) {
		k = cam->distance / 128;
		/* Isn't trig fun? */
		cam->target.x += k * ( mouse_dx * sin_phi -
		                       mouse_dy * sin_theta * cos_phi );
		cam->target.y -= k * ( mouse_dx * cos_phi +
		                       mouse_dy * sin_theta * sin_phi );
		cam->target.z += k * mouse_dy * cos_theta;
	}

	/* Button 3: Dolly camera (forward/backward) */
	if (mouse_btn3) {
		cam->distance -= (mouse_dy * vehicle_extents.avg / 48);
		cam->distance = MAX(cam->distance, vehicle_extents.avg / 8.0);
	}

	/* Now recalculate camera's xyz-position */
	camera_calc_xyz( CAM_POSITION, cam );

	/* Real quick, update Camera Position dialog if it is active */
	dialog_Camera_Position( NULL, MESG_(RESET) );

	/* Finally, ask for a redraw. NOTE: primary camera motion causes all
	 * cameras to be redrawn, as time t can vary with its position */
	if (cam_id != 0)
		queue_redraw( cam_id );
	else
		queue_redraw( -1 );

	return FALSE;
}

/* end camera.c */
