/* menu_cbs.c */

/* Callbacks for menus, dialogs */

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

/* Light Speed! title graphic for Help->About */
#include "title.xpm"


/* This services the entry and units button on the menu bar */
void
velocity_input( GtkWidget *widget, const int *message )
{
	static GtkWidget *velocity_entry_w = NULL;
	static GtkWidget *unit_label_w;
	static double prev_input_val = 1.0;
	static int blocking_changed = FALSE;
	GtkWidget *vbox_w;
	GtkWidget *button_w;
	double input_val;
	char *init_str;
	char *input_str;
	char *fixed_str;
	int i;

	switch (*message) {
	case RESET:
		blocking_changed = TRUE;
		/* Clear out any stale text in entry */
		init_str = add_commas( velocity_string( prev_input_val, FALSE ) );
		set_entry_text( velocity_entry_w, init_str );
		blocking_changed = FALSE;
		return;

	case VALUE_COMMITTED: /* <Enter> keystroke received */
		blocking_changed = TRUE;
		input_str = read_entry( velocity_entry_w );
		if (input_str[0] == '>') {
			/* Command entered */
			i = command( &input_str[1] );
			if (i == 0)
				set_entry_text( velocity_entry_w, STR_MSG_Okay );
			else
				set_entry_text( velocity_entry_w, STR_MSG_Invalid );
			blocking_changed = FALSE;
			return;
		}
		/* Clean out junk from input string */
		input_str = clean_number( input_str );
		input_val = strtod( input_str, NULL ) * unit_conv_factors[cur_unit_system];
		input_val = CLAMP(input_val, MIN_VELOCITY, MAX_VELOCITY);
		/* Start hitting the gas (or the brakes!) */
		transition( &velocity, TRUE, TRANS_SIGMOID, 5.0, input_val, -1 );
		init_str = add_commas( velocity_string( input_val, FALSE ) );
		set_entry_text( velocity_entry_w, init_str );
/* TODO: Find out why the highlight_entry( ) here DOES NOT WORK */
		highlight_entry( velocity_entry_w );
		prev_input_val = input_val;
		blocking_changed = FALSE;
		return;

	case VALUE_CHANGED:
		/* Avoid infinite recursion on "changed" signal */
		if (blocking_changed)
			return;
		blocking_changed = TRUE;
		input_str = read_entry( velocity_entry_w );
		if (input_str[0] == '>')
			return; /* command escape */
		/* Clean up numerical input and add commas for clarity */
		fixed_str = add_commas( clean_number( input_str ) );
/* TODO: Find out why gtk_entry_set_position() here doesn't work */
		/* set_entry_text( widget, fixed_str );
		gtk_entry_set_position( GTK_ENTRY(widget), strlen( fixed_str ) ); */
		blocking_changed = FALSE;
		return;

	case UNITS_CHANGED:
		blocking_changed = TRUE;
		cur_unit_system = (cur_unit_system + 1) % num_unit_systems;
		gtk_label_set_text( GTK_LABEL(unit_label_w), unit_suffixes[cur_unit_system] );
		velocity_input( NULL, MESG_(RESET) );
		/* Update info display if it is active */
		info_display( RESET, NIL );
		blocking_changed = FALSE;
		return;

	case INITIALIZE:
		if (velocity_entry_w != NULL) {
			prev_input_val = velocity;
			velocity_input( NULL, MESG_(RESET) );
			return;
		}
		init_str = velocity_string( velocity, FALSE );
		velocity_entry_w = add_entry( widget, init_str, velocity_input, MESG_(VALUE_COMMITTED) );
		set_entry_width( velocity_entry_w, "000,000,000,000" );
		gtk_signal_connect( GTK_OBJECT(velocity_entry_w), "changed", GTK_SIGNAL_FUNC(velocity_input), MESG_(VALUE_CHANGED) );
		keybind( velocity_entry_w, "^V" );
		vbox_w = add_vbox( widget, FALSE, 3 );
		button_w = add_button( vbox_w, NULL, velocity_input, MESG_(UNITS_CHANGED) );
		unit_label_w = add_label( button_w, unit_suffixes[cur_unit_system] );
		keybind( button_w, "U" );
		return;

	default:
#ifdef DEBUG
		crash( "velocity_input( ): invalid_message" );
#endif
		return;
	}
}


/* Services the slider to the side of the viewport */
#if VELOCITY_SLIDER
void
velocity_slider( GtkWidget *widget, const int *message )
{
	static GtkObject *velocity_adj;
	static GtkWidget *velocity_vscale_w;
	static int blocking_changed = FALSE;
	GtkWidget *frame_w;
	double input_val;
	float adj_value;

	switch (*message) {
	case VALUE_CHANGED:
		if (blocking_changed)
			return;
		adj_value = GTK_ADJUSTMENT(velocity_adj)->value;
		input_val = C * (1.0 - adj_value / 65536.0);
		/* Halt any ongoing velocity transition */
		break_transition( &velocity );
		velocity = CLAMP(input_val, MIN_VELOCITY, MAX_VELOCITY);
		/* Update velocity entry */
		velocity_input( NULL, MESG_(INITIALIZE) );
		queue_redraw( -1 );
		return;

	case RESET:
		/* Update slider if its value isn't up-to-date */
		blocking_changed = TRUE;
		adj_value = rint( 65536.0 * (1.0 - velocity / C) );
		if (GTK_ADJUSTMENT(velocity_adj)->value != adj_value)
			gtk_adjustment_set_value( GTK_ADJUSTMENT(velocity_adj), adj_value );
		blocking_changed = FALSE;
		return;

	case INITIALIZE:
		/* Frame for slider (purely for aesthetics) */
		frame_w = add_frame( widget, NULL );
		/* Velocity adjustment and scale widget */
		velocity_adj = gtk_adjustment_new( 65536.0, 0.0, 65536.0, 1.0, 2048.0, 0.0 );
		gtk_signal_connect( GTK_OBJECT(velocity_adj), "value_changed", GTK_SIGNAL_FUNC(velocity_slider), MESG_(VALUE_CHANGED) );
		velocity_vscale_w = gtk_vscale_new( GTK_ADJUSTMENT(velocity_adj) );
		gtk_scale_set_draw_value( GTK_SCALE(velocity_vscale_w), FALSE );
		/* gtk_box_pack_start( GTK_BOX(hbox_w), velocity_scale_w, FALSE, FALSE, 0 ); */
		gtk_container_add( GTK_CONTAINER(frame_w), velocity_vscale_w );
		keybind( velocity_vscale_w, "V" );
		gtk_widget_show( velocity_vscale_w );
		return;

	default:
#ifdef DEBUG
		crash( "velocity_slider( ): invalid message" );
#endif
		return;
	}
}
#endif /* VELOCITY_SLIDER */


