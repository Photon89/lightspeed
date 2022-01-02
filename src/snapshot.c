/* snapshot.c */

/* Image exporter */

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

#ifdef HAVE_LIBPNG
#include <png.h>
static int write_png( int message, const void *data );
#endif

#ifdef HAVE_LIBTIFF
#include <tiffio.h>
static int write_tiff( int message, const void *data );
#endif


/* [Other] forward declarations */
static int write_image( int format, int message, const void *data );


int
save_snapshot( int width, int height, const char *filename, int format )
{
	GdkGLContext *context;
	GdkGLPixmap *glpixmap;
	GdkVisual *visual;
	GdkPixmap *pixmap;
	GLubyte *scanline;
	float x,y,z;
	int visual_attributes[] = {
	    GDK_GL_RGBA,
	    GDK_GL_RED_SIZE, 8,
	    GDK_GL_GREEN_SIZE, 8,
	    GDK_GL_BLUE_SIZE, 8,
	    GDK_GL_DEPTH_SIZE, 1,
	    GDK_GL_NONE
	};
	int ogl_y;
	int err;
	int percent, prev_percent = -1;
	char comments[1024];
	char one_line[256];

	/* Initialize output camera */
	memcpy( &out_cam, usr_cams[0], sizeof(camera) );
	out_cam.width = width;
	out_cam.height = height;

	/* Create off-screen rendering buffer (i.e. pixmap) */
	visual = gdk_gl_choose_visual( visual_attributes );
	if (visual == NULL) {
		message_window( STR_DLG_Error, STR_MSG_no_ogl_visual );
		return -1;
	}
	pixmap = gdk_pixmap_new( NULL, width, height, visual->depth );
	if (pixmap == NULL) {
		sprintf( one_line, STR_MSG_no_render_buf_ARG, width, height );
		message_window( STR_DLG_Error, one_line );
		return -1;
	}
	glpixmap = gdk_gl_pixmap_new( visual, pixmap );
	context = gdk_gl_context_new( visual );
	err = !gdk_gl_pixmap_make_current( glpixmap, context );
	if (err) {
		message_window( STR_DLG_Error, STR_MSG_no_ogl_context );
		gdk_gl_context_unref( context );
		gdk_gl_pixmap_unref( glpixmap );
		gdk_pixmap_unref( pixmap );
		return -1;
	}

	/* Basic setup */
	ogl_initialize( NULL, NULL );

	/* For extra-high quality output */
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	/* GL_LINE_SMOOTH looks crummy */
	/* glEnable( GL_LINE_SMOOTH ); */

	/* Produce image description (comments) */
	if (object_mode == MODE_LATTICE) {
		sprintf( comments, "%dx%dx%d lattice, travelling at %s",
		         lattice_size_x, lattice_size_y, lattice_size_z,
		         velocity_string( velocity, TRUE ) );
	}
	else
		sprintf( comments, "Object travelling at %s", velocity_string( velocity, TRUE ) );
	sprintf( one_line, " in the +x direction.\n\n" );
	strcat( comments, one_line );
	x = out_cam.pos.x;
	y = out_cam.pos.y;
	z = out_cam.pos.z;
	sprintf( one_line, "Camera position: (%.3f, %.3f, %.3f)\n", x, y, z );
	strcat( comments, one_line );
	x = out_cam.target.x;
	y = out_cam.target.y;
	z = out_cam.target.z;
	sprintf( one_line, "Camera target..: (%.3f, %.3f, %.3f)\n", x, y, z );
	strcat( comments, one_line );
	sprintf( one_line, "Camera FOV.....: %.2f degrees", out_cam.fov );
	strcat( comments, one_line );
	if (width != height)
		sprintf( one_line, " horizontally\n\n" );
	else
		sprintf( one_line, "\n\n" );
	strcat( comments, one_line );
	sprintf( one_line, "Observed object is centered at the origin.\n" );
	strcat( comments, one_line );
	sprintf( one_line, "Coordinate system is right-handed with +z up.\n" );
	strcat( comments, one_line );
	sprintf( one_line, "Above units are in meters, where unspecified.\n\n" );
	strcat( comments, one_line );
	sprintf( one_line, "This image was produced by the Light Speed! relativistic simulator.\n" );
	strcat( comments, one_line );
/* TODO: Fix description...? it's inaccurate if an animation is ongoing, or
 * if any of the relativistic transforms are switched off */

	/* Initialize output file */
	err = FALSE;
	err = err || write_image( format, IMAGE_WIDTH, &width );
	err = err || write_image( format, IMAGE_HEIGHT, &height );
	err = err || write_image( format, IMAGE_COMMENTS, comments );
	err = err || write_image( format, IMAGE_FILENAME, filename );
	if (err) {
		message_window( STR_DLG_Error, STR_MSG_no_snapshot_output );
		return -1;
	}

	ogl_blank( 0, STR_MSG_Rendering_snapshot );
	gdk_gl_pixmap_make_current( glpixmap, context ); /* switch back */

	/* Draw! */
	ogl_draw( -1 );

	/* Read and export rows of pixels, one by one */
	scanline = xmalloc( width * 3 * sizeof(GLubyte) );
	glPixelStorei( GL_PACK_ALIGNMENT, 4 );
	for (ogl_y = height - 1; ogl_y >= 0; ogl_y--) {
		glReadPixels( 0, ogl_y, width, 1, GL_RGB, GL_UNSIGNED_BYTE, scanline );
		err = write_image( format, IMAGE_PIXELROW, scanline );
		if (err)
			break;
		percent = (100 * (height - ogl_y)) / height;
		if (percent != prev_percent) {
			sprintf( one_line, STR_MSG_Saving_snapshot_ARG, percent );
			ogl_blank( 0, one_line );
			/* switch back to pixmap GL context */
			gdk_gl_pixmap_make_current( glpixmap, context );
			prev_percent = percent;
		}
	}
	xfree( scanline );

	/* Close up output file */
	write_image( format, IMAGE_COMPLETE, NULL );

	gdk_gl_context_unref( context );
	gdk_gl_pixmap_unref( glpixmap );
	gdk_pixmap_unref( pixmap );

	return err;
}


