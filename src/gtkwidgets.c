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
	gtk_signal_connect( GTK_OBJECT(ogl_w), "realize",
	                    GTK_SIGNAL_FUNC(ogl_initialize), NULL );
	gtk_signal_connect( GTK_OBJECT(ogl_w), "expose_event",
	                    GTK_SIGNAL_FUNC(ogl_refresh), NULL );
	gtk_signal_connect( GTK_OBJECT(ogl_w), "configure_event",
	                    GTK_SIGNAL_FUNC(ogl_resize), NULL );
	gtk_signal_connect( GTK_OBJECT(ogl_w), "button_press_event",
	                    GTK_SIGNAL_FUNC(camera_move), NULL );
	gtk_signal_connect( GTK_OBJECT(ogl_w), "button_release_event",
	                    GTK_SIGNAL_FUNC(camera_move), NULL );
	gtk_signal_connect( GTK_OBJECT(ogl_w), "motion_notify_event",
	                    GTK_SIGNAL_FUNC(camera_move), NULL );

	return ogl_w;
}


/* Creates a base dialog window */
GtkWidget *
make_dialog_window( const char *title, void *callback_close )
{
	GtkWidget *dialog_window_w;

	dialog_window_w = gtk_window_new( GTK_WINDOW_DIALOG );
	gtk_window_set_title( GTK_WINDOW(dialog_window_w), title );
	gtk_container_set_border_width( GTK_CONTAINER(dialog_window_w), 0 );
	if (callback_close != NULL) {
		gtk_signal_connect( GTK_OBJECT(dialog_window_w), "destroy",
		                    GTK_SIGNAL_FUNC(callback_close), MESG_(DIALOG_CLOSE) );
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
	gtk_signal_connect( GTK_OBJECT(message_window_w), "delete_event",
	                    GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL );
	vbox_w = add_vbox( message_window_w, FALSE, 10 );
	frame_w = add_frame( vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );
	add_label( hbox_w, message_text );

	/* OK button */
	button_w = gtk_button_new( );
	add_label( button_w, "OK" );
	gtk_box_pack_start( GTK_BOX(vbox_w), button_w, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button_w), "clicked",
	                           GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(message_window_w) );
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
	gtk_signal_connect( GTK_OBJECT(confirm_window_w), "delete_event",
	                    GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL );
	main_vbox_w = add_vbox( confirm_window_w, FALSE, 10 );
	frame_w = add_frame( main_vbox_w, NULL );
	hbox_w = add_hbox( frame_w, FALSE, 10 );
	add_label( hbox_w, message_text );
	hbox_w = add_hbox( main_vbox_w, TRUE, 0 );

	/* OK button */
	button_w = gtk_button_new( );
	add_label( button_w, "OK" );
	gtk_box_pack_start( GTK_BOX(hbox_w), button_w, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button_w), "clicked",
	                           GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(confirm_window_w) );
	gtk_signal_connect( GTK_OBJECT(button_w), "clicked",
	                    GTK_SIGNAL_FUNC(callback), MESG_(DIALOG_OK_CONFIRM) );
	gtk_signal_connect_object( GTK_OBJECT(button_w), "clicked",
	                           GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(confirm_window_w) );
	gtk_widget_show( button_w );

	add_hbox( hbox_w, TRUE, 0 ); /* spacer */

	/* Cancel button */
	button_w = gtk_button_new( );
	add_label( button_w, "Cancel" );
	gtk_box_pack_start( GTK_BOX(hbox_w), button_w, TRUE, TRUE, 0 );
	gtk_signal_connect_object( GTK_OBJECT(button_w), "clicked",
	                           GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(confirm_window_w) );
	gtk_signal_connect( GTK_OBJECT(button_w), "clicked",
	                    GTK_SIGNAL_FUNC(callback), MESG_(DIALOG_CLOSE) );
	gtk_signal_connect_object( GTK_OBJECT(button_w), "clicked",
	                           GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(confirm_window_w) );
	gtk_widget_show( button_w );

	gtk_widget_show( confirm_window_w );

	return confirm_window_w;
}


/* Creates a file selection dialog */
GtkWidget *
make_filesel_window( const char *title, const char *init_filename, int show_fileops, void *callback_handler )
{
	GtkWidget *filesel_w;

	filesel_w = gtk_file_selection_new( title );
	if (!show_fileops)
		gtk_file_selection_hide_fileop_buttons( GTK_FILE_SELECTION(filesel_w) );
	if (init_filename != NULL)
		gtk_file_selection_set_filename( GTK_FILE_SELECTION(filesel_w), init_filename );
	gtk_signal_connect( GTK_OBJECT(filesel_w), "destroy",
	                    GTK_SIGNAL_FUNC(callback_handler), MESG_(DIALOG_CLOSE) );
	gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(filesel_w)->cancel_button),
	                    "clicked", GTK_SIGNAL_FUNC(callback_handler), MESG_(DIALOG_CLOSE) );
	gtk_signal_connect( GTK_OBJECT(GTK_FILE_SELECTION(filesel_w)->ok_button),
	                    "clicked", GTK_SIGNAL_FUNC(callback_handler), MESG_(DIALOG_OK) );

	return filesel_w;
}