void
dialog_File_NewLattice( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *dialog_window_w;
	static GtkObject *size_x_adj, *size_y_adj, *size_z_adj;
	static GtkObject *smoothness_adj;
	static int prev_smoothness = DEF_LATTICE_SMOOTH;
	static float dummy;
	GtkWidget *main_vbox_w;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *vbox_w;
	int input_size_x, input_size_y, input_size_z;
	int input_smoothness;
	int i;

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		gtk_widget_hide( dialog_window_w );
/* TODO: Find out why window widget doesn't actually hide until too late */
		input_size_x = (int)(GTK_ADJUSTMENT(size_x_adj)->value);
		input_size_y = (int)(GTK_ADJUSTMENT(size_y_adj)->value);
		input_size_z = (int)(GTK_ADJUSTMENT(size_z_adj)->value);
		if (advanced_interface)
			input_smoothness = (int)(GTK_ADJUSTMENT(smoothness_adj)->value);
		else
			input_smoothness = prev_smoothness;
		/* Redraw or blank the viewports first (do redraw(s) if it
		 * won't take too long AND a lattice is already up) */
		if ((framerate > ((float)num_cams * 5.0)) && (object_mode == MODE_LATTICE)) {
			for (i = 0; i < num_cams; i++)
				ogl_draw( i );
		}
		else {
			for (i = 0; i < num_cams; i++)
				ogl_blank( i, STR_MSG_Generating_lattice );
		}
		clear_all_objects( );
		make_lattice( input_size_x, input_size_y, input_size_z, input_smoothness );
		/* Reset camera(s) if user geometry had been loaded */
		if (object_mode == MODE_USER_GEOMETRY) {
			for (i = 0; i < num_cams; i++)
				camera_reset( i );
			/* and HIT THE BRAKES! */
			velocity = 1.0;
			velocity_input( NULL, MESG_(INITIALIZE) );
		}
		object_mode = MODE_LATTICE;
		/* Now have to recalibrate framerate */
		profile( PROFILE_FRAMERATE_RESET );
		/* (the transition will take care of redraw(s), too) */
		dummy = 1.0;
		transition( &dummy, FALSE, TRANS_LINEAR, 1.0, 0.0, -1 );
		lattice_size_x = input_size_x;
		lattice_size_y = input_size_y;
		lattice_size_z = input_size_z;
		prev_smoothness = input_smoothness;
		/* no break/return here */

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( dialog_window_w );
		return;

	default:
#ifdef DEBUG
		crash( "dialog_File_NewLattice( ): invalid message" );
#endif
		return;
	}

	/* Dialog window widget */
	dialog_window_w = make_dialog_window( STR_DLG_New_lattice, dialog_File_NewLattice );
	gtk_window_set_position( GTK_WINDOW(dialog_window_w), GTK_WIN_POS_MOUSE );
	/* Main vertical box widget */
	main_vbox_w = add_vbox( dialog_window_w, FALSE, 10 );

	/* Dimensions title frame */
	frame_w = add_frame( main_vbox_w, STR_DLG_Dimensions );
	hbox_w = add_hbox( frame_w, FALSE, 10 );

	/* X spinbutton */
	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "X" );
	size_x_adj = gtk_adjustment_new( (float)lattice_size_x, 1.0, 16.0, 1.0, 1.0, 0.0 );
	add_spin_button( vbox_w, size_x_adj );

	/* Y spinbutton */
	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "Y" );
	size_y_adj = gtk_adjustment_new( (float)lattice_size_y, 1.0, 16.0, 1.0, 1.0, 0.0 );
	add_spin_button( vbox_w, size_y_adj );

	/* Z spinbutton */
	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "Z" );
	size_z_adj = gtk_adjustment_new( (float)lattice_size_z, 1.0, 16.0, 1.0, 1.0, 0.0 );
	add_spin_button( vbox_w, size_z_adj );

	if (advanced_interface) {
		/* Smoothness title frame */
		frame_w = add_frame( main_vbox_w, STR_DLG_Smoothness );
		hbox_w = add_hbox( frame_w, TRUE, 10 );

		/* Smoothness slider */
		smoothness_adj = gtk_adjustment_new( (float)prev_smoothness, 3.0, 16.0, 1.0, 1.0, 0.0 );
		add_hscale( hbox_w, smoothness_adj );
	}

	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );
	add_button( hbox_w, STR_DLG_Okay_btn, dialog_File_NewLattice, MESG_(DIALOG_OK) );
	add_button( hbox_w, STR_DLG_Cancel_btn, dialog_File_NewLattice, MESG_(DIALOG_CLOSE) );

	gtk_widget_show( dialog_window_w );
}


#ifdef WITH_OBJECT_IMPORTER
void
dialog_File_ImportObject( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *filesel_w;
	static char *prev_filename = NULL;
	static float dummy;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *label_w;
	int i;
	char *filename;

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(filesel_w) );
		if (prev_filename != NULL)
			xfree( prev_filename );
		prev_filename = xstrdup( filename );
		gtk_widget_hide( filesel_w );
		/* Blank the viewports for loading */
		for (i = 0; i < num_cams; i++)
			ogl_blank( i, STR_MSG_Importing_object );
		/* Out with the existing objects... */
		clear_all_objects( );
		/* ...and in with the new ones */
		i = import_objects( prev_filename );
		if (i < 0) /* Error! */
			make_lattice( 1, 1, 1, 3 );
		/* A full view reset is (almost) certainly necessary */
		for (i = 0; i < num_cams; i++)
			camera_reset( i );
		/* Reset velocity too */
		velocity = 1.0;
		velocity_input( NULL, MESG_(INITIALIZE) );
		object_mode = MODE_USER_GEOMETRY;
		/* Reset and recalibrate framerate */
		profile( PROFILE_FRAMERATE_RESET );
		dummy = 1.0;
		transition( &dummy, FALSE, TRANS_LINEAR, 1.0, 0.0, -1 );
		/* no break/return here */

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( filesel_w );
		return;

	default:
#ifdef DEBUG
		crash( "dialog_File_ImportObject( ): invalid message" );
#endif
		return;
	}

	filesel_w = make_filesel_window( STR_DLG_Load_Object, prev_filename, FALSE, dialog_File_ImportObject );

	frame_w = add_frame( GTK_FILE_SELECTION(filesel_w)->main_vbox, STR_DLG_Recognized_formats );
	hbox_w = add_hbox( frame_w, FALSE, 10 );
	label_w = add_label( hbox_w, STR_DLG_3d_formats );
	gtk_label_set_justify( GTK_LABEL(label_w), GTK_JUSTIFY_LEFT );

	gtk_widget_show( filesel_w );
}
#endif /* WITH_OBJECT_IMPORTER */


