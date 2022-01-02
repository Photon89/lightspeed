/* lightspeed.c */

/****************************** Light Speed! ******************************

    Interactive visualization of relativistic distortion phenomena

    Written by Daniel Richard G., July 1998 - April 1999
    <skunk@mit.edu>

    Built on OpenGL, GTK+, and the GtkGLArea widget.

**************************************************************************/

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


static void
cmdline_help( const char *execname )
{
	int i;
	char opt, *lopt, *desc;

	printf( "\n" );
	printf( "%s %s\n", STR_Light_Speed, VERSION );
	printf( STR_copyright_ARG, 1999, "Daniel Richard G. <skunk@mit.edu>" );
	printf( "\n\n" );

	printf( STR_CLI_usage_ARG, execname );
	printf( "\n" );
	for (i = 0; i < 4; i++) {
		opt = STRS_CLI_options[i].opt;
		lopt = STRS_CLI_options[i].lopt;
		desc = STRS_CLI_options[i].desc;
#ifdef HAVE_GETOPT_LONG
		if (opt != '\0')
			printf( "  -%c --%-9s %s\n", opt, lopt, desc );
		else
			printf( "  %-14s %s\n", lopt, desc );
#else
		if (opt != '\0')
			printf( "  -%-7c %s\n", opt, desc );
		else
			printf( "  %-8s %s\n", lopt, desc );
#endif /* not HAVE_GETOPT_LONG */
	}
	printf( "\n" );
	fflush( stdout );
}


int
main( int argc, char **argv )
{
#ifdef HAVE_GETOPT_LONG
	struct option long_options[4];
#endif
	int opt, i;
	char *init_obj_file = NULL;

#ifdef HAVE_GETOPT_LONG
	/* Initialize long-options array */
	for (i = 0; i < 3; i++) {
		long_options[i].name = STRS_CLI_options[i].lopt;
		long_options[i].has_arg = no_argument;
		long_options[i].flag = NULL;
		long_options[i].val = STRS_CLI_options[i].opt;
	}
	long_options[3].name = 0;
	long_options[3].has_arg = 0;
	long_options[3].flag = 0;
	long_options[3].val = 0;
#endif /* HAVE_GETOPT_LONG */

	/* Parse command-line options */
	while (TRUE) {
#ifdef HAVE_GETOPT_LONG
		opt = getopt_long( argc, argv, STR_CLI_option_chars, long_options, NULL );
#else
		opt = getopt( argc, argv, STR_CLI_option_chars );
#endif /* not HAVE_GETOPT_LONG */
		if (opt == -1)
			break;

		if (opt == STRS_CLI_options[0].opt) {
			/* -h --help */
			cmdline_help( argv[0] );
			return 0;
		}
		if (opt == STRS_CLI_options[1].opt) {
			/* -s --simple */
			advanced_interface = FALSE;
			break;
		}
		if (opt == STRS_CLI_options[2].opt) {
			/* -a --advanced */
			advanced_interface = TRUE;
			break;
		}
	}
	if (optind < argc) {
		/* object */
		init_obj_file = argv[optind];
	}

	/* Initialize profiling */
	profile( INITIALIZE );

	/* Initialize GTK+ */
	gtk_init( &argc, &argv );

	/* Check for OpenGL support (GLX extension) */
	if (gdk_gl_query( ) == FALSE) {
		fprintf( stderr, "Light Speed! requires OpenGL support to run." );
		fflush( stderr );
		return -1;
	}

	/* Generate/load initial object */
	if (init_obj_file == NULL)
		make_lattice( DEF_LATTICE_X, DEF_LATTICE_Y, DEF_LATTICE_Z, DEF_LATTICE_SMOOTH );
	else {
		i = import_objects( init_obj_file );
		if (i < 0) {
			gtk_timeout_add( 5000, (GtkFunction)gtk_main_quit, NULL );
			gtk_main( );
			return -1;
		}
	}

	/* Make primary camera */
	new_camera( );

	/* Construct main window */
	main_window( );

	/* Fire up the warp engine */
	warp( INITIALIZE, NULL );

	/* Initialize gamma correction if default says so */
	if (DEF_DGAMMA_CORRECT != 1.0) {
		calc_dgamma_lut( DEF_DGAMMA_CORRECT );
		dgamma_correct = TRUE;
	}
	else
		dgamma_correct = FALSE;

	/* Showtime! */
	gtk_main( );

	return 0;
}

/* That's all, folks! */

/* end lightspeed.c */