/* Add a menu to a menu bar (or a submenu to a menu) */
GtkWidget *
add_menu( GtkWidget *parent_menu_w, const char *label )
{
	GtkWidget *menu_item_w;
	GtkWidget *menu_w;

	menu_item_w = gtk_menu_item_new_with_label( label );
	/* parent_menu can be a menu bar or a regular menu */
	if (GTK_IS_MENU_BAR(parent_menu_w))
		gtk_menu_bar_append( GTK_MENU_BAR(parent_menu_w), menu_item_w );
	else
		gtk_menu_append( GTK_MENU(parent_menu_w), menu_item_w );
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
	gtk_menu_append( GTK_MENU(menu_w), menu_item_w );
	if (callback != NULL) {
		gtk_signal_connect( GTK_OBJECT(menu_item_w), "activate",
		                    GTK_SIGNAL_FUNC(callback), callback_data );
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
	gtk_check_menu_item_set_show_toggle( GTK_CHECK_MENU_ITEM(chkmenu_item_w), TRUE );
	gtk_menu_append( GTK_MENU(menu_w), chkmenu_item_w );
	gtk_signal_connect( GTK_OBJECT(chkmenu_item_w), "toggled",
	                    GTK_SIGNAL_FUNC(callback), callback_data );
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
		radio_group = gtk_radio_menu_item_group( GTK_RADIO_MENU_ITEM(radmenu_item_w) );
		gtk_menu_append( GTK_MENU(menu_w), radmenu_item_w );
		if (radmenu_item_num == init_selected)
			gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(radmenu_item_w), TRUE );
		gtk_signal_connect( GTK_OBJECT(radmenu_item_w), "toggled",
		                    GTK_SIGNAL_FUNC(callback), callback_data );
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
GtkWidget *
option_menu_item( const char *label, void *callback, void *callback_data )
{
	GtkWidget *menu_item_w;

	menu_item_w = gtk_menu_item_new_with_label( label );
	if (callback != NULL) {
		gtk_signal_connect( GTK_OBJECT(menu_item_w), "activate",
		                    GTK_SIGNAL_FUNC(callback), callback_data );
	}
	add_option_menu( menu_item_w );

	return menu_item_w;
}


/* Add the finished option menu into the specified box widget
 * (widget can only be a menu item when called from option_menu_item( )) */
GtkWidget *
add_option_menu( GtkWidget *widget )
{
	static GtkWidget *menu_w = NULL;
	GtkWidget *optmenu_w;

	if (GTK_IS_MENU_ITEM(widget)) {
		if (menu_w == NULL)
			menu_w = gtk_menu_new( );
		gtk_menu_append( GTK_MENU(menu_w), widget );
		gtk_widget_show( widget );
		return NULL;
	}

	/* widget is a Gtk?box */
	optmenu_w = gtk_option_menu_new( );
	gtk_option_menu_set_menu( GTK_OPTION_MENU(optmenu_w), menu_w );
	gtk_box_pack_start( GTK_BOX(widget), optmenu_w, FALSE, FALSE, 0 );
	gtk_widget_show( optmenu_w );
	menu_w = NULL;

	return optmenu_w;
}


/* The ever-ubiquitous separator */
void
add_separator( GtkWidget *parent_w )
{
	GtkWidget *separator_w;

	if (GTK_IS_MENU(parent_w)) {
		separator_w = gtk_menu_item_new( );
		gtk_menu_append( GTK_MENU(parent_w), separator_w );
	}
	else {
		separator_w = gtk_hseparator_new( );
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
		gtk_accel_group_attach( accel_group, GTK_OBJECT(widget) );
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

	hbox_w = gtk_hbox_new( homog, spacing );
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

	vbox_w = gtk_vbox_new( homog, spacing );
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
add_hscale( GtkWidget *parent_w, GtkObject *adjustment )
{
	GtkWidget *hscale_w;

	hscale_w = gtk_hscale_new( GTK_ADJUSTMENT(adjustment) );
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
add_vscale( GtkWidget *parent_w, GtkObject *adjustment )
{
	GtkWidget *vscale_w;

	vscale_w = gtk_vscale_new( GTK_ADJUSTMENT(adjustment) );
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
add_spin_button( GtkWidget *parent_w, GtkObject *adjustment )
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
		gtk_signal_connect( GTK_OBJECT(chkbutton_w), "toggled",
		                    GTK_SIGNAL_FUNC(callback), callback_data );
	}
	gtk_widget_show( chkbutton_w );

	return chkbutton_w;
}


/* Text/data/number entry */
GtkWidget *
add_entry( GtkWidget *parent_w, const char *init_str, void *callback, void *callback_data )
{
	GtkWidget *entry_w;

	entry_w = gtk_entry_new_with_max_length( 16 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), entry_w, FALSE, FALSE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), entry_w );
	gtk_entry_set_text( GTK_ENTRY(entry_w), init_str );
	if (callback != NULL )
		gtk_signal_connect( GTK_OBJECT(entry_w), "activate",
		                    GTK_SIGNAL_FUNC(callback), callback_data );
	gtk_widget_show( entry_w );

	return entry_w;
}


/* Sets an entry to the width of a specified string */
void
set_entry_width( GtkWidget *entry_w, const char *span_str )
{
	GtkStyle *style;
	int width;

	style = gtk_widget_get_style( entry_w );
	width = gdk_string_width( style->font, span_str );
	gtk_widget_set_usize( entry_w, width, 0 );
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
	gtk_entry_select_region( GTK_ENTRY(entry_w), 0, GTK_ENTRY(entry_w)->text_length );
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
	gtk_signal_connect( GTK_OBJECT(button_w), "clicked",
	                    GTK_SIGNAL_FUNC(callback), callback_data );
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


/* Scrollable (but non-editing) text window
 * (I call it a "text area," as just "text" is ambiguous */
GtkWidget *
add_text_area( GtkWidget *parent_w, const char *content )
{
	GtkWidget *text_w;
	GtkWidget *hbox_w;
	GtkWidget *vscrollbar_w;

	/* Horizontal box for text area + vertical scrollbar */
	hbox_w = gtk_hbox_new( FALSE, 0 );
	if (GTK_IS_BOX(parent_w))
		gtk_box_pack_start( GTK_BOX(parent_w), hbox_w, TRUE, TRUE, 0 );
	else
		gtk_container_add( GTK_CONTAINER(parent_w), hbox_w );
	gtk_widget_show( hbox_w );
	/* Text [area] widget */
	text_w = gtk_text_new( NULL, NULL );
	/* Set properties suited for a text viewer */
	gtk_text_set_editable( GTK_TEXT(text_w), FALSE );
	gtk_text_set_word_wrap( GTK_TEXT(text_w), TRUE );
	gtk_box_pack_start( GTK_BOX(hbox_w), text_w, TRUE, TRUE, 0 );
	gtk_widget_show( text_w );
	/* Vertical scroll bar */
	vscrollbar_w = gtk_vscrollbar_new( GTK_TEXT(text_w)->vadj );
	gtk_box_pack_start( GTK_BOX(hbox_w), vscrollbar_w, FALSE, FALSE, 0 );
	gtk_widget_show( vscrollbar_w );

	/* Bring in the text */
	gtk_widget_realize( text_w );
	gtk_text_insert( GTK_TEXT(text_w), NULL, NULL, NULL, content, -1 );

	return text_w;
}


void
add_tooltip( GtkWidget *ambiguous_widget, const char *tip_text )
{
	static GtkTooltips *tooltips = NULL;

	if (tooltips == NULL) {
		tooltips = gtk_tooltips_new( );
		gtk_tooltips_set_delay( tooltips, 2000 );
	}
	gtk_tooltips_set_tip( tooltips, ambiguous_widget, tip_text, NULL );
}


/* Adds an XPM */
GtkWidget *
add_pixmap( GtkWidget *parent_w, GtkWidget *parent_window_w, char **xpm_data )
{
	GtkWidget *pixmap_w;
	GtkStyle *style;
	GdkPixmap *pixmap;
	GdkBitmap *mask;

	/* Realize the window to prevent a "NULL window" error */
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


/* Associates an XPM icon to a window */
void assign_icon( GtkWidget *window_w, char **xpm_data )
{
	GtkStyle *style;
	GdkPixmap *icon_pixmap;
	GdkBitmap *mask;

	gtk_widget_realize( window_w );
	style = gtk_widget_get_style( window_w );
	icon_pixmap = gdk_pixmap_create_from_xpm_d( window_w->window, &mask,
	              &style->bg[GTK_STATE_NORMAL], xpm_data );
	gdk_window_set_icon( window_w->window, NULL, icon_pixmap, mask );
}

/* end gtkwidgets.c */
