/* ogl.c */

/* Main OpenGL routines */

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


/* Initialize OpenGL state
 * (will be connected to the GL widget's "realize" signal) */
void
ogl_initialize( GtkWidget *ogl_w, void *nothing )
{
	float light0_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	float light0_diffuse[] = { 0.625, 0.625, 0.625, 1.0 };
	float light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	float light_model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	float light_pos[] = { 0.0, 0.0, 0.0, 1.0 };
	int on_screen = TRUE;

	if (ogl_w == NULL)
		on_screen = FALSE;

	if (on_screen)
		gtk_gl_area_make_current( GTK_GL_AREA(ogl_w) );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glLightfv( GL_LIGHT0, GL_AMBIENT, light0_ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light0_diffuse );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light0_specular );
	glRotatef( -90.0, 1.0, 0.0, 0.0 );
	glRotatef( -90.0, 0.0, 0.0, 1.0 );
	glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, light_model_ambient );
	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );

	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glShadeModel( GL_SMOOTH );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	/* Initialize ogl_draw_string for primary viewport and pixmap buffers
	 * (other viewports will get this via shared context) */
	if ((assoc_cam_id( ogl_w ) == 0) || !on_screen)
		ogl_draw_string( NULL, INITIALIZE, NIL );

	if (on_screen) {
		GTKGL_TEMP_endgl( GTK_GL_AREA(ogl_w) );

		/* Call ogl_resize( ) to finish viewport initialization */
		ogl_resize( ogl_w, NULL, NULL );
	}

#ifdef DEBUG
	if (on_screen)
		printf( "Initialized viewport %d\n", assoc_cam_id( ogl_w ) );
	else
		printf( "Initialized off-screen pixmap buffer\n" );
	fflush( stdout );
#endif
}


/* Change viewport size (after whatever window size adjustment)
 * Will be connected to the "configure_event" signal
 * Note: For some reason, this gets called once before ogl_initialize( )... */
int
ogl_resize( GtkWidget *ogl_w, GdkEventConfigure *ev_config, void *nothing )
{
	static float dummy;
	int width, height;
	int i;

	width = ogl_w->allocation.width;
	height = ogl_w->allocation.height;
	gtk_gl_area_make_current( GTK_GL_AREA(ogl_w) );
	glViewport( 0, 0, width, height );
	GTKGL_TEMP_endgl( GTK_GL_AREA(ogl_w) );

	/* Update dimensions in camera struct */
	i = assoc_cam_id( ogl_w );
	usr_cams[i]->width = width;
	usr_cams[i]->height = height;

	/* Recalibrate framerate */
	profile( PROFILE_FRAMERATE_RESET );
	dummy = 1.0;
	transition( &dummy, FALSE, TRANS_LINEAR, 1.0, 0.0, -1 );

#ifdef DEBUG
	printf( "Resized camera %d to %dx%d\n", i, width, height );
	fflush( stdout );
#endif

	return FALSE;
}


/* Refresh a viewport (static redraw)
 * Will be connected to the "expose_event" signal */
int
ogl_refresh( GtkWidget *ogl_w, GdkEventExpose *ev_expose, void *nothing )
{
	int i;

	i = assoc_cam_id( ogl_w );
	queue_redraw( i );

#ifdef SUPER_DEBUG
	printf( "Refresh camera %d\n", i );
	fflush( stdout );
#endif

	return FALSE;
}


/* Redraws the viewport of the camera indicated by cam_id
 * A cam_id of -1 means we're drawing the primary view into a pixmap buffer */