#ifdef CAN_SAVE_SNAPSHOT
void
dialog_File_SaveSnapshot( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *filesel_w;
	static GtkWidget *size_entry_w;
	static int format = -1;
	static int width, height;
	static char *prev_filename = NULL;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *vbox_w;
	int f, f1, f2;
	char *filename;
	char *input_str;
	char init_str[256];

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		/* Grab and save the specified filename */
		filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(filesel_w) );
		xfree( prev_filename );
		prev_filename = xstrdup( filename );
		/* Get snapshot dimensions */
		input_str = read_entry( size_entry_w );
		width = (int)strtod( input_str, NULL );
		/* next part (height) */
		input_str = strpbrk( input_str, "xX* " );
		if (input_str != NULL)
			input_str = strpbrk( input_str, "0123456789" );
		if (input_str != NULL)
			height = (int)strtod( input_str, NULL );
		else
			height = (3 * width) / 4;
		if ((width < 100) || (height < 75)) {
			/* Dimensions are too small (or negative??) */
			width = MAX(100, width);
			height = MAX(75, height);
			sprintf( init_str, "%d x %d", width, height );
			set_entry_text( size_entry_w, init_str );
			return;
		}
		gtk_widget_hide( filesel_w );
	case DIALOG_OK_CONFIRM:
		if (file_exists( prev_filename ) && (*message != DIALOG_OK_CONFIRM)) {
			filename = file_basename( prev_filename, NULL );
			sprintf( init_str, STR_MSG_overwrite_warn_ARG, filename );
			confirmation_dialog( STR_DLG_Warning, init_str, dialog_File_SaveSnapshot );
			return;
		}
		else {
			ogl_blank( 0, NULL );
			save_snapshot( width, height, prev_filename, format );
			ogl_draw( 0 );
		}
		/* no break/return here */

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( filesel_w );
		return;

	case IMAGE_FORMAT_PNG:
	case IMAGE_FORMAT_TIFF:
		/* Note newly selected format and update filename extension
		 * (the latter only if it matches previously selected format) */
		f1 = format - IMAGE_FORMAT - 1;
		format = *message;
		f2 = format - IMAGE_FORMAT - 1;
		filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(filesel_w) );
		filename = swap_filename_ext( filename, image_format_exts[f1], image_format_exts[f2] );
		gtk_file_selection_set_filename( GTK_FILE_SELECTION(filesel_w), filename );
		return;

	default:
#ifdef DEBUG
		crash( "dialog_File_SaveSnapshot( ): invalid message" );
#endif
		return;
	}

#ifdef HAVE_LIBPNG
	if (format == -1)
		format = IMAGE_FORMAT_PNG;
#endif
#ifdef HAVE_LIBTIFF
	if (format == -1)
		format = IMAGE_FORMAT_TIFF;
#endif
	/* Set defaults */
	if (prev_filename == NULL) {
		width = usr_cams[0]->ogl_w->allocation.width;
		height = usr_cams[0]->ogl_w->allocation.height;
		f = format - IMAGE_FORMAT - 1;
		sprintf( init_str, "%s%s", STR_DLG_snapshot_basename, image_format_exts[f] );
		prev_filename = xstrdup( init_str );
	}
	filesel_w = make_filesel_window( STR_DLG_Save_Snapshot, prev_filename, TRUE, dialog_File_SaveSnapshot );

	/* Frame for image parameters */
	frame_w = add_frame( GTK_FILE_SELECTION(filesel_w)->main_vbox, STR_DLG_snapshot_Parameters );

	/* hbox for image size/format inputs */
	hbox_w = add_hbox( frame_w, FALSE, 10 );

	/* Snapshot size label & entry */
	vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox_w );
	add_label( vbox_w, STR_DLG_snapshot_Size );
	sprintf( init_str, "%d x %d", width, height );
	size_entry_w = add_entry( vbox_w, init_str, NULL, NULL );

	/* Image format option menu */
	vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox_w );
	add_label( vbox_w, STR_DLG_snapshot_Format );
#ifdef HAVE_LIBPNG
	option_menu_item( "PNG", dialog_File_SaveSnapshot, MESG_(IMAGE_FORMAT_PNG) );
#endif
#ifdef HAVE_LIBTIFF
	option_menu_item( "TIFF", dialog_File_SaveSnapshot, MESG_(IMAGE_FORMAT_TIFF) );
#endif
	add_option_menu( vbox_w );

	gtk_widget_show( filesel_w );
}
#endif /* CAN_SAVE_SNAPSHOT */


#ifdef WITH_SRS_EXPORTER
void
dialog_File_ExportSRS( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *filesel_w;
	static GtkWidget *size_entry_w;
	static GtkWidget *stereo_chkbtn_w;
	static GtkWidget *visfaceonly_chkbtn_w;
	static int width, height;
	static int stereo_view = FALSE;
	static int visible_faces_only = TRUE;
	static char *prev_filename = NULL;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *vbox_w;
	char *filename;
	char *input_str;
	char init_str[256];

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		/* Grab and save the specified filename */
		filename = gtk_file_selection_get_filename( GTK_FILE_SELECTION(filesel_w) );
		xfree( prev_filename );
		prev_filename = xstrdup( filename );
		/* Get rendering dimensions */
		input_str = read_entry( size_entry_w );
		width = (int)strtod( input_str, NULL );
		/* next part (height) */
		input_str = strpbrk( input_str, "xX* " );
		if (input_str != NULL)
			input_str = strpbrk( input_str, "0123456789" );
		if (input_str != NULL)
			height = (int)strtod( input_str, NULL );
		else
			height = (3 * width) / 4;
		if ((width < 100) || (height < 75)) {
			/* Dimensions are too small (or negative??) */
			width = MAX(100, width);
			height = MAX(75, height);
			sprintf( init_str, "%d x %d", width, height );
			set_entry_text( size_entry_w, init_str );
			return;
		}
		stereo_view = GTK_TOGGLE_BUTTON(stereo_chkbtn_w)->active;
		visible_faces_only = GTK_TOGGLE_BUTTON(visfaceonly_chkbtn_w)->active;
		gtk_widget_hide( filesel_w );
	case DIALOG_OK_CONFIRM:
		if (file_exists( prev_filename ) && (*message != DIALOG_OK_CONFIRM)) {
			filename = file_basename( prev_filename, NULL );
			sprintf( init_str, STR_MSG_overwrite_warn_ARG, filename );
			confirmation_dialog( STR_DLG_Warning, init_str, dialog_File_ExportSRS );
			return;
		}
		else {
			ogl_blank( 0, NULL );
			export_srs( prev_filename, width, height, stereo_view, visible_faces_only );
			ogl_draw( 0 );
		}
		/* no break/return here */

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( filesel_w );
		return;

	default:
#ifdef DEBUG
		crash( "dialog_File_ExportSRS( ): invalid message" );
#endif
		return;
	}

	if (prev_filename == NULL) {
		/* Set defaults */
		width = usr_cams[0]->ogl_w->allocation.width;
		height = usr_cams[0]->ogl_w->allocation.height;
		if (object_mode == MODE_LATTICE)
			sprintf( init_str, "%s.srs", STR_DLG_srs_basename1 );
		else
			sprintf( init_str, "%s.srs", STR_DLG_srs_basename2 );
		prev_filename = xstrdup( init_str );
	}
	filesel_w = make_filesel_window( STR_DLG_Export_srs, prev_filename, TRUE, dialog_File_ExportSRS );

	/* Label and frame for SRS parameters */
	add_label( GTK_FILE_SELECTION(filesel_w)->main_vbox, STR_DLG_srs );
	frame_w = add_frame( GTK_FILE_SELECTION(filesel_w)->main_vbox, STR_DLG_srs_Parameters );

	/* hbox for image size / misc. parameter input */
	hbox_w = add_hbox( frame_w, FALSE, 10 );

	/* Rendering size label & entry */
	vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox_w );
	add_label( vbox_w, STR_DLG_srs_Size );
	sprintf( init_str, "%d x %d", width, height );
	size_entry_w = add_entry( vbox_w, init_str, NULL, NULL );

	/* Misc. parameter check buttons */
	vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox_w );
	stereo_chkbtn_w = add_check_button( vbox_w, STR_DLG_srs_Stereo_view, stereo_view, NULL, NULL );
	visfaceonly_chkbtn_w = add_check_button( vbox_w, STR_DLG_srs_Vis_faces_only, visible_faces_only, NULL, NULL );
	if (object_mode != MODE_USER_GEOMETRY)
		gtk_widget_hide( visfaceonly_chkbtn_w );

	gtk_widget_show( filesel_w );
}
#endif /* WITH_SRS_EXPORTER */


