/* gtkwidgets.c */

/* GTK+ UI-building semi-abstractions
 * (Most of the drudge code is in here) */

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


/* Creates and installs an OpenGL widget into specified box */
GtkWidget *
add_gl_area( GtkWidget *parent_box_w )
{
	GtkWidget *ogl_w;

	ogl_w = ogl_make_widget( );
	gtk_box_pack_start( GTK_BOX(parent_box_w), ogl_w, TRUE, TRUE, 0 );
	gtk_widget_show( ogl_w );
	gtk_widget_set_events( GTK_WIDGET(ogl_w), GDK_EXPOSURE_MASK |
	                       GDK_BUTTON_MOTION_MASK | GDK_BUTTON1_MOTION_MASK |
	                       GDK_BUTTON2_MOTION_MASK | GDK_BUTTON3_MOTION_MASK |
	                       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK );
	/* Attach the signal handlers */
	g_signal_connect( G_OBJECT(ogl_w), "realize",
	                    G_CALLBACK(ogl_initialize), NULL );
	g_signal_connect( G_OBJECT(ogl_w), "draw",
	                    G_CALLBACK(ogl_refresh), NULL );
	g_signal_connect( G_OBJECT(ogl_w), "configure_event",
	                    G_CALLBACK(ogl_resize), NULL );
	g_signal_connect( G_OBJECT(ogl_w), "button_press_event",
	                    G_CALLBACK(camera_move), NULL );
	g_signal_connect( G_OBJECT(ogl_w), "button_release_event",
	                    G_CALLBACK(camera_move), NULL );
	g_signal_connect( G_OBJECT(ogl_w), "motion_notify_event",
	                    G_CALLBACK(camera_move), NULL );

	return ogl_w;
}


/* Creates a base dialog window */
GtkWidget *
make_dialog_window( const char *title, void *callback_close )
{
	GtkWidget *dialog_window_w;

	dialog_window_w = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW(dialog_window_w), title );
	gtk_container_set_border_width( GTK_CONTAINER(dialog_window_w), 0 );
	if (callback_close != NULL) {
		g_signal_connect( G_OBJECT(dialog_window_w), "destroy",
		                    G_CALLBACK(callback_close), MESG_(DIALOG_CLOSE) );
	}
	/* Note, no gtk_widget_show( ) */

	return dialog_window_w;
}


/* Creates a simple, click-away message dialog */
GtkWidget *
message_window( const char *title, const char *message_text )
{
	GtkWidget *message_window_w;
	GtkWidget *vbox_w;
	GtkWidget *hbox_w;
	GtkWidget *frame_w;
	GtkWidget *button_w;

	message_window_w = make_dialog_window( title, NULL );
	gtk_window_set_position( GTK_WINDOW(message_window_w), GTK_WIN_POS_MOUSE );
	g_signal_connect( G_OBJECT(message_window_w), "delete_event",
	                    G_CALLBACK(gtk_widget_destroy), NULL );
	vbox_w = add_vbox( message_window_w, FALSE, 10 );
	frame_w = add_frame( vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );
	add_label( hbox_w, message_text );

	/* OK button */
	button_w = gtk_button_new( );
	add_label( button_w, "OK" );
	gtk_box_pack_start( GTK_BOX(vbox_w), button_w, TRUE, TRUE, 0 );
	g_signal_connect_swapped( G_OBJECT(button_w), "clicked",
	                           G_CALLBACK(gtk_widget_destroy), G_OBJECT(message_window_w) );
	gtk_widget_show( button_w );

	gtk_widget_show( message_window_w );

	return message_window_w;
}


