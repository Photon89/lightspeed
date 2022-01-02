/* misc.c */

/* Random stuff */

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


/* A kinder, gentler malloc( ) */
void *
xmalloc( size_t size )
{
	void *block;
#ifdef DEBUG
	int i;
#endif

	block = malloc( size );
	if (block == NULL)
		crash( "Insufficient memory" );
#ifdef WITH_TRACKMEM
	trackmem_malloc( block, size );
#endif
#ifdef DEBUG
	/* Fill memory block with random junk */
	for (i = 0; i < size; i++)
		((unsigned char *)block)[i] = (unsigned char)(rand( ) % 256);
#endif

	return block;
}


/* ditto for realloc( ) */
void *
xrealloc( void *block, size_t size )
{
	void *block2;

	block2 = realloc( block, size );
	if (block2 == NULL)
		crash( "Insufficient memory" );
#ifdef WITH_TRACKMEM
	trackmem_realloc( block, block2, size );
#endif

	return block2;
}


char *
xstrdup( const char *str )
{
	char *new_str;

	new_str = strdup( str );
	if (new_str == NULL)
		crash( "Insufficient memory" );
#ifdef WITH_TRACKMEM
	trackmem_strdup( new_str );
#endif

	return new_str;
}


void
xfree( void *block )
{
#ifdef WITH_TRACKMEM
	trackmem_free( block );
#endif
	free( block );
}


/* Checks if a file already exists */
int
file_exists( const char *filename )
{
	FILE *fp;

	fp = fopen( filename, "r" );
	if (fp == NULL)
		return FALSE;
	fclose( fp );
	return TRUE;
}


/* This is a functional equivalent of the UNIX "basename" command */
char *
file_basename( const char *name, const char *suffix )
{
	static char basename_buf[256];
	int blen, slen;
	char *base;

	base = strrchr( name, '/' );
	if (base == NULL)
		base = (char *)name;
	else
		base = &base[1];
	strcpy( basename_buf, base );

	if (suffix != NULL) {
		blen = strlen( basename_buf );
		slen = strlen( suffix );
		if (!strcasecmp( &basename_buf[blen - slen], suffix ))
			basename_buf[blen - slen] = '\0';
	}

	return basename_buf;
}


/* Swaps the extension of a filename to new_ext if it already ends in old_ext
 * (a NULL for old_exts means any period-demarcated extension) */
char *
swap_filename_ext( const char *filename, const char *old_ext, const char *new_ext )
{
	static char filename_buf[256];
	char *ext_begin;
	int i;

	strcpy( filename_buf, filename );
	if (old_ext != NULL) {
		i = strlen( filename_buf ) - strlen( old_ext );
		ext_begin = &filename_buf[i];
		if (!strcasecmp( ext_begin, old_ext ))
			strcpy( ext_begin, new_ext );
	}
	else {
		ext_begin = strrchr( filename_buf, '.' );
		if (ext_begin != NULL)
			strcpy( ext_begin, new_ext );
	}

	return filename_buf;
}


/* Wrapper for gettimeofday( ) */
double
read_system_clock( void )
{
	struct timeval tv;
	double t;

	gettimeofday( &tv, NULL );
	t = (double)tv.tv_sec;
	t += (double)tv.tv_usec / 1E6;

	return t;
}


/* Change cursor appearance */
void
set_cursor_glyph( int glyph )
{
	static GdkCursor *cursor;
	static int prev_glyph = -1;

	if (prev_glyph != -1)
		gdk_cursor_destroy( cursor );
	cursor = gdk_cursor_new( glyph );
	gdk_window_set_cursor( usr_cams[cur_cam]->ogl_w->window, cursor );
	prev_glyph = glyph;
}


/* Returns the specified velocity (in m/s) formatted to current unit system
 * "fancy" parameter adds commas and units suffix when TRUE */
