/* command.c */

/* Type-in command handlers */

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


int
command( const char *input )
{
	static char *cmd_strs[] = {
	    "ZROT_CCW",
	    "ZROT_CW",
	    "PERFSTATS",
	    "GEOMSTATS",
	    "MEMSTATS",
	    "MEMBLOCKS",
	    "DGAMMA=",
	    "STRAKER",
	    "SKUNK"
	};
	const int num_cmd_strs = sizeof(cmd_strs) / sizeof(char *);
	const char *arg = NULL;
	float f;
	int cmd;
	int i;

	for (cmd = 0; cmd < num_cmd_strs; cmd++) {
		i = strlen( cmd_strs[cmd] );
		if (!strncasecmp( input, cmd_strs[cmd], i )) {
			arg = &input[strlen( cmd_strs[cmd] )];
			break;
		}
	}

	switch (cmd) {
	case 0: /* ZROT_CCW */
		rotate_all_objects( Z_ROTATE_CCW );
		return 0;

	case 1: /* ZROT_CW */
		rotate_all_objects( Z_ROTATE_CW );
		return 0;

	case 2: /* PERFSTATS */
		profile( PROFILE_SHOW_STATS );
		return 0;

	case 3: /* GEOMSTATS */
		show_geometry_stats( );
		return 0;

	case 4: /* MEMSTATS */
#ifdef WITH_TRACKMEM
		trackmem_show_stats( );
		return 0;
#else
		/* TRACKMEM was not compiled in */
		return -1;
#endif

	case 5: /* MEMBLOCKS */
#ifdef WITH_TRACKMEM
		trackmem_show_blocks( );
		return 0;
#else
		/* TRACKMEM was not compiled in */
		return -1;
#endif

	case 6: /* DGAMMA= */
		/* Adjust display gamma correction */
		if (!strcasecmp( arg, "OFF" )) {
			dgamma_correct = FALSE;
			queue_redraw( -1 );
			return 0;
		}
		f = strtod( arg, NULL );
		if ((f < 0.1) || (f > 8.0))
			return -1;
		if (f == 1.0)
			dgamma_correct = FALSE;
		else {
			dgamma_correct = TRUE;
			calc_dgamma_lut( f );
		}
		queue_redraw( -1 );
		return 0;

	case 7: /* STRAKER */
	case 8: /* SKUNK */
		ss( ); /* // */
		return 0;

	default:
		return -1;
	}
}

/* end command.c */
