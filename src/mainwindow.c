/* mainwindow.c */

/* Constructs main window
 * (and menu of spawned windows) */

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

/* Construct main window */
void main_window( void )
{
	GtkWidget *main_window_w;
	GtkWidget *main_vbox_w;
	GtkWidget *hbox_w;
	GtkWidget *menu_bar_w;
	GtkWidget *menu_w;
	GtkWidget *menu_item_w;
	GdkPixbuf *window_icon_pixbuf;

	/* Main window widget */
	main_window_w = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW(main_window_w), STR_Light_Speed );
	gtk_widget_set_size_request( main_window_w, 600, 450 );
	gtk_container_set_border_width( GTK_CONTAINER(main_window_w), 0 );
	g_signal_connect( G_OBJECT(main_window_w), "focus_in_event",
	                    G_CALLBACK(camera_set_current), NULL );

	/* Need to set this so assoc_cam_id( ) will work */
	usr_cams[0]->window_w = main_window_w;
	usr_cams[0]->ogl_w = NULL; /* for ogl_make_widget( ) */

	/* (and its obligatory kill buttons) */
	g_signal_connect( G_OBJECT(main_window_w), "delete_event",
	                    G_CALLBACK(gtk_main_quit), NULL );
	/* Destroy window before exiting */
	/*TODO: This doesn't work in GTK3 */
	//gtk_quit_add_destroy( 1, G_OBJECT(main_window_w) );

	/* Main vertical box widget */
	main_vbox_w = add_vbox( main_window_w, FALSE, 0 );

	/* Horizontal box for menu bar + velocity entry */
	hbox_w = add_hbox( main_vbox_w, FALSE, 0 );

	/* Initialize keybinding */
	keybind( NULL, NULL );

	/* Build menu bar */

	/* Root menu bar widget */
	menu_bar_w = gtk_menu_bar_new( );
	gtk_box_pack_start( GTK_BOX(hbox_w), menu_bar_w, TRUE, TRUE, 0 );
	gtk_widget_show( menu_bar_w );

	/* File menu */
	menu_w = add_menu( menu_bar_w, STR_MNU_File );
	/* File menu items */
	menu_item_w = add_menu_item( menu_w, STR_MNU_New_lattice, dialog_File_NewLattice, MESG_(DIALOG_OPEN) );
	keybind( menu_item_w, "^N" );
#ifdef WITH_OBJECT_IMPORTER
	menu_item_w = add_menu_item( menu_w, STR_MNU_Load_object, dialog_File_ImportObject, MESG_(DIALOG_OPEN) );
	keybind( menu_item_w, "^O" );
#endif
#ifdef CAN_SAVE_SNAPSHOT
	menu_item_w = add_menu_item( menu_w, STR_MNU_Save_snapshot, dialog_File_SaveSnapshot, MESG_(DIALOG_OPEN) );
	keybind( menu_item_w, "^S" );
#endif
#ifdef WITH_SRS_EXPORTER
	add_menu_item( menu_w, STR_MNU_Export_srs, dialog_File_ExportSRS, MESG_(DIALOG_OPEN) );
