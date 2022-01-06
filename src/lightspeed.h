/* lightspeed.h */

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


/**** Includes ***********************************************************/

/* Autoconf stuff */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* The usual headers */
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <math.h>
#include <string.h>
#include <sys/time.h>

/* OpenGL */
#include <GL/gl.h>
#include <GL/glx.h>

/* GTK+ */
#include <gtk/gtk.h>
#include <gtkgl-2.0/gtkgl/gtkglarea.h>
#include <gtk/gtktext.h>
#include <gtk/gtkaccelgroup.h>

/* Compile-time settings */
#include "settings.h"

/* Optional memory allocation tracking */
#ifdef WITH_TRACKMEM
#include "trackmem.h"
#endif


/**** Constants/macros ***************************************************/

/* We need >=1 image library to save snapshots */
//#if (HAVE_LIBPNG || HAVE_LIBTIFF)
/* Snapshot export crashes, so deactivate it completely for now */
#if (FALSE)
#define CAN_SAVE_SNAPSHOT
#else
#undef CAN_SAVE_SNAPSHOT
#endif

/* C = speed of light in meters/sec */
#define C			299792458.0
/* MAX_VELOCITY < C */
#define MAX_VELOCITY		(C - 1.0)
/* MIN_VELOCITY > 0 */
#define MIN_VELOCITY		1.0
#define PI			3.14159265358979323846264338327
#define MAGIC_NUMBER		1.618033989

/* EM wavelengths (in nanometers) */
#define LAMBDA_UV		340.0
#define LAMBDA_BLUE		460.0
#define LAMBDA_GREEN		520.0
#define LAMBDA_RED		700.0
#define LAMBDA_IR		1000.0

/* Relative perceptual strengths of the RGB components */
#define RED_STRENGTH		0.3
#define GREEN_STRENGTH		0.59
#define BLUE_STRENGTH		0.11

/* Useful */
#define DEG(angleRad)		((angleRad) * 180 / PI)
#define RAD(angle360)		((angle360) * PI / 180)
#define SQR(x)			((x) * (x))
#define SIGN(x)			(((x) >= 0) ? 1.0 : -1.0)
#define C2			(C * C)
/* a value counterpart to NULL */
#define NIL			0

/* Macro for message passing via pointer */
#define MESG_(m)		((int *)&mesg_vals[m])

/* Message constants: */
enum {
	/* Objects modes (for global variable "objects_mode") */
	MODE_LATTICE,
	MODE_USER_GEOMETRY,

	/* transition types (trans_var->type field) */
	TRANS_LINEAR,
	TRANS_QTR_SIN,
	TRANS_RAMP,
	TRANS_SIGMOID,
	TRANS_STOP,

	/* graphics mode control by menu_Camera_GraphicsMode_select( ) */
	OGL_WIREFRAME_MODE,
	OGL_SHADED_MODE,

	/* auxiliary object control by aux_objs( ) */
	AUXOBJS_DRAW,
	AUXOBJS_SET_AXES,
	AUXOBJS_SET_BBOX,
	AUXOBJS_SET_GRID,

	/* relativistic distortion control by warp( ) */
	WARP_DISTORT,
	WARP_LORENTZ_CONTRACTION,
	WARP_OPTICAL_DEFORMATION,
	WARP_DOPPLER_SHIFT,
	WARP_HEADLIGHT_EFFECT,
	/* time and animation control by warp_time( ) */
	WARP_UPDATE_TIME_T,
	WARP_BEGIN_ANIM,
	WARP_STOP_ANIM,

	/* performance profiling control by profile( ) */
	PROFILE_START_ITERATION,
	PROFILE_FRAME_BEGIN,
	PROFILE_FRAME_DONE,
	PROFILE_FRAMERATE_RESET,
	PROFILE_WARP_BEGIN,
	PROFILE_WARP_DONE,
	PROFILE_OGLDRAW_BEGIN,
	PROFILE_OGLDRAW_DONE,
	PROFILE_IDLE,
	PROFILE_SHOW_STATS,

	/* info display control by info_display( ) */
	INFODISP_DRAW,
	INFODISP_ACTIVE,
	INFODISP_SHOW_VELOCITY,
	INFODISP_SHOW_TIME_T,
	INFODISP_SHOW_GAMMA,
	INFODISP_SHOW_FRAMERATE,
	INFODISP_UPDATE,

	/* position codes for ogl_draw_string( ) */
	POS_TOP_LEFT,
	POS_TOP_RIGHT,
	POS_BOTTOM_LEFT,
	POS_BOTTOM_RIGHT,
	POS_CENTER,