void
menu_Objects_toggles( GtkWidget *widget, const int *message )
{
	int flag;

	flag = GTK_CHECK_MENU_ITEM(widget)->active;
	auxiliary_objects( *message, flag );
}


void
dialog_Objects_Animation( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *dialog_window_w;
	static GtkWidget *x0_entry_w;
	static GtkWidget *x1_entry_w;
	static GtkWidget *action_label_w;
	static GtkObject *loop_time_adj;
	static float prev_recomm_x0 = 999.0;
	static float prev_recomm_x1 = -999.0;
	static float prev_x0;
	static float prev_x1;
	static float prev_loop_time = 10.0;
	static int in_motion = FALSE;
	GtkWidget *main_vbox_w;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *hbox2_w;
	GtkWidget *vbox_w;
	GtkWidget *button_w;
	float recomm_x0;
	float recomm_x1;
	float input_x0;
	float input_x1;
	float input_loop_time;
	char *x_span_str = "0000000000";
	char *input_str;
	char init_str[16];

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( dialog_window_w );
		return;

	case DIALOG_OK:
		in_motion = !in_motion;
		if (in_motion) {
			/* Begin animation */
			if (advanced_interface) {
				input_str = read_entry( x0_entry_w );
				input_x0 = strtod( input_str, NULL );
				input_str = read_entry( x1_entry_w );
				input_x1 = strtod( input_str, NULL );
			}
			else {
				input_x0 = prev_x0;
				input_x1 = prev_x1;
			}
			input_loop_time = GTK_ADJUSTMENT(loop_time_adj)->value;
			if (input_x0 > input_x1)
				return;
			if ((input_x1 - input_x0) < 0.5)
				return;
			warp_time( input_x0, input_x1, input_loop_time, WARP_BEGIN_ANIM );
			gtk_label_set_text( GTK_LABEL(action_label_w), STR_DLG_Stop_btn );
			prev_x0 = input_x0;
			prev_x1 = input_x1;
			prev_loop_time = input_loop_time;
		}
		else {
			/* Stop animation */
			warp_time( NIL, NIL, NIL, WARP_STOP_ANIM );
			gtk_label_set_text( GTK_LABEL(action_label_w), STR_DLG_Begin_btn );
		}
		return;

	default:
#ifdef DEBUG
		crash( "dialog_Objects_Animation( ): invalid message" );
#endif
		return;
	}

	/* Recommended values for x0 & x1
	 * All this prev_/recomm_/prev_recomm_ jockeying is to show recommended
	 * values ONLY when they change, otherwise use previous user values */
	recomm_x0 = world_extents.xmin - vehicle_extents.xmax;
	recomm_x1 = world_extents.xmax - vehicle_extents.xmin;
	if (recomm_x0 != prev_recomm_x0) {
		prev_x0 = recomm_x0;
		prev_x1 = recomm_x1;
	}
	prev_recomm_x0 = recomm_x0;
	prev_recomm_x1 = recomm_x1;

	/* Dialog window widget */
	dialog_window_w = make_dialog_window( STR_DLG_Animation, dialog_Objects_Animation );
	gtk_window_set_position( GTK_WINDOW(dialog_window_w), GTK_WIN_POS_MOUSE );

	/* Main vertical box widget */
	main_vbox_w = add_vbox( dialog_window_w, FALSE, 10 );

	if (advanced_interface) {
		/* Title frame widget for range of motion entries */
		frame_w = add_frame( main_vbox_w, STR_DLG_Observed_range );
		hbox_w = add_hbox( frame_w, FALSE, 10 );

		/* x0 entry */
		vbox_w = add_vbox( hbox_w, FALSE, 0 );
		add_label( vbox_w, STR_DLG_Start_X );
		sprintf( init_str, "%.2f", prev_x0 );
		x0_entry_w = add_entry( vbox_w, init_str, NULL, NULL );
		set_entry_width( x0_entry_w, x_span_str );

		/* x1 entry */
		vbox_w = add_vbox( hbox_w, FALSE, 0 );
		add_label( vbox_w, STR_DLG_End_X );
		sprintf( init_str, "%.2f", prev_x1 );
		x1_entry_w = add_entry( vbox_w, init_str, NULL, NULL );
		set_entry_width( x1_entry_w, x_span_str );

		hbox_w = add_hbox( main_vbox_w, FALSE, 0 );
	}
	else {
		frame_w = add_frame( main_vbox_w, NULL );
		hbox_w = add_hbox( frame_w, FALSE, 10 );
	}

	hbox2_w = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), hbox2_w, TRUE, FALSE, 0 );
	gtk_widget_show( hbox2_w );

	/* Loop time spin button */
	add_label( hbox2_w, STR_DLG_Loop_time );
	loop_time_adj = gtk_adjustment_new( prev_loop_time, 1, 60, 1, 1, 0.0 );
	add_spin_button( hbox2_w, loop_time_adj );
	add_label( hbox2_w, STR_DLG_seconds );

	/* Begin/Stop and Close buttons */
	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );
	button_w = add_button( hbox_w, NULL, dialog_Objects_Animation, MESG_(DIALOG_OK) );
	action_label_w = add_label( button_w, STR_DLG_Begin_btn );
	if (in_motion)
		gtk_label_set_text( GTK_LABEL(action_label_w), STR_DLG_Stop_btn );
	add_button( hbox_w, STR_DLG_Close_btn, dialog_Objects_Animation, MESG_(DIALOG_CLOSE) );

	gtk_widget_show( dialog_window_w );
}