void
ogl_draw( int cam_id )
{
	camera *cam;
	ogl_object *obj;
	float r,g,b;
	float fr_x, fr_y;
	int drawing_to_screen = TRUE;
	int o, i, v;

	i = 0; /* Avoid pesky "unused variable..." warnings */
	v = 0; /* (if they come up-- see the #ifdef's) */

#ifdef SUPER_DEBUG
	if (cam_id >= 0)
		printf( "Drawing camera %d...", cam_id );
	else
		printf( "Drawing into off-screen pixmap..." );
	fflush( stdout );
#endif

	if (cam_id < 0) {
		drawing_to_screen = FALSE;
		cam = &out_cam;
		cam_id = 0;
	}
	else
		cam = usr_cams[cam_id];

	/* Apply relativistic distortions for this view */
	if (drawing_to_screen) {
		profile( PROFILE_WARP_BEGIN );
		warp( WARP_DISTORT, &cam->pos );
		profile( PROFILE_WARP_DONE );

		profile( PROFILE_OGLDRAW_BEGIN );
		gtk_gl_area_make_current( GTK_GL_AREA(cam->ogl_w) );
	}
	else
		warp( WARP_DISTORT, &cam->pos );

	r = background.r;
	g = background.g;
	b = background.b;
	if (dgamma_correct) {
		r = dgamma_lut[(int)(r * LUT_RES)];
		g = dgamma_lut[(int)(g * LUT_RES)];
		b = dgamma_lut[(int)(b * LUT_RES)];
	}
	glClearColor( r, g, b, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* Set view frustum (a.k.a. field of view) */
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	fr_x = cam->near_clip * tan( RAD(cam->fov) / 2 );
	fr_y = fr_x / ((float)cam->width / (float)cam->height);
	glFrustum( - fr_x, fr_x, - fr_y, fr_y, cam->near_clip, cam->far_clip );

	/* (Re)initialize transformation matrix */
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	/* Initialize the coordinate system to correct alignment */
	glRotatef( -90.0, 1.0, 0.0, 0.0 );
	glRotatef( -90.0, 0.0, 0.0, 1.0 );

	/* Perform world transformations */
	glTranslatef( - cam->distance, 0.0, 0.0 );
	glRotatef( cam->theta, 0.0, 1.0, 0.0 );
	glRotatef( - cam->phi, 0.0, 0.0, 1.0 );
	glTranslatef( - cam->target.x, - cam->target.y, - cam->target.z );

	/* For wireframe mode, if active */
	glLineWidth( 2 );

	/* Draw all vehicle objects */
	for (o = 0; o < num_vehicle_objs; o++) {
		obj = vehicle_objs[o];

		/* Execute "before" display list, if there is one */
		if (obj->pre_dlist != 0)
			glCallList( obj->pre_dlist );

#ifdef GL_VERSION_1_1
		glInterleavedArrays( GL_C4F_N3F_V3F, sizeof(ogl_point), obj->iarrays );
#ifdef GL_VERSION_1_2
		glDrawRangeElements( obj->type, 0, obj->num_vertices - 1, obj->num_indices, GL_UNSIGNED_INT, obj->indices );
#else
		glDrawElements( obj->type, obj->num_indices, GL_UNSIGNED_INT, obj->indices );
#endif /* else GL_VERSION_1_2 */
#else
		/* Fine, we'll do this the old-fashioned way */
		glBegin( obj->type );
		for (i = 0; i < obj->num_indices; i++) {
			v = obj->indices[i];
			glColor4fv( &obj->iarrays[v].r );
			glNormal3fv( &obj->iarrays[v].nx );
			glVertex3fv( &obj->iarrays[v].x );
		}
		glEnd( );
#endif /* else GL_VERSION_1_1 */

		/* Execute "after" display list if there is one */
		if (obj->post_dlist != 0)
			glCallList( obj->post_dlist );
	}

	/* Draw all active auxiliary objects */
	auxiliary_objects( AUXOBJS_DRAW, cam_id );

#if 0
	/* Draw normal lines (a.k.a. "hedgehogification")
	 * (for debugging purposes) */
	if (FALSE) {
		glLineWidth( 1 );
		glBegin( GL_LINES );
		for (o = 0; o < num_vehicle_objs; o++) {
			obj = vehicle_objs[o];
			for (v = 0; v < obj->num_vertices; v++) {
				float x, y, z;

				x = obj->vertices[v].x;
				y = obj->vertices[v].y;
				z = obj->vertices[v].z;
				glVertex3f( x, y, z );

				x += obj->normals[v].x;
				y += obj->normals[v].y;
				z += obj->normals[v].z;
				glVertex3f( x, y, z );
			}
		}
		glEnd( );
	}
#endif /* 0 */

	/* Initialize string drawer (i.e. inform of viewport dimensions) */
	ogl_draw_string( cam, RESET, NIL );

	/* Finally, draw info display (nothing if it's turned off)
	 * Only the primary camera or an off-screen image gets this */
	if ((cam_id == 0) || !drawing_to_screen)
		info_display( INFODISP_DRAW, NIL );

	if (drawing_to_screen) {
		GTKGL_TEMP_endgl( GTK_GL_AREA(cam->ogl_w) );
		gtk_gl_area_swapbuffers( GTK_GL_AREA(cam->ogl_w) );
		profile( PROFILE_OGLDRAW_DONE );
		cam->redraw = FALSE;
	}

#ifdef SUPER_DEBUG
	printf( "done.\n" );
	fflush( stdout );
#endif
}


/* Draws a string in the viewport
 * Meaning of args can vary, see initial switch statement
 * "size" specifies size of text: 0 (small), 1 (medium) or 2 (large)
 * NOTE: This function requires some pre-existing state; namely, the target
 * GL context must already be made current as well as properly initialized
 * (see ogl_blank( ) or info_display( ) to see what I mean) */
void
ogl_draw_string( const void *data, int message, int size )
{
	static GdkFont **fonts;
	static unsigned int *font_dlist_bases;
	static int *font_heights;
	static int width, height;
	static int fn_big;
	static int num_tl_lines, num_tr_lines, num_bl_lines, num_br_lines;
	static int num_cen_lines;
	const char *test_str = "XXXX XXXX XXXX XXXX";
	camera *cam;
	int pos_code;
	int edge_dx = 1, edge_dy = 1;
	int x = 0, y = 0;
	int fn;
	int i;
	char str_buf[256];
	char *disp_str;
	char *next_disp_str;

	switch (message) {
	case INITIALIZE:
		/* First-time initialization */
		fonts = xmalloc( num_font_sizes * sizeof(GdkFont *) );
		font_dlist_bases = xmalloc( num_font_sizes * sizeof(unsigned int *) );
		font_heights = xmalloc( num_font_sizes * sizeof(int *) );
		for (i = 0; i < num_font_sizes; i++) {
			fonts[i] = gdk_font_load( sys_font_names[i] );
			if (fonts[i] == NULL) {
				printf( "ERROR: Cannot load font: %s\n", sys_font_names[i] );
				fflush( stdout );
				continue;
			}
			font_heights[i] = fonts[i]->ascent + fonts[i]->descent;

			font_dlist_bases[i] = glGenLists( 192 );
			gdk_gl_use_gdk_font( fonts[i], 0, 192, font_dlist_bases[i] );
		}
		return;

	case RESET:
		/* Once-per-GL-redraw initialization */
		cam = (camera *)data;
		/* Get GL widget dimensions */
		width = cam->width;
		height = cam->height;
		/* Reset line counters */
		num_tl_lines = 0;
		num_tr_lines = 0;
		num_bl_lines = 0;
		num_br_lines = 0;
		num_cen_lines = 0;
		/* Determine upper limit on the font sizes we should use */
		for (i = 0; i < num_font_sizes; i++) {
			fn_big = i; /* font number of "big" (size 2) font */
			/* Test string should be minimally 1/3 viewport width */
			if (gdk_string_width( fonts[i], test_str ) > (width / 3))
				break;
		}
		return;

	default:
		pos_code = message;
		disp_str = (char *)data;
		break;
	}

	/* Check string for newlines, and queue if necessary */
	i = strcspn( disp_str, "\n" );
	if (i < strlen( disp_str )) {
		strcpy( str_buf, disp_str );
		str_buf[i] = '\0';
		disp_str = str_buf;
		next_disp_str = &str_buf[i + 1];
	}
	else
		next_disp_str = NULL;

	/* Determine which (proportional) font size to use */
	fn = MIN(fn_big, MAX(0, fn_big - 2 + size)); /* 0 <= fn <= fn_big */

	/* x coord. of base point */
	switch (pos_code) {
	case POS_TOP_LEFT:
	case POS_BOTTOM_LEFT:
		x = width / 100;
		edge_dx = 1;
		break;

	case POS_TOP_RIGHT:
	case POS_BOTTOM_RIGHT:
		x = (width * 99) / 100;
		x -= gdk_string_width( fonts[fn], disp_str );
		edge_dx = -1;
		break;

	case POS_CENTER:
		x = width / 2;
		x -= gdk_string_width( fonts[fn], disp_str ) / 2;
		edge_dx = 0;
		break;

	default:
#ifdef DEBUG
		crash( "ogl_draw_string( ): invalid position code" );
#endif
		return;
	}

	/* y coord. of base point */
	switch (pos_code) {
	case POS_BOTTOM_LEFT:
	case POS_BOTTOM_RIGHT:
		y = height / 100;
		edge_dy = 1;
		break;

	case POS_TOP_LEFT:
	case POS_TOP_RIGHT:
		y = (99 * height) / 100;
		y -= font_heights[fn];
		edge_dy = -1;
		break;

	case POS_CENTER:
		y = height / 2;
		y -= font_heights[fn] / 2;
		edge_dy = -1;
		break;
	}

	/* This makes multi-line readouts possible */
	switch (pos_code) {
	case POS_TOP_LEFT:
		y -= num_tl_lines * font_heights[fn];
		++num_tl_lines;
		break;

	case POS_TOP_RIGHT:
		y -= num_tr_lines * font_heights[fn];
		++num_tr_lines;
		break;

	case POS_BOTTOM_LEFT:
		y += num_bl_lines * font_heights[fn];
		++num_bl_lines;
		break;

	case POS_BOTTOM_RIGHT:
		y += num_br_lines * font_heights[fn];
		++num_br_lines;
		break;

	case POS_CENTER:
		y -= num_cen_lines * font_heights[fn];
		++num_cen_lines;
		break;
	}

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	glOrtho( 0.0, (double)width, 0.0, (double)height, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	/* Draw text backing */
	glColor3f( INFODISP_TEXT_BACK_R, INFODISP_TEXT_BACK_G, INFODISP_TEXT_BACK_B );
	glRasterPos2i( x + edge_dx, y + edge_dy );
	glListBase( font_dlist_bases[fn] );
	glCallLists( strlen(disp_str), GL_UNSIGNED_BYTE, disp_str );

	/* Draw text face */
	glColor3f( INFODISP_TEXT_FRONT_R, INFODISP_TEXT_FRONT_G, INFODISP_TEXT_FRONT_B );
	glRasterPos2i( x, y );
	glCallLists( strlen(disp_str), GL_UNSIGNED_BYTE, disp_str );

	/* Finally, do next line if disp_str had newlines */
	if (next_disp_str != NULL)
		ogl_draw_string( next_disp_str, pos_code, size );
}


/* Blanks out a viewport, optionally displaying a [centered] message */
void
ogl_blank( int cam_id, const char *blank_message )
{
	camera *cam;

	cam = usr_cams[cam_id];

	gtk_gl_area_make_current( GTK_GL_AREA(cam->ogl_w) );

	glClearColor( background.r, background.g, background.b, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if (blank_message != NULL) {
		/* Tell ogl_draw_string( ) about GL widget dimensions */
		ogl_draw_string( cam, RESET, NIL );

		glDisable( GL_DEPTH_TEST );
		glDisable( GL_LIGHTING );

		ogl_draw_string( blank_message, POS_CENTER, 2 );

		glEnable( GL_LIGHTING );
		glEnable( GL_DEPTH_TEST );
	}

	GTKGL_TEMP_endgl( GTK_GL_AREA(cam->ogl_w) );
	gtk_gl_area_swapbuffers( GTK_GL_AREA(cam->ogl_w) );
}


/* The GL widget begins life here
 * If the primary GL widget (camera 0) has already been defined, create a
 * new one with shared context, to let ogl_draw_string( ) also work in the
 * new window, and simplify display list management a good bit */
GtkWidget *
ogl_make_widget( void )
{
	GtkWidget *primary_ogl_w;
	int gl_area_attributes[] = {
	    GDK_GL_RGBA,
	    GDK_GL_RED_SIZE, 1,
	    GDK_GL_GREEN_SIZE, 1,
	    GDK_GL_BLUE_SIZE, 1,
	    GDK_GL_DEPTH_SIZE, 1,
	    GDK_GL_DOUBLEBUFFER,
	    GDK_GL_NONE
	};

	primary_ogl_w = usr_cams[0]->ogl_w;
	if (primary_ogl_w == NULL)
		return gtk_gl_area_new( gl_area_attributes );
	else
		return gtk_gl_area_share_new( gl_area_attributes, GTK_GL_AREA(primary_ogl_w) );
}

/* end ogl.c */