	/* dialog control */
	DIALOG_OPEN,
	DIALOG_CLOSE,
	DIALOG_OK,
	DIALOG_OK_CONFIRM,

	/* for camera_calc_xyz( ) */
	CAM_POSITION,
	CAM_TARGET,

	/* for velocity_input( ) et. al. */
	VALUE_COMMITTED,
	VALUE_CHANGED,
	UNITS_CHANGED,

	/* generic OO */
	ACTIVATE,
	DEACTIVATE,
	ITERATION,
	INITIALIZE,
	RESET,
	QUERY,

	/* for rotate_xyz( ), etc. */
	X_ALIGN,
	Y_ALIGN,
	Z_ALIGN,
	Z_ROTATE_CW,
	Z_ROTATE_CCW,

	/* for image handling, esp. save_snapshot( ) */
	IMAGE_FORMAT,
	IMAGE_FORMAT_PNG,
	IMAGE_FORMAT_TIFF,
	IMAGE_WIDTH,
	IMAGE_HEIGHT,
	IMAGE_COMMENTS,
	IMAGE_FILENAME,
	IMAGE_PIXELROW,
	IMAGE_COMPLETE
};


/**** Data structures ****************************************************/

/* Command-line option description */
struct option_desc {
	char opt;	/* Short option */
	char *lopt;	/* Equivalent long option (GNU-style) */
	char *desc;	/* Description string */
};


/* Transitory variable state container
 * Keeps track of a float or double var as it goes from initial to final value
 * (see transition_engine( ) for the details) */
typedef struct trans_var_struct trans_var;
struct trans_var_struct {
	void *var;
	int is_double : 1;
	int looping : 1;
	int type;
	double start_t;
	double end_t;
	int cam_id;	/* = -1 if transition affects all camera views */
	double initial;
	double final;
	trans_var *next; /* transition queue is a linked list.... */
};


/* Absolute coordinate extents info */
typedef struct extents_struct extents;
struct extents_struct {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
	float avg; /* = ((xmax-xmin)+(ymax-ymin)+(zmax-zmin))/3 */
};


/* RGB color definition */
typedef struct rgb_color_struct rgb_color;
struct rgb_color_struct {
	float r;
	float g;
	float b;
};


/* 3D point/vector definition */
typedef struct point_struct point;
struct point_struct {
	float x;
	float y;
	float z;
};


/* OpenGL point (color+normal+location) definition
 * (for use in interleaved arrays) */
typedef struct ogl_point_struct ogl_point;
struct ogl_point_struct {
	/* RGBA color (C4F) */
	float r;
	float g;
	float b;
	float a;
	/* Normal direction (N3F) */
	float nx;
	float ny;
	float nz;
	/* Vertex location (V3F) */
	float x;
	float y;
	float z;
};


/* OpenGL object definition */
typedef struct ogl_object_struct ogl_object;
struct ogl_object_struct {
	int		type;
	int		num_vertices;
        point		*vertices0;	/* Original, unwarped geometry */
	point		*normals0;
	rgb_color	color0;
	ogl_point	*iarrays;	/* C4F+N3F+V3F interleaved arrays */
	int		num_indices;
	unsigned int	*indices;
	int		pre_dlist;	/* OGL display list executed before... */
	int		post_dlist;	/* ...and after drawing the object */
};


/* Camera state container */
typedef struct camera_struct camera;
struct camera_struct {
	float phi;		/* heading (in degres) */
	float theta;		/* elevation (in degrees) */
	float distance;		/* distance to target */
	point target;
	point pos;		/* Derived from phi/theta/distance/target */
	float fov;		/* field of view in degrees */
	int width, height;	/* of associated viewport */
	float near_clip;	/* Clipping plane distances */
	float far_clip;
	GtkWidget *window_w;	/* Associated window widget */
	GtkWidget *ogl_w;	/* Associated GL widget (viewport) */
	int redraw : 1;		/* Flag: does viewport want a redraw? */
};


/**** Global variables ***************************************************/

/* These are all in globals.c */
extern const int mesg_vals[];
extern const int num_unit_systems;
extern const char *unit_suffixes[];
extern const double unit_conv_factors[];
extern const int unit_num_decimals[];
extern int cur_unit_system;
extern const int num_background_colors;
extern const rgb_color bkgd_colors[];
extern rgb_color background;
extern const int num_font_sizes;
extern const char *sys_font_names[];
extern const int sys_font_sizes[];
extern const int micro_glyph;
extern const int num_stock_lenses;
extern const float stock_lenses[];
extern const char *image_format_exts[];
extern double velocity;
extern double cur_time_t;
extern float vehicle_real_x;
extern int object_mode;
extern int lattice_size_x;
extern int lattice_size_y;
extern int lattice_size_z;
extern float framerate;
extern ogl_object **vehicle_objs;
extern int num_vehicle_objs;
extern extents vehicle_extents;
extern extents world_extents;
extern camera **usr_cams;
extern int num_cams;
extern int cur_cam;
extern camera out_cam;
extern int advanced_interface;
extern int dgamma_correct;
extern float dgamma_lut[];
extern float mouse_sens;