/* Confirmation dialog */
GtkWidget *
confirmation_dialog( const char *title, const char *message_text, void *callback )
{
	GtkWidget *confirm_window_w;
	GtkWidget *main_vbox_w;
	GtkWidget *hbox_w;
	GtkWidget *frame_w;
	GtkWidget *button_w;

	confirm_window_w = make_dialog_window( title, callback );
	gtk_window_set_position( GTK_WINDOW(confirm_window_w), GTK_WIN_POS_MOUSE );
	g_signal_connect( G_OBJECT(confirm_window_w), "delete_event",
	                    G_CALLBACK(gtk_widget_destroy), NULL );
	main_vbox_w = add_vbox( confirm_window_w, FALSE, 10 );
	frame_w = add_frame( main_vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );
	add_label( hbox_w, message_text );
	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );

	/* OK button */
	button_w = gtk_button_new( );
	add_label( button_w, "OK" );
	gtk_box_pack_start( GTK_BOX(hbox_w), button_w, TRUE, TRUE, 0 );
	g_signal_connect_swapped( G_OBJECT(button_w), "clicked",
	                           G_CALLBACK(gtk_widget_hide), G_OBJECT(confirm_window_w) );
	g_signal_connect( G_OBJECT(button_w), "clicked",
	                    G_CALLBACK(callback), MESG_(DIALOG_OK_CONFIRM) );
	g_signal_connect_swapped( G_OBJECT(button_w), "clicked",
	                           G_CALLBACK(gtk_widget_destroy), G_OBJECT(confirm_window_w) );
	gtk_widget_show( button_w );

	add_hbox( hbox_w, TRUE, 0 ); /* spacer */

	/* Cancel button */
	button_w = gtk_button_new( );
	add_label( button_w, "Cancel" );
	gtk_box_pack_start( GTK_BOX(hbox_w), button_w, TRUE, TRUE, 0 );
	g_signal_connect_swapped( G_OBJECT(button_w), "clicked",
	                           G_CALLBACK(gtk_widget_hide), G_OBJECT(confirm_window_w) );
	g_signal_connect( G_OBJECT(button_w), "clicked",
	                    G_CALLBACK(callback), MESG_(DIALOG_CLOSE) );
	g_signal_connect_swapped( G_OBJECT(button_w), "clicked",
	                           G_CALLBACK(gtk_widget_destroy), G_OBJECT(confirm_window_w) );
	gtk_widget_show( button_w );

	gtk_widget_show( confirm_window_w );

	return confirm_window_w;
}


/* Add a menu to a menu bar (or a submenu to a menu) */
GtkWidget *
add_menu( GtkWidget *parent_menu_w, const char *label )
{
	GtkWidget *menu_item_w;
	GtkWidget *menu_w;

	menu_item_w = gtk_menu_item_new_with_label( label );
	gtk_menu_shell_append( GTK_MENU_SHELL(parent_menu_w), menu_item_w );
	gtk_widget_show( menu_item_w );
	menu_w = gtk_menu_new( );
	gtk_menu_item_set_submenu( GTK_MENU_ITEM(menu_item_w), menu_w );

	return menu_w;
}


/* Add a menu item to a menu */
GtkWidget *
add_menu_item( GtkWidget *menu_w, const char *label, void *callback, void *callback_data )
{
	GtkWidget *menu_item_w;

	menu_item_w = gtk_menu_item_new_with_label( label );
	gtk_menu_shell_append( GTK_MENU_SHELL(menu_w), menu_item_w );
	if (callback != NULL) {
		g_signal_connect( G_OBJECT(menu_item_w), "activate",
		                    G_CALLBACK(callback), callback_data );
	}
	gtk_widget_show( menu_item_w );

	return menu_item_w;
}


/* Add a check-button menu item to a menu */
GtkWidget *
add_check_menu_item( GtkWidget *menu_w, const char *label, int init_state, void *callback, void *callback_data )
{
	GtkWidget *chkmenu_item_w;

	chkmenu_item_w = gtk_check_menu_item_new_with_label( label );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(chkmenu_item_w), init_state );
	/*TODO: Looks like the next call is unnecessary in GTK3 but better check */
	// gtk_check_menu_item_set_show_toggle( GTK_CHECK_MENU_ITEM(chkmenu_item_w), TRUE );
	gtk_menu_shell_append( GTK_MENU_SHELL(menu_w), chkmenu_item_w );
	g_signal_connect( G_OBJECT(chkmenu_item_w), "toggled",
	                    G_CALLBACK(callback), callback_data );
	gtk_widget_show( chkmenu_item_w );

	return chkmenu_item_w;
}


