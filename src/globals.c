/* globals.c */

/* Global variables */

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


/* Message value array (used in conjunction with the MESG_ macro) */
const int mesg_vals[] = {
	 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,
	60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
};

/* Unit definitions */
const int num_unit_systems = 3;
const char *unit_suffixes[] = {
	"c",
	"m/s",
	"Km/h"
};
const double unit_conv_factors[] = { C, 1.0, (5.0/18.0) };
const int unit_num_decimals[] = { 6, 0, 0 };
int cur_unit_system = DEF_VELOCITY_UNITS;

/* Background colors */
const int num_background_colors = 4;
/* Background color names are defined in lstrings.c */
const rgb_color bkgd_colors[] = {
	{ 0.0, 0.0, 0.0 },
	{ 0.25, 0.25, 0.25 },
	{ 0.85, 0.85, 0.85 },
	{ 1.0, 1.0, 1.0 }
};
rgb_color background = { 0.0, 0.0, 0.0 };

/* Fonts for info display */
const int num_font_sizes = 4;
const char *sys_font_names[] = {
	"-adobe-helvetica-bold-o-normal--*-120-*-*-*-*-*-*",
	"-adobe-helvetica-bold-o-normal--*-140-*-*-*-*-*-*",
	"-adobe-helvetica-bold-o-normal--*-180-*-*-*-*-*-*",
	"-adobe-helvetica-bold-o-normal--*-240-*-*-*-*-*-*"
};
const int sys_font_sizes[] = { 120, 140, 180, 240 };
/* Character code for the "micro-" prefix glyph (looks similar to a u)
 * NOTE: revise ogl_draw_string( ) if >= 192 */
const int micro_glyph = 181;

/* Stock camera lenses (specified in mm) */
const int num_stock_lenses = 6;
const float stock_lenses[ ] = { 28, 35, 50, 85, 135, 200 };

/* Image format extensions
 * Order must match that of the IMAGE_FORMAT_??? symbols in lightspeed.h */
const char *image_format_exts[] = {
	".png",
	".tif"
};

/* Vehicle velocity */
double velocity = 1.0;

/* Current time t
 * (of simulation-- NOT system clock */
double cur_time_t;

/* This is the "real" x-position of the vehicle */
float vehicle_real_x;

/* Type of object(s) currently in the viewer */
int object_mode = MODE_LATTICE;

/* Current lattice dimensions (in lattice units) */
int lattice_size_x = DEF_LATTICE_X;
int lattice_size_y = DEF_LATTICE_Y;
int lattice_size_z = DEF_LATTICE_Z;

/* Graphical framerate (fps)
 * (don't worry, it gets better :-) */
float framerate = 0.0;

/* Vehicle (moving) object(s) */
ogl_object **vehicle_objs;
int num_vehicle_objs;
extents vehicle_extents;

/* World extents
 * Only xmin/xmax fields are significant, for now
 * (and are determined by size of floating grid) */
extents world_extents = { -10.0, 10.0, NIL, NIL, NIL, NIL, NIL };

/* Camera array. *usr_cam[0] is primary */
camera **usr_cams = NULL;
int num_cams = 0;
int cur_cam;

/* Output camera (for snapshots and SRS scenes) */
camera out_cam;

/* Advanced interface flag (controls presence of extra features) */
int advanced_interface = DEF_ADVANCED_INTERFACE;

/* Gamma correction flag & lookup table
 * NOTE: This is display gamma, not Lorentz factor gamma! */
int dgamma_correct;
float dgamma_lut[LUT_RES + 1];

/* Mouse sensitivity setting */
float mouse_sens = DEF_MOUSE_SENS;

/* end globals.c */