/* Language-specific strings */
#include "lstrings.h"


/**** Forward declarations ***********************************************/

/* animation.c */
void queue_redraw( int cam_id );
int update( int message );
void profile( int message );
void transition( void *var, int is_double, int trans_type, double duration, double final, int cam_id );
void animate( void *var, int is_double, int trans_type, double duration, double initial, double final, int cam_id );
void break_transition( void *var );

/* auxobjects.c */
void auxiliary_objects( int message1, int message2 );

/* camera.c */
camera *new_camera( void);
void kill_camera( int cam_id );
void camera_reset( int cam_id );
int assoc_cam_id( void *widget );
int camera_set_current( GtkWidget *widget, GdkEventFocus *ev_focus, void *nothing );
float camera_calc_fov( float lens_length );
float camera_calc_lens_length( float fov );
void camera_calc_xyz( int point_type, camera *cam );
void camera_make_target( camera *cam );
int camera_move( GtkWidget *widget, GdkEventAny *event, void *nothing );

/* command.c */
int command( const char *input );

/* geometry.c */
ogl_object *alloc_ogl_object( int num_vertices, int num_indices );
int calc_ogl_object_memusage( int num_vertices, int num_indices );
void free_ogl_object( ogl_object *obj );
void clear_all_objects( void);
void rotate_all_objects( int direction );
void rotate_xyz( int action, float *x, float *y, float *z, float x0, float y0, float z0 );
point *calc_tri_normal( point *a, point *b, point *c );
point *calc_tri_centroid( point *a, point *b, point *c );
float calc_tri_area( point *a, point *b, point *c );
void show_geometry_stats( void);

#ifdef WITH_SRS_EXPORTER
int export_srs( const char *filename, int width, int height, int stereo_view, int visible_faces_only );
void write_srs_mesh( FILE *srs, point *cam_pos, int visible_faces_only );
void write_srs_camera( FILE *srs, camera *cam, float t );
void write_srs_sphere( FILE *srs, point *center, float radius );
void write_srs_cylinder( FILE *srs, point *p1, point *p2, float radius );
void write_srs_smooth_triangle( FILE *srs, point **vertices, point **normals );
void write_srs_pigment( FILE *srs, rgb_color *color );
void convert_to_srs_cs( point *p_srs, point *p_ls );
#endif /* WITH_SRS_EXPORTER */

/* gtkwidgets.c */
GtkWidget *add_gl_area( GtkWidget *parent_box_w );
GtkWidget *make_dialog_window( const char *title, void *callback_close );
GtkWidget *message_window( const char *title, const char *message_text );
GtkWidget *confirmation_dialog( const char *title, const char *message_text, void *callback );
GtkWidget *make_filesel_window( const char *title, const char *init_filename, int show_fileops, void *callback_handler );
GtkWidget *add_menu( GtkWidget *parent_menu_w, const char *label );
GtkWidget *add_menu_item( GtkWidget *menu_w, const char *label, void *callback, void *callback_data );
GtkWidget *add_check_menu_item( GtkWidget *menu_w, const char *label, int init_state, void *callback, void *callback_data );
void begin_radio_menu_group( int init_selected );
GtkWidget *add_radio_menu_item( GtkWidget *menu_w, const char *label, void *callback, void *callback_data );
GtkWidget *option_menu_item( const char *label, void *callback, void *callback_data );
GtkWidget *add_option_menu( GtkWidget *widget );
void add_separator( GtkWidget *parent_w );
void keybind( GtkWidget *widget, char *keys );
GtkWidget *add_hbox( GtkWidget *parent_w, int homog, int spacing );
GtkWidget *add_vbox( GtkWidget *parent_w, int homog, int spacing );
GtkWidget *add_frame( GtkWidget *parent_w, const char *title );
GtkWidget *add_hscale( GtkWidget *parent_w, GtkObject *adjustment );
GtkWidget *add_vscale( GtkWidget *parent_w, GtkObject *adjustment );
GtkWidget *add_spin_button( GtkWidget *parent_w, GtkObject *adjustment );
GtkWidget *add_check_button( GtkWidget *parent_w, const char *label, int init_state, void *callback, void *callback_data );
GtkWidget *add_entry( GtkWidget *parent_w, const char *init_str, void *callback, void *callback_data );
void set_entry_width( GtkWidget *entry_w, const char *span_str );
void set_entry_text( GtkWidget *entry_w, const char *entry_text );
char *read_entry( GtkWidget *entry_w );
void highlight_entry( GtkWidget *entry_w );
GtkWidget *add_button( GtkWidget *parent_w, const char *label, void *callback, void *callback_data );
GtkWidget *add_label( GtkWidget *parent_w, const char *label_text );
GtkWidget *add_pixmap( GtkWidget *parent_w, GtkWidget *parent_window_w, char **xpm_data );
void assign_icon( GtkWidget *window_w, char **xpm_data );