/* Initiates the definition of a radio menu item group, having a specified
 * initially selected item (0 == first, 1 == second, etc.) */
void
begin_radio_menu_group( int init_selected )
{
	add_radio_menu_item( NULL, (char *)&init_selected, NULL, NULL );
}


/* Add a radio-button menu item to a menu
 * NOTE: Call begin_radio_menu_group( ) before defining the group! */
GtkWidget *
add_radio_menu_item( GtkWidget *menu_w, const char *label, void *callback, void *callback_data )
{
	static GSList *radio_group;
	static int radmenu_item_num;
	static int init_selected;
	GtkWidget *radmenu_item_w = NULL;

	if (menu_w != NULL) {
		radmenu_item_w = gtk_radio_menu_item_new_with_label( radio_group, label );
		radio_group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(radmenu_item_w) );
		gtk_menu_shell_append( GTK_MENU_SHELL(menu_w), radmenu_item_w );
		if (radmenu_item_num == init_selected)
			gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(radmenu_item_w), TRUE );
		g_signal_connect( G_OBJECT(radmenu_item_w), "toggled",
		                    G_CALLBACK(callback), callback_data );
		gtk_widget_show( radmenu_item_w );
		++radmenu_item_num;
	}
	else {
		/* we're being called from begin_radio_menu_group( ) */
		radio_group = NULL;
		radmenu_item_num = 0;
		init_selected = *((int *)label);
	}

	return radmenu_item_w;
}


/* Option menu definiton
 * Call this however many times necessary, and then call add_option_menu( )
 * to stick in the finished widget */
 /*TODO: Possibly this isn't needed in GTK3 any more */
/*
GtkWidget *
option_menu_item( const char *label, void *callback, void *callback_data )
{
	GtkWidget *menu_item_w;

	menu_item_w = gtk_menu_item_new_with_label( label );
	if (callback != NULL) {
		g_signal_connect( G_OBJECT(menu_item_w), "activate",
		                    G_CALLBACK(callback), callback_data );
	}
	add_option_menu( menu_item_w );

	return menu_item_w;
}
*/

/* The ever-ubiquitous separator */
void
add_separator( GtkWidget *parent_w )
{
	GtkWidget *separator_w;

	if (GTK_IS_MENU(parent_w)) {
		separator_w = gtk_menu_item_new( );
		gtk_menu_shell_append( GTK_MENU_SHELL(parent_w), separator_w );
	}
	else {
		separator_w = gtk_separator_new( GTK_ORIENTATION_HORIZONTAL );
		gtk_box_pack_start( GTK_BOX(parent_w), separator_w, TRUE, TRUE, 0 );
	}
	gtk_widget_show( separator_w );
}


/* Sets up keybindings (accelerators)
 * Call first with widget == NULL, then with any number of widget/keystroke
 * pairs, and finally with the parent window. */
void
keybind( GtkWidget *widget, char *keys )
{
	static GtkAccelGroup *accel_group;
	int mods;
	char k;

	if (widget == NULL) {
		accel_group = gtk_accel_group_new( );
		return;
	}
	if (GTK_IS_WINDOW(widget)) {
		gtk_window_add_accel_group( GTK_WINDOW(widget), accel_group );
		return;
	}

	/* Parse keystroke string */
	switch (keys[0]) {
	case '^':
		/* Ctrl- keystroke specified */
		mods = GDK_CONTROL_MASK;
		k = keys[1];
		break;

	default:
		/* Simple keypress */
		mods = 0;
		k = keys[0];
		break;
	}

	if (GTK_IS_MENU_ITEM(widget)) {
		gtk_widget_add_accelerator( widget, "activate", accel_group, k, mods, GTK_ACCEL_VISIBLE );
		return;
	}
	if (GTK_IS_BUTTON(widget)) {
		gtk_widget_add_accelerator( widget, "clicked", accel_group, k, mods, GTK_ACCEL_VISIBLE );
		return;
	}
	/* Default: make widget grab focus */
	gtk_widget_add_accelerator( widget, "grab_focus", accel_group, k, mods, GTK_ACCEL_VISIBLE );
}