char *
velocity_string( double v_ms, int fancy )
{
	static char v_str[32];
	const char *suffix;
	double v_conv;
	int num_decimals;
	char num_str[32];

	v_conv = v_ms / unit_conv_factors[cur_unit_system];
	num_decimals = unit_num_decimals[cur_unit_system];
	suffix = unit_suffixes[cur_unit_system];
	if (fancy) {
		sprintf( num_str, "%.*f", num_decimals, v_conv );
		sprintf( v_str, "%s %s", add_commas( num_str ), suffix );
	}
	else
		sprintf( v_str, "%.*f", num_decimals, v_conv );

	return v_str;
}


/* Adds commas to a number string */
char *
add_commas( const char *num_str )
{
	static char new_str[64];
	int len;
	int decimal_reached = FALSE;
	int group_digits = 0;
	int n = 64;
	int i;
	char c;

	if (strchr( num_str, '.' ) == NULL)
		decimal_reached = TRUE; /* no decimal point */
	len = strlen( num_str );
	new_str[--n] = '\0';

	for (i = len - 1; i >= 0; i--) {
		c = num_str[i];
		if (group_digits == 3) {
			new_str[--n] = ',';
			group_digits = 0;
		}
		new_str[--n] = c;
		if (decimal_reached)
			++group_digits;
		if (c == '.')
			decimal_reached = TRUE;
	}

	return &new_str[n];
}


/* Removes commas and other junk from a number string */
char *
clean_number( const char *num_str )
{
	static char new_str[64];
	int start, len;
	int n = 64;
	int decimal_reached = FALSE;
	int i;
	char c;

	start = strcspn( num_str, "123456789." );
	len = strlen( num_str );
	new_str[--n] = '\0';

	for (i = len - 1; i >= start; i--) {
		c = num_str[i];
		if (c == '.') {
			/* Allow only one occurrence of decimal point */
			if (decimal_reached)
				continue;
			else
				decimal_reached = TRUE;
		}
		else if ((c < '0') || (c > '9'))
			continue;
		new_str[--n] = c;
	}

	return &new_str[n];
}


/* Returns current time, formatted nicely and to an appropriate precision */
char *
time_string( void )
{
	static char t_str[64];
	float abs_t, t_usec;

	abs_t = ABS(cur_time_t);
	if (abs_t >= 1E-2)
		sprintf( t_str, "%+.3f ", cur_time_t );
	else if (abs_t >= 1E-4)
		sprintf( t_str, "%+.5f ", cur_time_t );
	else {
		t_usec = 1E6 * cur_time_t; /* microseconds */
		sprintf( t_str, "%+.3f %c", t_usec, micro_glyph );
	}

	return t_str;
}


/* Generates display gamma correction color component lookup table */
void
calc_dgamma_lut( float dgamma )
{
	float k, x;
	int i;

	k = 1.0 / dgamma;
	for (i = 0; i <= LUT_RES; i++) {
		x = (float)i / LUT_RES;
		dgamma_lut[i] = pow( x, k );
	}
}


/* Hopefully this never gets called */
void
crash( char *error_mesg )
{
	fprintf( stderr, "CRASH: %s\n", error_mesg );
	fflush( stderr );
	exit( -1 );
}