void
menu_Warp_toggles( GtkWidget *widget, const int *message )
{
	if (GTK_CHECK_MENU_ITEM(widget)->active)
		warp( *message, MESG_(1) );
	else
		warp( *message, MESG_(0) );
}


/* Camera / Lens / (lens length) */
void
menu_Camera_Lens_select( GtkWidget *widget, const float *new_lens_length )
{
	float new_fov;

	new_fov = camera_calc_fov( *new_lens_length );
	if (GTK_CHECK_MENU_ITEM(widget)->active)
		transition( &usr_cams[cur_cam]->fov, FALSE, TRANS_QTR_SIN, 3.0, new_fov, cur_cam );
}


/* Camera / Lens / Custom */
void
dialog_Camera_Lens_Custom( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *dialog_window_w;
	static GtkWidget *entry_w;
	static GtkWidget *frame_w;
	static GtkWidget *button_label_w;
	static camera *cam;
	static int fov_mode = FALSE;
	GtkWidget *main_vbox_w;
	GtkWidget *hbox_w;
	GtkWidget *vbox_w;
	GtkWidget *button_w;
	float input_val;
	float init_val;
	char *input_str;
	char init_str[16];

	switch (*message) {
	case DIALOG_OPEN:
		if (active || !GTK_CHECK_MENU_ITEM(widget)->active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		input_str = read_entry( entry_w );
		input_val = strtod( input_str, NULL );
		if (!fov_mode)
			input_val = camera_calc_fov( input_val );
		if ((input_val < 1.0) || (input_val > 175.0)) {
			set_entry_text( entry_w, STR_MSG_Invalid );
			highlight_entry( entry_w );
			return;
		}
		else
			transition( &cam->fov, FALSE, TRANS_SIGMOID, 3.0, input_val, cur_cam );
		/* no break/return here */

	case DIALOG_CLOSE:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( dialog_window_w );
		return;

	case INITIALIZE:

	case UNITS_CHANGED:
		fov_mode = !fov_mode;
		if (fov_mode) {
			gtk_frame_set_label( GTK_FRAME(frame_w), STR_DLG_Field_of_view );
			gtk_label_set_text( GTK_LABEL(button_label_w), STR_DLG_degree_suffix );
			sprintf( init_str, "%.2f", cam->fov );
			set_entry_text( entry_w, init_str );
		}
		else {
			gtk_frame_set_label( GTK_FRAME(frame_w), STR_DLG_Lens_length );
			gtk_label_set_text( GTK_LABEL(button_label_w), "mm" );
			init_val = camera_calc_lens_length( cam->fov );
			sprintf( init_str, "%.2f", init_val );
			set_entry_text( entry_w, init_str );
		}
		return;

	default:
#ifdef DEBUG
		crash( "dialog_Lens_Custom( ): invalid message" );
#endif
		return;
	}

	cam = usr_cams[cur_cam];

	/* Dialog window widget */
	dialog_window_w = make_dialog_window( STR_DLG_Custom_Lens, dialog_Camera_Lens_Custom );
	gtk_window_set_position( GTK_WINDOW(dialog_window_w), GTK_WIN_POS_MOUSE );

	/* Main vertical box widget */
	main_vbox_w = add_vbox( dialog_window_w, FALSE, 10 );

	/* Title frame widget */
	frame_w = add_frame( main_vbox_w, "---" );

	/* Vertical box for spacing purposes only */
	vbox_w = add_vbox( frame_w, FALSE, 10 );

	/* Horizontal box for entry widget, units button */
	hbox_w = add_hbox( vbox_w, FALSE, 0 );

	/* Entry widget for lens length/FOV value */
	entry_w = add_entry( hbox_w, "---", dialog_Camera_Lens_Custom, MESG_(DIALOG_OK) );
	set_entry_width( entry_w, "00000000" );

	/* Vertical box to frane the button */
	vbox_w = add_vbox( hbox_w, FALSE, 3 );

	/* Units select button */
	button_w = add_button( vbox_w, NULL, dialog_Camera_Lens_Custom, MESG_(UNITS_CHANGED) );

	/* Label for units select button */
	button_label_w = add_label( button_w, "---" );

	/* Horizontal box widget for OK/Close */
	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );

	/* OK button */
	add_button( hbox_w, STR_DLG_Okay_btn, dialog_Camera_Lens_Custom, MESG_(DIALOG_OK) );

	/* "Cancel" button */
	add_button( hbox_w, STR_DLG_Cancel_btn, dialog_Camera_Lens_Custom, MESG_(DIALOG_CLOSE) );

	/* Call ourselves to set all the "---"'s to something sensible */
	fov_mode = !fov_mode; /* following call will flip this back */
	dialog_Camera_Lens_Custom( NULL, MESG_(INITIALIZE) );

	gtk_widget_show( dialog_window_w );
}


/* Camera / Position dialog
 * NOTE: phi & theta as seen by the user are NOT those in struct cam!
 * cam->phi = user_phi + 180, cam->theta = - user_theta */