/* The horizontal box widget */
GtkWidget *
add_hbox( GtkWidget *parent_w, int homog, int spacing )
{
	GtkWidget *hbox_w;

	hbox_w = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, spacing );
	gtk_container_set_border_width( GTK_CONTAINER(hbox_w), spacing );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), hbox_w, FALSE, FALSE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), hbox_w );
	gtk_widget_show( hbox_w );

	return hbox_w;
}


/* The vertical box widget */
GtkWidget *
add_vbox( GtkWidget *parent_w, int homog, int spacing )
{
	GtkWidget *vbox_w;

	vbox_w = gtk_box_new( GTK_ORIENTATION_VERTICAL, spacing );
	gtk_container_set_border_width( GTK_CONTAINER(vbox_w), spacing );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), vbox_w, FALSE, FALSE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), vbox_w );
	gtk_widget_show( vbox_w );

	return vbox_w;
}


/* Frame widget */
GtkWidget *
add_frame( GtkWidget *parent_w, const char *title )
{
	GtkWidget *frame_w;

	frame_w = gtk_frame_new( title );
	if (parent_w != NULL) {
		if (GTK_IS_BOX(parent_w))
			gtk_box_pack_start( GTK_BOX(parent_w), frame_w, FALSE, FALSE, 0 );
		else
			gtk_container_add( GTK_CONTAINER(parent_w), frame_w );
		gtk_widget_show( frame_w );
	}

	return frame_w;
}


/* The horizontal value slider widget */
GtkWidget *
add_hscale( GtkWidget *parent_w, GObject *adjustment )
{
	GtkWidget *hscale_w;

	hscale_w = gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adjustment) );
	gtk_scale_set_digits( GTK_SCALE(hscale_w), 0 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), hscale_w, TRUE, TRUE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), hscale_w );
	gtk_widget_show( hscale_w );

	return hscale_w;
}


/* The vertical value slider widget */
GtkWidget *
add_vscale( GtkWidget *parent_w, GObject *adjustment )
{
	GtkWidget *vscale_w;

	vscale_w = gtk_scale_new( GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT(adjustment) );
	gtk_scale_set_value_pos( GTK_SCALE(vscale_w), GTK_POS_RIGHT );
	gtk_scale_set_digits( GTK_SCALE(vscale_w), 0 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), vscale_w, TRUE, TRUE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), vscale_w );
	gtk_widget_show( vscale_w );

	return vscale_w;
}


/* The spin button widget */
GtkWidget *
add_spin_button( GtkWidget *parent_w, GObject *adjustment )
{
	GtkWidget *spin_button_w;

	spin_button_w = gtk_spin_button_new( GTK_ADJUSTMENT(adjustment), 0.0, 0 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), spin_button_w, TRUE, FALSE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), spin_button_w );
	gtk_widget_show( spin_button_w );

	return spin_button_w;
}


/* The check button widget */
GtkWidget *
add_check_button( GtkWidget *parent_w, const char *label, int init_state, void *callback, void *callback_data )
{
	GtkWidget *chkbutton_w;

	chkbutton_w = gtk_check_button_new_with_label( label );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(chkbutton_w), init_state );
	gtk_toggle_button_set_mode( GTK_TOGGLE_BUTTON(chkbutton_w), TRUE );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), chkbutton_w, TRUE, TRUE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), chkbutton_w );
	if (callback != NULL) {
		g_signal_connect( G_OBJECT(chkbutton_w), "toggled",
		                    G_CALLBACK(callback), callback_data );
	}
	gtk_widget_show( chkbutton_w );

	return chkbutton_w;
}