#endif
	add_separator( menu_w );
	menu_item_w = add_menu_item( menu_w, STR_MNU_Exit, gtk_main_quit, NULL );
	keybind( menu_item_w, "^Q" );

	/* Objects menu */
	menu_w = add_menu( menu_bar_w, STR_MNU_Objects );
	/* Objects menu items */
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Coordinate_axes, FALSE, menu_Objects_toggles, MESG_(AUXOBJS_SET_AXES) );
	keybind( menu_item_w, "C" );
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Floating_grid, TRUE, menu_Objects_toggles, MESG_(AUXOBJS_SET_GRID) );
	keybind( menu_item_w, "G" );
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Bounding_box, FALSE, menu_Objects_toggles, MESG_(AUXOBJS_SET_BBOX) );
	keybind( menu_item_w, "B" );
	add_separator( menu_w );
	menu_item_w = add_menu_item( menu_w, STR_MNU_Animation, dialog_Objects_Animation, MESG_(DIALOG_OPEN) );
	keybind( menu_item_w, "A" );

	/* Warp menu */
	menu_w = add_menu( menu_bar_w, STR_MNU_Warp );
	/* Warp menu items */
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Lorentz_contraction, TRUE, menu_Warp_toggles, MESG_(WARP_LORENTZ_CONTRACTION) );
	keybind( menu_item_w, "L" );
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Doppler_shift, TRUE, menu_Warp_toggles, MESG_(WARP_DOPPLER_SHIFT) );
	keybind( menu_item_w, "D" );
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Headlight_effect, TRUE, menu_Warp_toggles, MESG_(WARP_HEADLIGHT_EFFECT) );
	keybind( menu_item_w, "H" );
	menu_item_w = add_check_menu_item( menu_w, STR_MNU_Optical_deformation, TRUE, menu_Warp_toggles, MESG_(WARP_OPTICAL_DEFORMATION) );
	keybind( menu_item_w, "O" );

	/* Camera menu defined elsewhere */
	add_Camera_menu( menu_bar_w, main_window_w );

	/* Help menu */
	menu_w = add_menu( menu_bar_w, STR_MNU_Help );
	/* Help menu items */
	add_menu_item( menu_w, STR_MNU_Overview, dialog_Help_Overview, MESG_(DIALOG_OPEN) );
	add_menu_item( menu_w, STR_MNU_Controls, dialog_Help_Controls, MESG_(DIALOG_OPEN) );
	add_separator( menu_w );
	add_menu_item( menu_w, STR_MNU_About, dialog_Help_About, MESG_(DIALOG_OPEN) );

	/* Done with the menu bar */

	/* Velocity entry/units select button */
	velocity_input( hbox_w, MESG_(INITIALIZE) );

	/* Horizontal box for viewport and velocity slider */
	hbox_w = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox_w), hbox_w, TRUE, TRUE, 0 );
	gtk_widget_show( hbox_w );

#if (VELOCITY_SLIDER == 1)
	velocity_slider( hbox_w, MESG_(INITIALIZE) );
#endif

	/* OpenGL area widget */
	usr_cams[0]->ogl_w = add_gl_area( hbox_w );

#if (VELOCITY_SLIDER == 2)
	velocity_slider( hbox_w, MESG_(INITIALIZE) );
#endif

	/* Attach keybindings */
	keybind( main_window_w, NULL );

	/* Give window a nifty icon */
	window_icon_pixbuf = gdk_pixbuf_new_from_resource ("/lightspeed/icon.png", NULL);
	gtk_window_set_icon ( GTK_WINDOW(main_window_w), window_icon_pixbuf );
	gtk_window_set_default_icon ( window_icon_pixbuf );
	/* Also use the icon for all dialog windows */
	gtk_widget_show( main_window_w );
}


