/* settings.h */

/* Compile-time settings */

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


/**** Feature switches ***************************************************/
/** Comment any of these out and they will not be compiled in **/

/* Allows importing of 3D Studio and Lightwave objects */
#define WITH_OBJECT_IMPORTER

/* Allows exporting to the SRS (Special Relativity Scene) format used
 * by Antony Searle's BACKLIGHT relativistic raytracer */
#define WITH_SRS_EXPORTER


/**** Completely arbitrary defaults **************************************/

/* Set advanced interface as default */
#define DEF_ADVANCED_INTERFACE	TRUE

/* Initial lattice dimensions and smoothness factor */
#define DEF_LATTICE_X           1
#define DEF_LATTICE_Y           1
#define DEF_LATTICE_Z           1
#define DEF_LATTICE_SMOOTH      6

/* 0 == decimal of c, 1 == meters/sec, 2 == kilometers/hour */
#define DEF_VELOCITY_UNITS	1

/* Initial/resetted camera view. Angles are in degrees */
#define DEF_CAMERA_PHI		195.0
#define DEF_CAMERA_THETA	-10.0
#define DEF_CAMERA_FOV		60.0

/* Initial info display configuration */
#define DEF_INFODISP_ACTIVE		TRUE
#define DEF_INFODISP_SHOW_VELOCITY	TRUE
#define DEF_INFODISP_SHOW_TIME_T	TRUE
#define DEF_INFODISP_SHOW_GAMMA		FALSE
#define DEF_INFODISP_SHOW_FRAMERATE	FALSE

/* Display gamma correction (1.0 == none)
 * e.g. 1.5 for a dark monitor, 0.5 for a bright one */
#define DEF_DGAMMA_CORRECT	1.0

/* Mouse sensitivity (mostly a trial & error constant) */
#define DEF_MOUSE_SENS		0.5


/**** Less arbitrary defaults ********************************************/

/* Velocity slider position
 * 0 == no slider, 1 == at left of viewport, 2 == at right
 * NOTE: Text for Overview dialog will need updating if this is not 2 ! */
#define VELOCITY_SLIDER		2

/* Look-up tables speed up the warp( ) engine at the expense of a small
 * margin of precision */
#define USE_LOOKUP_TABLES	TRUE
/* Table resolution (each will have LUT_RES+1 entries) */
#define LUT_RES			1023

/* Lattice geometry (in meters) */
#define LATTICE_UNIT_SIZE	1.0
#define BALL_RADIUS		0.125
#define STICK_RADIUS		(0.125 / SQR(MAGIC_NUMBER))

/* Lattice colors (RGB) */
#define BALL_R			0.01
#define BALL_G			0.125
#define BALL_B			0.5
#define STICK_R			0.5
#define STICK_G			0.5
#define STICK_B			0.5

/* Info display text colors (RGB) */
#define INFODISP_TEXT_FRONT_R	0.0
#define INFODISP_TEXT_FRONT_G	0.75
#define INFODISP_TEXT_FRONT_B	0.0
#define INFODISP_TEXT_BACK_R	0.0
#define INFODISP_TEXT_BACK_G	0.25
#define INFODISP_TEXT_BACK_B	0.0

/* Eye spacing for stereographic SRS output (in meters) */
#define EYE_SPACING		0.064

/* Framerate is a rolling average calculated over roughly this many seconds
 * (minimum n, maximum n+1 seconds) */
#define FRAMERATE_AVERAGE_TIME	4

/* end settings.h */