/* Dispatcher for the various file format handlers */
static int
write_image( int format, int message, const void *data )
{
	switch (format) {
#ifdef HAVE_LIBPNG
	case IMAGE_FORMAT_PNG:
		return write_png( message, data );
#endif

#ifdef HAVE_LIBTIFF
	case IMAGE_FORMAT_TIFF:
		return write_tiff( message, data );
#endif

	default:
		break;
	}

	return -1;
}


/* PNG output */
#ifdef HAVE_LIBPNG
static int
write_png( int message, const void *data )
{
	static FILE *png_fp;
	static png_structp png_write_s;
	static png_infop png_info_s;
	static png_text comments[2];
	static int width, height;
	GLubyte *pixelrow;
	char *filename;
	char *text;

	switch (message) {
	case IMAGE_WIDTH:
		width = *((int *)data);
		break;

	case IMAGE_HEIGHT:
		height = *((int *)data);
		break;

	case IMAGE_COMMENTS:
		text = (char *)data;
		comments[0].key = "Title";
		comments[0].text = "Light Speed! snapshot";
		comments[0].compression = PNG_TEXT_COMPRESSION_NONE;
		comments[1].key = "Description";
		comments[1].text = text;
		comments[1].compression = PNG_TEXT_COMPRESSION_NONE;
		break;

	case IMAGE_FILENAME:
		filename = (char *)data;
		png_fp = fopen( filename, "wb" );
		if (png_fp == NULL)
			return -1;
		png_write_s = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		png_info_s = png_create_info_struct( png_write_s );
		if (setjmp( png_write_s->jmpbuf )) {
			/* Error writing file */
			png_destroy_write_struct( &png_write_s, &png_info_s );
			fclose( png_fp );
			return -1;
		}
		png_init_io( png_write_s, png_fp );
		png_set_IHDR( png_write_s, png_info_s, width, height, 8,
		              PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		              PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
		/* png_set_gAMA( png_write_s, png_info_s, dgamma ); */
		png_set_compression_level( png_write_s, Z_BEST_COMPRESSION );
		png_set_text( png_write_s, png_info_s, comments, 2 );
		png_write_info( png_write_s, png_info_s );
		break;

	case IMAGE_PIXELROW:
		pixelrow = (GLubyte *)data;
		png_write_row( png_write_s, pixelrow );
		break;

	case IMAGE_COMPLETE:
		png_write_end( png_write_s, NULL );
		png_destroy_write_struct( &png_write_s, &png_info_s );
		fclose( png_fp );
		break;
	}

	return 0;
}
#endif /* HAVE_LIBPNG */


/* TIFF output */
#ifdef HAVE_LIBTIFF
static int
write_tiff( int message, const void *data )
{
	static TIFF *tiff;
	static FILE *tiff_fp;
	static int width, height;
	static char *text = NULL;
	static int img_y = 0;
	GLubyte *scanline;
	int rows_per_strip;
	int rv;
	char *filename;

	switch (message) {
	case IMAGE_WIDTH:
		width = *((int *)data);
		break;

	case IMAGE_HEIGHT:
		height = *((int *)data);
		break;

	case IMAGE_COMMENTS:
		text = (char *)data;
		break;

	case IMAGE_FILENAME:
		filename = (char *)data;
		tiff_fp = fopen( filename, "w" );
		if (tiff_fp == NULL)
			return -1;
		tiff = TIFFFdOpen( fileno( tiff_fp ), "output file", "w" );
		if (tiff == NULL)
			return -1;
		TIFFSetField( tiff, TIFFTAG_IMAGEWIDTH, width );
		TIFFSetField( tiff, TIFFTAG_IMAGELENGTH, height );
		TIFFSetField( tiff, TIFFTAG_BITSPERSAMPLE, 8 );
		TIFFSetField( tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW );
		TIFFSetField( tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( tiff, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB );
		TIFFSetField( tiff, TIFFTAG_DOCUMENTNAME, "Light Speed! snapshot" );
		TIFFSetField( tiff, TIFFTAG_IMAGEDESCRIPTION, text );
		TIFFSetField( tiff, TIFFTAG_SAMPLESPERPIXEL, 3 );
		rows_per_strip = MAX(1, 8192 / (3 * width));
		TIFFSetField( tiff, TIFFTAG_ROWSPERSTRIP, rows_per_strip );
		TIFFSetField( tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
		break;

	case IMAGE_PIXELROW:
		scanline = (GLubyte *)data;
		rv = TIFFWriteScanline( tiff, scanline, img_y++, 0 );
		if (rv < 0)
			return -1;
		break;

	case IMAGE_COMPLETE:
		TIFFFlushData( tiff );
		TIFFClose( tiff );
		fclose( tiff_fp );
		break;
	}

	return 0;
}
#endif /* HAVE_LIBTIFF */

/* end snapshot.c */
