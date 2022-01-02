/* compat.h */

/* Compatibility hack for GTK+/glib/GtkGLArea 1.0 */

/* I'll spare you the boilerplate just this once :-) */


#ifndef GTK_HAVE_FEATURES_1_1_0

/*	---- GTK+ 1.1 name ----		---- GTK+ 1.0 name ---- */
#define gtk_container_set_border_width	gtk_container_border_width
#define gtk_window_set_position		gtk_window_position
#define gtk_toggle_button_set_active	gtk_toggle_button_set_state
#define gtk_check_menu_item_set_active	gtk_check_menu_item_set_state
#define gtk_label_set_text		gtk_label_set

/* GtkGLArea prior to 1.? uses a slightly different API */
#define gtk_gl_area_make_current	gtk_gl_area_begingl
#define GTKGL_TEMP_endgl		gtk_gl_area_endgl

/* Neutralize accelerator code in keybind( ) */
#define GtkAccelGroup			int
#define gtk_accel_group_new		NOP
#define gtk_accel_group_attach(a,b)	NOP( )
#define gtk_widget_add_accelerator(a,b,c,d,e,f) NOP( )

#else /* not GTK_HAVE_FEATURES_1_1_0 */

#define GTKGL_TEMP_endgl(a)		NOP( )

#endif /* not GTK_HAVE_FEATURES_1_1_0 */

/* end compat.h */