void
dialog_Camera_Position( GtkWidget *widget, const int *message )
{
	static int active = FALSE;
	static GtkWidget *dialog_window_w;
	static GtkWidget *loc_x_entry_w;
	static GtkWidget *loc_y_entry_w;
	static GtkWidget *loc_z_entry_w;
	static GtkWidget *targ_frame_w;
	static GtkWidget *targ_1_entry_w;
	static GtkWidget *targ_2_entry_w;
	static GtkWidget *targ_z_entry_w;
	static GtkWidget *unit_label_w;
	static GtkWidget *targ_1_label_w;
	static GtkWidget *targ_2_label_w;
	static GtkWidget *targ_z_vbox_w;
	static GtkWidget *action_label_w;
	static camera *cam;
	static int cam_id;
	static int phi_theta_mode = FALSE;
	static int input_queued;
	static int blocking_changed = FALSE;
	GtkWidget *main_vbox_w;
	GtkWidget *frame_w;
	GtkWidget *hbox_w;
	GtkWidget *vbox_w;
	GtkWidget *vbox2_w;
	GtkWidget *button_w;
	camera input_cam;
	float dx, dy, dz, dxy;
	char *xyz_span_str = "0000000000";
	char *phi_theta_span_str = "00000000000000";
	char init_str[16];
	char *input_str;

	switch (*message) {
	case DIALOG_OPEN:
		if (active)
			return;
		active = TRUE;
		break;

	case DIALOG_OK:
		/* If user didn't enter new values, don't do anything, just close */
		if (!input_queued)
			goto cam_pos_close;
		/* Suck in all the input text */
		input_str = read_entry( loc_x_entry_w );
		input_cam.pos.x = strtod( input_str, NULL );
		input_str = read_entry( loc_y_entry_w );
		input_cam.pos.y = strtod( input_str, NULL );
		input_str = read_entry( loc_z_entry_w );
		input_cam.pos.z = strtod( input_str, NULL );
		if (phi_theta_mode) {
			input_str = read_entry( targ_1_entry_w );
			input_cam.phi = fmod( strtod( input_str, NULL ) + 180.0, 360.0 );
			input_str = read_entry( targ_2_entry_w );
			input_cam.theta = - fmod( strtod( input_str, NULL ), 90.0 );
			/* Grok a better target and distance */
			camera_make_target( &input_cam );
		}
		else {
			input_str = read_entry( targ_1_entry_w );
			input_cam.target.x = strtod( input_str, NULL );
			input_str = read_entry( targ_2_entry_w );
			input_cam.target.y = strtod( input_str, NULL );
			input_str = read_entry( targ_z_entry_w );
			input_cam.target.z = strtod( input_str, NULL );
			/* Need to get phi+theta+distance from XYZpos/XYZtarget */
			dx = input_cam.target.x - input_cam.pos.x;
			dy = input_cam.target.y - input_cam.pos.y;
			dz = input_cam.target.z - input_cam.pos.z;
			dxy = sqrt( SQR(dx) + SQR(dy) );
			input_cam.phi = fmod( DEG(atan2( dy, dx )) + 180.0, 360.0 );
			input_cam.theta = DEG(atan2( - dz, dxy ));
			input_cam.distance = sqrt( SQR(dx) + SQR(dy) + SQR(dz) );
			if (input_cam.distance < 0.01) /* not good */
				return;
		}

		if (ABS(input_cam.phi - cam->phi) > 180.0) {
			/* e.g. don't do a 300deg turn, -60deg will do */
			if (cam->phi > input_cam.phi)
				cam->phi -= 360.0;
			else
				cam->phi += 360.0;
		}

		/* And finally, get the camera moving! */
		transition( &cam->phi, FALSE, TRANS_SIGMOID, 4.0, input_cam.phi, cam_id );
		transition( &cam->theta, FALSE, TRANS_SIGMOID, 4.0, input_cam.theta, cam_id );
		transition( &cam->distance, FALSE, TRANS_SIGMOID, 4.0, input_cam.distance, cam_id );
		transition( &cam->target.x, FALSE, TRANS_SIGMOID, 4.0, input_cam.target.x, cam_id );
		transition( &cam->target.y, FALSE, TRANS_SIGMOID, 4.0, input_cam.target.y, cam_id );
		transition( &cam->target.z, FALSE, TRANS_SIGMOID, 4.0, input_cam.target.z, cam_id );
		/* no break/return here */

	case DIALOG_CLOSE:
cam_pos_close:
		if (!active)
			return;
		active = FALSE;
		gtk_widget_destroy( dialog_window_w );
		return;

	case INITIALIZE:
		input_queued = TRUE; /* will be set FALSE shortly */

	case UNITS_CHANGED:
		/* Switch between XYZ target and phi/theta specifications */
		phi_theta_mode = !phi_theta_mode;
		if (phi_theta_mode) {
			/* Set the labels for XYZ target mode */
			gtk_label_set_text( GTK_LABEL(unit_label_w), STR_DLG_Xyz_instead );
			gtk_label_set_text( GTK_LABEL(targ_1_label_w), STR_DLG_Phi_label );
			gtk_label_set_text( GTK_LABEL(targ_2_label_w), STR_DLG_Theta_label );
			gtk_widget_hide( targ_z_vbox_w );
			gtk_frame_set_label( GTK_FRAME(targ_frame_w), STR_DLG_Direction );
		}
		else {
			/* Set the labels for phi/theta mode */
			gtk_label_set_text( GTK_LABEL(unit_label_w), STR_DLG_Angles_instead );
			gtk_label_set_text( GTK_LABEL(targ_1_label_w), "X" );
			gtk_label_set_text( GTK_LABEL(targ_2_label_w), "Y" );
			gtk_widget_show( targ_z_vbox_w );
			gtk_frame_set_label( GTK_FRAME(targ_frame_w), STR_DLG_View_target );
		}
		/* no break/return here */

	case RESET:
		if (!active)
			return;
		/* Do this because set_entry_text( ) emits "changed" signals */
		blocking_changed = TRUE;
		/* Update stale text in entry widgets */
		if (*message != UNITS_CHANGED) {
			/* Update location entries
			 * (but not on phi/theta mode switch) */
			sprintf( init_str, "%.3f", cam->pos.x );
			set_entry_text( loc_x_entry_w, init_str );
			sprintf( init_str, "%.3f", cam->pos.y );
			set_entry_text( loc_y_entry_w, init_str );
			sprintf( init_str, "%.3f", cam->pos.z );
			set_entry_text( loc_z_entry_w, init_str );
			/* Also note: entries do not contain user input (anymore) */
			if (input_queued)
				gtk_label_set_text( GTK_LABEL(action_label_w), STR_DLG_Okay_btn );
			input_queued = FALSE;
		}
		if (phi_theta_mode) {
			/* Update phi+theta/target entries */
			sprintf( init_str, "%.2f", fmod( cam->phi + 180.0, 360.0 ) );
			set_entry_text( targ_1_entry_w, init_str );
			set_entry_width( targ_1_entry_w, phi_theta_span_str );
			sprintf( init_str, "%.2f", - cam->theta );
			set_entry_text( targ_2_entry_w, init_str );
			set_entry_width( targ_2_entry_w, phi_theta_span_str );
		}
		else {
			/* Update XYZ target entries */
			sprintf( init_str, "%.3f", cam->target.x );
			set_entry_text( targ_1_entry_w, init_str );
			set_entry_width( targ_1_entry_w, xyz_span_str );
			sprintf( init_str, "%.3f", cam->target.y );
			set_entry_text( targ_2_entry_w, init_str );
			set_entry_width( targ_2_entry_w, xyz_span_str );
			sprintf( init_str, "%.3f", cam->target.z );
			set_entry_text( targ_z_entry_w, init_str );
			set_entry_width( targ_z_entry_w, xyz_span_str );
		}
		blocking_changed = FALSE;
		return;

	case VALUE_CHANGED:
		/* Don't catch signals from the set_entry_text( )'s above */
		if (blocking_changed)
			return;
		/* Entries now contain user input */
		if (!input_queued)
			gtk_label_set_text( GTK_LABEL(action_label_w), STR_DLG_Reposition_btn );
		input_queued = TRUE;
		return;

	default:
#ifdef DEBUG
		crash( "dialog_Camera_Position( ): invalid message" );
#endif
		return;
	}

	cam_id = cur_cam;
	cam = usr_cams[cam_id];

	/* New dialog window */
	dialog_window_w = make_dialog_window( STR_DLG_Camera_Position, dialog_Camera_Position );
	gtk_window_set_position( GTK_WINDOW(dialog_window_w), GTK_WIN_POS_MOUSE );

	/* Main vertical box widget */
	main_vbox_w = add_vbox( dialog_window_w, FALSE, 10 );

	/* Title frame for location entries */
	frame_w = add_frame( main_vbox_w, STR_DLG_Location );

	/* Horizontal box widget for location entries */
	hbox_w = add_hbox( frame_w, TRUE, 10 );

	/* The XYZ location entries */

	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "X" );
	loc_x_entry_w = add_entry( vbox_w, "---", NULL, NULL );
	set_entry_width( loc_x_entry_w, xyz_span_str );
	gtk_signal_connect( GTK_OBJECT(loc_x_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "Y" );
	loc_y_entry_w = add_entry( vbox_w, "---", NULL, NULL );
	set_entry_width( loc_y_entry_w, xyz_span_str );
	gtk_signal_connect( GTK_OBJECT(loc_y_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	vbox_w = add_vbox( hbox_w, FALSE, 0 );
	add_label( vbox_w, "Z" );
	loc_z_entry_w = add_entry( vbox_w, "---", NULL, NULL );
	set_entry_width( loc_z_entry_w, xyz_span_str );
	gtk_signal_connect( GTK_OBJECT(loc_z_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	/* Title frame for target entries */
	targ_frame_w = add_frame( main_vbox_w, "---" );

	/* Vertical box widget for target entries + entry mode button */
	vbox_w = add_vbox( targ_frame_w, FALSE, 10 );

	/* Horizontal box widget for target entries (custom) */
	hbox_w = add_hbox( vbox_w, TRUE, 0 );

	/* The XYZ-target/phi+theta-heading entries */

	/* X or phi */
	vbox2_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox2_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox2_w );
	targ_1_label_w = add_label( vbox2_w, "---" );
	targ_1_entry_w = add_entry( vbox2_w, "---", NULL, NULL );
	gtk_signal_connect( GTK_OBJECT(targ_1_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	/* Y or theta */
	vbox2_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), vbox2_w, TRUE, FALSE, 0 );
	gtk_widget_show( vbox2_w );
	targ_2_label_w = add_label( vbox2_w, "---" );
	targ_2_entry_w = add_entry( vbox2_w, "---", NULL, NULL );
	gtk_signal_connect( GTK_OBJECT(targ_2_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	/* Z */
	targ_z_vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(hbox_w), targ_z_vbox_w, TRUE, FALSE, 0 );
	gtk_widget_show( targ_z_vbox_w );
	add_label( targ_z_vbox_w, "Z" );
	targ_z_entry_w = add_entry( targ_z_vbox_w, "---", NULL, NULL );
	gtk_signal_connect( GTK_OBJECT(targ_z_entry_w), "changed",
	                    GTK_SIGNAL_FUNC(dialog_Camera_Position), MESG_(VALUE_CHANGED) );

	/* Entry mode button */
	button_w = add_button( vbox_w, NULL, dialog_Camera_Position, MESG_(UNITS_CHANGED) );
	unit_label_w = add_label( button_w, "---" );

	/* Horizontal box widget for OK/Cancel */
	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );

	/* OK/Cancel buttons */
	button_w = add_button( hbox_w, NULL, dialog_Camera_Position, MESG_(DIALOG_OK) );
	action_label_w = add_label( button_w, "---" );
	add_button( hbox_w, STR_DLG_Cancel_btn, dialog_Camera_Position, MESG_(DIALOG_CLOSE) );

	/* Call ourselves to set all the "---"'s to something sensible */
	phi_theta_mode = !phi_theta_mode; /* the next call will flip it back */
	dialog_Camera_Position( NULL, MESG_(INITIALIZE) );

	gtk_widget_show( dialog_window_w );
}


void
menu_Camera_ResetView( GtkWidget *widget, const int *cam_id )
{
	camera_reset( *cam_id );
}


void
menu_Camera_InfoDisplay_toggles( GtkWidget *widget, const int *message )
{
	int flag;

	flag = GTK_CHECK_MENU_ITEM(widget)->active;
	info_display( *message, flag );
}


/* Set a new background color */
void
menu_Camera_Background_select( GtkWidget *widget, const int *color_id )
{
	float new_r, new_g, new_b;

	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;

#ifdef DEBUG
	if (*color_id >= num_background_colors)
		crash( "menu_Camera_Background_select( ): invalid color" );
#endif

	new_r = bkgd_colors[*color_id].r;
	new_g = bkgd_colors[*color_id].g;
	new_b = bkgd_colors[*color_id].b;

	transition( &background.r, FALSE, TRANS_QTR_SIN, 2.0, new_r, -1 );
	transition( &background.g, FALSE, TRANS_QTR_SIN, 2.0, new_g, -1 );
	transition( &background.b, FALSE, TRANS_QTR_SIN, 2.0, new_b, -1 );
}


/* Camera / Graphics mode
 * Switches between OpenGL's shaded and wireframe modes */
void
menu_Camera_GraphicsMode_select( GtkWidget *widget, const int *message )
{
	int i;

	/* "toggled" signal also catches "toggle off" */
	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;

	for (i = 0; i < num_cams; i++) {
		gtk_gl_area_make_current( GTK_GL_AREA(usr_cams[i]->ogl_w) );
		switch (*message) {
		case OGL_WIREFRAME_MODE:
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			break;

		case OGL_SHADED_MODE:
			glPolygonMode( GL_FRONT, GL_FILL );
			break;

		default:
#ifdef DEBUG
			crash( "menu_Camera_GraphicsMode( ): invalid message" );
#endif
			break;
		}
		//GTKGL_TEMP_endgl( GTK_GL_AREA(usr_cams[i]->ogl_w) );
	}

	/* Redraw everything */
	queue_redraw( -1 );
}


/* Camera / Spawn camera
 * Creates a new window, with a new, autonomous camera view */
void
menu_Camera_Spawn( GtkWidget *widget, void *dummy )
{
	GtkWidget *cam_window_w;
	GtkWidget *main_vbox_w;
	GtkWidget *menu_bar_w;
	camera *new_cam;

	/* Create the camera */
	new_cam = new_camera( );

	/* New window widget */
	cam_window_w = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW(cam_window_w), STR_DLG_Camera );
	gtk_widget_set_usize( cam_window_w, 400, 300 );
	gtk_container_set_border_width( GTK_CONTAINER(cam_window_w), 0 );
	gtk_signal_connect( GTK_OBJECT(cam_window_w), "focus_in_event",
	                    GTK_SIGNAL_FUNC(camera_set_current), NULL );
	gtk_signal_connect( GTK_OBJECT(cam_window_w), "delete_event",
	                    GTK_SIGNAL_FUNC(menu_Camera_Close), NULL );
	/* Destroy window before exiting */
	gtk_quit_add_destroy( 1, GTK_OBJECT(cam_window_w) );

	/* Main vertical box widget */
	main_vbox_w = gtk_vbox_new( FALSE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER(main_vbox_w), 0 );
	gtk_container_add( GTK_CONTAINER(cam_window_w), main_vbox_w );
	gtk_widget_show( main_vbox_w );

	/* Initialize keybinding */
	keybind( NULL, NULL );

	/* Build minimal menu bar */
	menu_bar_w = gtk_menu_bar_new( );
	gtk_box_pack_start( GTK_BOX(main_vbox_w), menu_bar_w, FALSE, FALSE, 0 );
	gtk_widget_show( menu_bar_w );

	/* NOTE: add_Camera_menu( ) uses assoc_cam_id( ) */
	new_cam->window_w = cam_window_w;

	/* Add a stripped-down Camera menu */
	add_Camera_menu( menu_bar_w, cam_window_w );

	/* Add viewport */
	new_cam->ogl_w = add_gl_area( main_vbox_w );

	/* Attach keybindings */
	keybind( cam_window_w, NULL );

	gtk_widget_show( cam_window_w ); /* Ready for action! */
}


/* Camera / Close (appears only in spawned windows)
 * Gets rid of a spawned camera */
void
menu_Camera_Close( GtkWidget *widget_to_kill, void *dummy )
{
	int cam_id;

	cam_id = assoc_cam_id( widget_to_kill );
	kill_camera( cam_id );
}


/* Help / Overview
 * Tell the user what's going on */
void
dialog_Help_Overview( GtkWidget *widget, int *message )
{
	GtkWidget *parent_window;
	static int active = FALSE;
	static GtkWidget *help_window_w;
	GtkWidget *content_area;
	GtkWidget *main_vbox_w;
	GtkWidget *frame_w;
	GtkWidget *help_label_w;
	GtkWidget *hbox_w;

	parent_window = gtk_widget_get_toplevel( widget );
	help_window_w = gtk_dialog_new_with_buttons(STR_DLG_Overview,
                             GTK_WINDOW(parent_window),
                             GTK_DIALOG_MODAL,
                             GTK_STOCK_CLOSE
                             );

	gtk_window_set_position( GTK_WINDOW(help_window_w), GTK_WIN_POS_CENTER );

	content_area = gtk_dialog_get_content_area ( GTK_DIALOG(help_window_w) );
	main_vbox_w = add_vbox( content_area, FALSE, 10 );

	frame_w = add_frame( main_vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );

	help_label_w = add_label( hbox_w, STR_DLG_Overview_TEXT );
	gtk_label_set_justify( GTK_LABEL(help_label_w), GTK_JUSTIFY_LEFT );

	gtk_widget_show_all( help_window_w );

	gint result = gtk_dialog_run (GTK_DIALOG (help_window_w));
	active = FALSE;
	gtk_widget_hide (help_window_w);
}


/* Help / Controls
 * Give a summary of mouse input syntax */
void
dialog_Help_Controls( GtkWidget *widget, int *message )
{
	GtkWidget *parent_window;
	static int active = FALSE;
	static GtkWidget *help_window_w;
	GtkWidget *content_area;
	GtkWidget *main_vbox_w;
	GtkWidget *frame_w;
	GtkWidget *help_label_w;
	GtkWidget *hbox_w;

	parent_window = gtk_widget_get_toplevel( widget );
	help_window_w = gtk_dialog_new_with_buttons(STR_DLG_Controls,
                             GTK_WINDOW(parent_window),
                             GTK_DIALOG_MODAL,
                             GTK_STOCK_CLOSE
                             );
	gtk_window_set_position( GTK_WINDOW(help_window_w), GTK_WIN_POS_CENTER );

	content_area = gtk_dialog_get_content_area ( GTK_DIALOG(help_window_w) );
	main_vbox_w = add_vbox( content_area, FALSE, 10 );

	frame_w = add_frame( main_vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );

	help_label_w = add_label( hbox_w, STR_DLG_Controls_TEXT );
	gtk_label_set_justify( GTK_LABEL(help_label_w), GTK_JUSTIFY_LEFT );

	gtk_widget_show_all (help_window_w);

	gint result = gtk_dialog_run (GTK_DIALOG (help_window_w));
	active = FALSE;
	gtk_widget_hide (help_window_w);
}


/* Help / About
 * Tell a little about ourselves */
void
dialog_Help_About( GtkWidget *widget, int *message )
{
	GtkWidget *parent_window;
	static int active = FALSE;
	static GtkWidget *about_window_w;
	GtkWidget *main_vbox_w;
	GtkWidget *entry_w;
	GtkWidget *text_label;
	char info_str[256];

	parent_window = gtk_widget_get_toplevel( widget );
	about_window_w = gtk_dialog_new_with_buttons(STR_DLG_About,
                             GTK_WINDOW(parent_window),
                             GTK_DIALOG_DESTROY_WITH_PARENT,
                             GTK_STOCK_CLOSE
                             );

	main_vbox_w = gtk_vbox_new(TRUE, 0);
	text_label = gtk_label_new(NULL);

	/* Get the Light Speed! title up */
	add_pixmap( main_vbox_w, about_window_w, lightspeed_title_xpm );

	sprintf( info_str, STR_DLG_Version_x_y_ARG, VERSION );
	gtk_label_set_markup(GTK_LABEL(text_label), info_str);
	gtk_box_pack_start(GTK_BOX(main_vbox_w), text_label, FALSE, FALSE, 0);
	sprintf( info_str, STR_DLG_authorship_ARG, "Daniel Richard G." );
	add_label( main_vbox_w, info_str );
	sprintf( info_str, "skunk@mit.edu" );
	add_label( main_vbox_w, info_str );
	sprintf( info_str, STR_copyright_ARG, 1999, "DRG" );
	add_label( main_vbox_w, info_str );

	/* Evaluate markup to get a clickable URL */
	text_label = gtk_label_new(NULL);
	sprintf( info_str, STR_DLG_home_page_url );
	gtk_label_set_markup(GTK_LABEL(text_label), info_str);
	gtk_box_pack_end(GTK_BOX(main_vbox_w), text_label, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(about_window_w))), main_vbox_w);
	gtk_widget_show_all (about_window_w);

	gint result = gtk_dialog_run (GTK_DIALOG (about_window_w));
	active = FALSE;
	gtk_widget_hide (about_window_w);
}

/* end menu_cbs.c */