/* Camera menu is common to main and spawned windows, thus defined separately */
void add_Camera_menu( GtkWidget *menu_bar_w, GtkWidget *window_w )
{
	GtkWidget *menu_w;
	GtkWidget *menu_item_w;
	GtkWidget *submenu_w;
	char lens_name[16];
	int cam_id;
	int spawned_camera = FALSE;
	int i;

	cam_id = assoc_cam_id( window_w );
	if (cam_id > 0)
		spawned_camera = TRUE;

	/* Camera menu */
	menu_w = add_menu( menu_bar_w, STR_MNU_Camera );
	/* Lens submenu */
	submenu_w = add_menu( menu_w, STR_MNU_Lens );
	/* Lens submenu items */
	begin_radio_menu_group( num_stock_lenses ); /* custom lens is default */
	for (i = 0; i < num_stock_lenses; i++) {
		sprintf( lens_name, "%.0fmm", stock_lenses[i] );
		add_radio_menu_item( submenu_w, lens_name, menu_Camera_Lens_select, (float *)(&stock_lenses[i]) );
	}
	add_separator( submenu_w );
	menu_item_w = add_radio_menu_item( submenu_w, STR_MNU_Custom, dialog_Camera_Lens_Custom, MESG_(DIALOG_OPEN) );
	g_signal_connect( G_OBJECT(menu_item_w), "activate",
	                    G_CALLBACK(dialog_Camera_Lens_Custom), MESG_(DIALOG_OPEN) );
	/* Lens submenu finished */
	if (advanced_interface)
		add_menu_item( menu_w, STR_MNU_Position, dialog_Camera_Position, MESG_(DIALOG_OPEN) );
	menu_item_w = add_menu_item( menu_w, STR_MNU_Reset_view, menu_Camera_ResetView, MESG_(cam_id) );
	keybind( menu_item_w, "R" );
	add_separator( menu_w );
	if (!spawned_camera) {
		/* For primary camera only */
		/* Info display submenu */
		submenu_w = add_menu( menu_w, STR_MNU_Info_display );
		/* Info display submenu items */
		menu_item_w = add_check_menu_item( submenu_w, STR_MNU_Active, DEF_INFODISP_ACTIVE, menu_Camera_InfoDisplay_toggles, MESG_(INFODISP_ACTIVE) );
		keybind( menu_item_w, "I" );
		add_separator( submenu_w );
		add_check_menu_item( submenu_w, STR_MNU_Velocity, DEF_INFODISP_SHOW_VELOCITY, menu_Camera_InfoDisplay_toggles, MESG_(INFODISP_SHOW_VELOCITY) );
		add_check_menu_item( submenu_w, STR_MNU_Time_t, DEF_INFODISP_SHOW_TIME_T, menu_Camera_InfoDisplay_toggles, MESG_(INFODISP_SHOW_TIME_T) );
		add_check_menu_item( submenu_w, STR_MNU_Gamma_factor, DEF_INFODISP_SHOW_GAMMA, menu_Camera_InfoDisplay_toggles, MESG_(INFODISP_SHOW_GAMMA) );
		add_check_menu_item( submenu_w, STR_MNU_Framerate, DEF_INFODISP_SHOW_FRAMERATE, menu_Camera_InfoDisplay_toggles, MESG_(INFODISP_SHOW_FRAMERATE) );
		/* Info display submenu finished */
		/* Background submenu */
		submenu_w = add_menu( menu_w, STR_MNU_Background );
		/* Background submenu items */
		begin_radio_menu_group( 0 ); /* first one (black) is default */
		for (i = 0; i < num_background_colors; i++)
			add_radio_menu_item( submenu_w, STRS_MNU_bkgd_color_names[i], menu_Camera_Background_select, MESG_(i) );
		/* Background submenu finished */
		if (advanced_interface) {
			/* Graphics mode submenu */
			submenu_w = add_menu( menu_w, STR_MNU_Graphics_mode );
			/* Graphics mode submenu items */
			begin_radio_menu_group( 1 ); /* Shaded mode is default */
			add_radio_menu_item( submenu_w, STR_MNU_Wireframe, menu_Camera_GraphicsMode_select, MESG_(OGL_WIREFRAME_MODE) );
			add_radio_menu_item( submenu_w, STR_MNU_Shaded, menu_Camera_GraphicsMode_select, MESG_(OGL_SHADED_MODE) );
			/* Graphics mode submenu finished */
			add_separator( menu_w );
			menu_item_w = add_menu_item( menu_w, STR_MNU_Spawn_camera, menu_Camera_Spawn, NULL );
			keybind( menu_item_w, "S" );
		}
	}
	else {
		/* For spawned cameras */
		menu_item_w = add_menu_item( menu_w, STR_MNU_Close, NULL, G_OBJECT(window_w) );
		g_signal_connect( G_OBJECT(menu_item_w), "activate",
		                           G_CALLBACK(menu_Camera_Close), G_OBJECT(window_w) );
		keybind( menu_item_w, "^X" );
	}
}

/* end mainwindow.c */
