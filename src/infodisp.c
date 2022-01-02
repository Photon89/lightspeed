/* infodisp.c */

/* Graphical text information display ("HUD") */

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


/* Info display control */
void
info_display( int message1, int message2 )
{
	static int disp_anything = DEF_INFODISP_ACTIVE;
	static int disp_velocity = DEF_INFODISP_SHOW_VELOCITY;
	static int disp_time_t = DEF_INFODISP_SHOW_TIME_T;
	static int disp_gamma = DEF_INFODISP_SHOW_GAMMA;
	static int disp_framerate = DEF_INFODISP_SHOW_FRAMERATE;
	int no_contract;
	int no_deform;
	int no_headlight;
	int no_doppler;
	char disp_str[64];

	switch (message1) {
	case INFODISP_DRAW:
		if (disp_anything)
			break;
		else
			return;

	case INFODISP_ACTIVE:
		disp_anything = message2;
		queue_redraw( 0 );
		return;

	case INFODISP_SHOW_VELOCITY:
		disp_velocity = message2;
		goto dd_redraw;

	case INFODISP_SHOW_TIME_T:
		disp_time_t = message2;
		goto dd_redraw;

	case INFODISP_SHOW_GAMMA:
		disp_gamma = message2;
		goto dd_redraw;

	case INFODISP_SHOW_FRAMERATE:
		disp_framerate = message2;
		goto dd_redraw;

	case RESET:
dd_redraw:
		if (disp_anything) {
			/* Info in display has changed in some way */
			queue_redraw( 0 );
		}
		return;

	default:
#ifdef DEBUG
		crash( "info_display( ): invalid message" );
#endif
		return;
	}

	/* Z-buffer no good for 2D drawing, lighting likewise */
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	if (disp_velocity) {
		sprintf( disp_str, STR_INF_velocity_ARG, velocity_string( velocity, TRUE ) );
		ogl_draw_string( disp_str, POS_BOTTOM_LEFT, 2 );
	}

	if (disp_time_t) {
		sprintf( disp_str, STR_INF_time_ARG, time_string( ) );
		ogl_draw_string( disp_str, POS_TOP_LEFT, 2 );
	}

	if (disp_gamma) {
		sprintf( disp_str, STR_INF_gamma_ARG, lorentz_factor( velocity ) );
		ogl_draw_string( disp_str, POS_BOTTOM_RIGHT, 1 );
	}

	/* Get status of each of the warp transforms */
	no_contract = !warp( QUERY, MESG_(WARP_LORENTZ_CONTRACTION) );
	no_deform = !warp( QUERY, MESG_(WARP_OPTICAL_DEFORMATION) );
	no_headlight = !warp( QUERY, MESG_(WARP_HEADLIGHT_EFFECT) );
	no_doppler = !warp( QUERY, MESG_(WARP_DOPPLER_SHIFT) );

	if (no_contract && no_doppler && no_headlight && no_deform)
		ogl_draw_string( STR_INF_no_relativity, POS_TOP_RIGHT, 1 );
	else {
		if (no_contract)
			ogl_draw_string( STR_INF_no_contraction, POS_TOP_RIGHT, 0 );

		if (no_doppler)
			ogl_draw_string( STR_INF_no_doppler_shift, POS_TOP_RIGHT, 0 );

		if (no_headlight)
			ogl_draw_string( STR_INF_no_headlight_effect, POS_TOP_RIGHT, 0 );

		if (no_deform)
			ogl_draw_string( STR_INF_no_deformation, POS_TOP_RIGHT, 0 );
	}

	if (disp_framerate) {
		sprintf( disp_str, STR_INF_fps_ARG, framerate );
		ogl_draw_string( disp_str, POS_TOP_RIGHT, 0 );
	}

	glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
}

/* end infodisp.c */