/* ^_^ */
void
ss( void )
{
	const float r1 = 0.0375749;
	const float r2 = 0.112725;
	const float r3 = 0.187875;
	const float r4 = 0.42485;
	const float r5 = 0.5;
	const float r6 = 0.518787;
	const float a1 = 20.7181;
	const float a2 = 25.7579;
	const float a3 = 26.8014;
	const float a4 = 45.0353;
	const float xd = 0.1161;
	const float yd = 0.1127;
	camera *cam;
	float x, y;
	float a;
	int i;

	cam = usr_cams[0];

	gtk_gl_area_make_current( GTK_GL_AREA(cam->ogl_w) );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	x = (float)cam->width;
	y = (float)cam->height;
	if ((x / y) > 1.0 ) {
		x = 0.6 * x / y;
		y = 0.6;
	}
	else {
		y = 0.6 * y / x;
		x = 0.6;
	}
	glOrtho( - x, x, - y, y, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	glBegin( GL_QUAD_STRIP );
	glColor3f( 1.0, 1.0, 1.0 );
	glVertex2f( - xd, yd + r1 );
	glVertex2f( - xd, yd + r2 );
	for (i = 0; i <= 32; i++) {
		a = RAD(90.0 + 180.0 * (float)i / 32.0);
		x = cos(a);
		y = sin(a);
		glVertex2f( - xd + r1 * x, yd + r1 * y );
		glVertex2f( - xd + r2 * x, yd + r2 * y );
	}
	for (i = 0; i <= 64; i++) {
		a = RAD(90.0 - 180.0 * (float)i / 64.0);
		x = cos(a);
		y = sin(a);
		glVertex2f( xd + r3 * x, - yd + r3 * y );
		glVertex2f( xd + r2 * x, - yd + r2 * y );
	}
	for (i = 0; i <= 128; i++) {
		a = RAD(180.0 + a4 + (a1 - a4 + 180.0) * (float)i / 128.0);
		glVertex2f( r4 * cos(a), r4 * sin(a) );
		a = RAD(180.0 + a2 + 180.0 * (float)i / 128.0);
		glVertex2f( r6 * cos(a), r6 * sin(a) );
	}
	glVertex2f( - xd, yd + r1 );
	glVertex2f( - xd, yd + r2 );
	glEnd( );

	glBegin( GL_QUAD_STRIP );
	glColor3f( 1.0, 1.0, 1.0 );
	for (i = 0; i <= 128; i++) {
		a = RAD( a3 + 180.0 * (float)i / 128.0 );
		glVertex2f( r5 * cos(a), r5 * sin(a) );
		a = RAD( a2 + 180.0 * (float)i / 128.0 );
		glVertex2f( r6 * cos(a), r6 * sin(a) );
	}
	glEnd( );

	glBegin( GL_TRIANGLE_FAN );
	glColor3f( 1.0, 1.0, 1.0 );
	glVertex2f( - xd, yd + r3 );
	for (i = 0; i <= 32; i ++) {
		a = RAD( a4 + (90.0 - a4) * (float)i / 32.0 );
		glVertex2f( r4 * cos(a), r4 * sin(a) );
	}
	glEnd( );

	glBegin( GL_QUAD_STRIP );
	glColor3f( 1.0, 1.0, 1.0 );
	for (i = 0; i <= 128; i++) {
		a = RAD( 90.0 + 180.0 * (float)i / 128.0 );
		glVertex2f( - xd + r3 * cos(a), yd + r3 * sin(a) );
		a = RAD( 90.0 + (a1 + 90.0) * (float)i / 128.0 );
		glVertex2f( r4 * cos(a), r4 * sin(a) );
	}
	glVertex2f( xd, - yd + r1 );
	glVertex2f( xd, - yd - r1 );
	glEnd( );

	glBegin( GL_TRIANGLE_FAN );
	glColor3f( 1.0, 1.0, 1.0 );
	for (i = 0; i <= 16; i++) {
		a = RAD( -90.0 + 180.0 * (float)i / 16.0 );
		glVertex2f( xd + r1 * cos(a), - yd + r1 * sin(a) );
	}
	glEnd( );

	glHint( GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE );
	glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	GTKGL_TEMP_endgl( GTK_GL_AREA(cam->ogl_w) );
	gtk_gl_area_swapbuffers( GTK_GL_AREA(cam->ogl_w) );
}


/* NOP is referenced in compat.h
 * When no one in the world is using GTK 1.0, I'll get rid of this */
int *NOP( void )
{
	while (FALSE);
	return NULL;
}

/* end misc.c */