/* Text/data/number entry */
GtkWidget *
add_entry( GtkWidget *parent_w, const char *init_str, void *callback, void *callback_data )
{
	GtkWidget *entry_w;

	entry_w = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY(entry_w), 16 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), entry_w, FALSE, FALSE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), entry_w );
	gtk_entry_set_text( GTK_ENTRY(entry_w), init_str );
	if (callback != NULL )
		g_signal_connect( G_OBJECT(entry_w), "activate",
		                    G_CALLBACK(callback), callback_data );
	gtk_widget_show( entry_w );

	return entry_w;
}


/* Sets an entry to the width of a specified string */
void
set_entry_width( GtkWidget *entry_w, const char *span_str )
{
	gint width;

	width = sizeof(span_str);
	gtk_entry_set_width_chars( GTK_ENTRY(entry_w), width );
}


/* Resets the text in an entry to specified string */
void
set_entry_text( GtkWidget *entry_w, const char *entry_text )
{
	gtk_entry_set_text( GTK_ENTRY(entry_w), entry_text );
}


/* Returns the text in an entry */
char *
read_entry( GtkWidget *entry_w )
{
	return gtk_entry_get_text( GTK_ENTRY(entry_w) );
}


/* Highlights the text in an entry */
void
highlight_entry( GtkWidget *entry_w )
{
	gtk_editable_select_region( GTK_EDITABLE(entry_w), 0, -1 );
}


GtkWidget *
add_button( GtkWidget *parent_w, const char *label, void *callback, void *callback_data )
{
	GtkWidget *button_w;

	button_w = gtk_button_new( );
	if (label != NULL)
		add_label( button_w, label );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), button_w, TRUE, TRUE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), button_w );
	g_signal_connect( G_OBJECT(button_w), "clicked",
	                    G_CALLBACK(callback), callback_data );
	gtk_widget_show( button_w );

	return button_w;
}


GtkWidget *
add_label( GtkWidget *parent_w, const char *label_text )
{
	GtkWidget *label_w;
	GtkWidget *hbox_w;

	label_w = gtk_label_new( label_text );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), label_w, FALSE, FALSE, 0 );
	else if (GTK_IS_BUTTON(parent_w)) {
		/* Labels are often too snug inside buttons */
		hbox_w = add_hbox( parent_w, FALSE, 0 );
		gtk_box_pack_start( GTK_BOX(hbox_w), label_w, TRUE, FALSE, 5 );
	}
	else
		gtk_container_add( GTK_CONTAINER(parent_w), label_w );
	gtk_widget_show( label_w );

	return label_w;
}


/* Adds an XPM */
/*
GtkWidget *
add_pixmap( GtkWidget *parent_w, GtkWidget *parent_window_w, char **xpm_data )
{
	GtkWidget *pixmap_w;
	GtkStyle *style;
	GdkPixmap *pixmap;
	GdkBitmap *mask;

	// Realize the window to prevent a "NULL window" error
	gtk_widget_realize( parent_window_w );
	style = gtk_widget_get_style( parent_window_w );
	pixmap = gdk_pixmap_create_from_xpm_d( parent_window_w->window, &mask,
	                                       &style->bg[GTK_STATE_NORMAL], xpm_data );
	pixmap_w = gtk_pixmap_new( pixmap, mask );
	gdk_pixmap_unref( pixmap );
	gdk_bitmap_unref( mask );
	gtk_box_pack_start( GTK_BOX(parent_w), pixmap_w, FALSE, FALSE, 0 );
	gtk_widget_show( pixmap_w );

	return pixmap_w;
}
*/

/* end gtkwidgets.c */