/* importobjs.c */
int import_objects( const char *filename );

/* infodisp.c */
void info_display( int message1, int message2 );

/* lattice.c */
void make_lattice( int size_x, int size_y, int size_z, int smoothness );
#ifdef WITH_SRS_EXPORTER
void write_srs_lattice( FILE *srs );
#endif /* WITH_SRS_EXPORTER */

/* mainwindow.c */
void main_window( void );
void add_Camera_menu( GtkWidget *menu_bar_w, GtkWidget *window_w );

/* menu_cbs.c */
void velocity_input( GtkWidget *widget, const int *message );
#ifdef VELOCITY_SLIDER
void velocity_slider( GtkWidget *widget, const int *message );
#endif /* VELOCITY_SLIDER */
void dialog_File_NewLattice( GtkWidget *widget, const int *message );
#ifdef WITH_OBJECT_IMPORTER
void dialog_File_ImportObject( GtkWidget *widget, const int *message );
#endif /* WITH_OBJECT_IMPORTER */
#ifdef CAN_SAVE_SNAPSHOT
void dialog_File_SaveSnapshot( GtkWidget *widget, const int *message );
#endif /* CAN_SAVE_SNAPSHOT */
#ifdef WITH_SRS_EXPORTER
void dialog_File_ExportSRS( GtkWidget *widget, const int *message );
#endif /* WITH_SRS_EXPORTER */
void menu_Objects_toggles( GtkWidget *widget, const int *message );
void dialog_Objects_Animation( GtkWidget *widget, const int *message );
void menu_Warp_toggles( GtkWidget *widget, const int *message );
void menu_Camera_Lens_select( GtkWidget *widget, const float *new_lens_length );
void dialog_Camera_Lens_Custom( GtkWidget *widget, const int *message );
void dialog_Camera_Position( GtkWidget *widget, const int *message );
void menu_Camera_ResetView( GtkWidget *widget, const int *cam_id );
void menu_Camera_InfoDisplay_toggles( GtkWidget *widget, const int *message );
void menu_Camera_Background_select( GtkWidget *widget, const int *color_id );
void menu_Camera_GraphicsMode_select( GtkWidget *widget, const int *message );
void menu_Camera_Spawn( GtkWidget *widget, void *dummy );
void menu_Camera_Close( GtkWidget *widget_to_kill, void *dummy );
void dialog_Help_Overview( GtkWidget *widget, int *message );
void dialog_Help_Controls( GtkWidget *widget, int *message );
void dialog_Help_About( GtkWidget *widget, int *message );

/* misc.c */
void *xmalloc( size_t size );
void *xrealloc( void *block, size_t size );
char *xstrdup( const char *str );
void xfree( void *block );
int file_exists( const char *filename );
char *file_basename( const char *name, const char *suffix );
char *swap_filename_ext( const char *filename, const char *old_ext, const char *new_ext );
double read_system_clock( void);
void set_cursor_glyph( int glyph );
char *velocity_string( double v_ms, int fancy );
char *add_commas( const char *num_str );
char *clean_number( const char *num_str );
char *time_string( void );
void calc_dgamma_lut( float dgamma );
void crash( char *error_mesg );
void ss( void );
int *NOP( void );

/* ogl.c */
void ogl_initialize( GtkWidget *ogl_w, void *nothing );
int ogl_resize( GtkWidget *ogl_w, GdkEventConfigure *ev_config, void *nothing );
int ogl_refresh( GtkWidget *ogl_w, GdkEventExpose *ev_expose, void *nothing );
void ogl_draw( int cam_id );
void ogl_draw_string( const void *data, int message, int size );
void ogl_blank( int cam_id, const char *blank_message );
GtkWidget *ogl_make_widget( void);

/* snapshot.c */
int save_snapshot( int width, int height, const char *filename, int format );

/* warp.c */
int warp( int message, void *data );
void warp_point( point *vertex, point *normal, point *cam_pos );
void warp_time( float x0, float x1, double value, int message );
double lorentz_factor( double v );

/* end lightspeed.h */
