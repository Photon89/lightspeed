/* lstrings.c */

/* Language-specific strings (English) */

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


/* All changes necessary to port Light Speed! to a new language should be
 * contained within this file. If modifications are necessary elsewhere in
 * the code, please let me know. There are surely a few odd English-specific
 * constructs not quite in here yet :-) */


const char *STR_Light_Speed		= "Light Speed!";
/* %d = year, %s = author's name or author's name + e-mail address */
const char *STR_copyright_ARG		= "Copyright (C) %d by %s";

/**** COMMAND-LINE INTERFACE ****/

/* %s = executable name */
const char *STR_CLI_usage_ARG		= "usage: %s [-hsa] [object]";
struct option_desc STRS_CLI_options[] = {
	{ 'h', "help", "Print this help screen" },
#ifdef DEF_ADVANCED_INTERFACE
	{ 's', "simple", "Use simplified interface" },
	{ 'a', "advanced", "Use advanced interface (default)" },
#else
	{ 's', "simple", "Use simple interface (default)" },
	{ 'a', "advanced", "Use more advanced interface" },
#endif /* not DEF_ADVANCED_INTERFACE */
	{ '\0', "object", "3D file to load on startup (.3DS or .LWO)" }
};
const char *STR_CLI_option_chars	= "hsa";

/**** MENUS ****/

/* Menu bar headings */
const char *STR_MNU_File		= "File";
const char *STR_MNU_Objects		= "Objects";
const char *STR_MNU_Warp		= "Warp";
const char *STR_MNU_Camera		= "Camera";
const char *STR_MNU_Help		= "Help";

/* File menu */
const char *STR_MNU_New_lattice		= "New lattice...";
const char *STR_MNU_Load_object		= "Load object...";
const char *STR_MNU_Save_snapshot	= "Save snapshot...";
const char *STR_MNU_Export_srs		= "Export SRS...";
const char *STR_MNU_Exit		= "Exit";

/* Objects menu */
const char *STR_MNU_Coordinate_axes	= "Coordinate axes";
const char *STR_MNU_Floating_grid	= "Floating grid";
const char *STR_MNU_Bounding_box	= "Bounding box";
const char *STR_MNU_Animation		= "Animation...";

/* Warp menu */
const char *STR_MNU_Lorentz_contraction	= "Lorentz contraction";
const char *STR_MNU_Doppler_shift	= "Doppler red/blue shift";
const char *STR_MNU_Headlight_effect	= "Headlight effect";
const char *STR_MNU_Optical_deformation	= "Optical aberration";

/* Camera menu */
const char *STR_MNU_Lens		= "Lens";
const char *STR_MNU_Position		= "Position...";
const char *STR_MNU_Reset_view		= "Reset view";
const char *STR_MNU_Info_display	= "Info display";
const char *STR_MNU_Background		= "Background";
const char *STR_MNU_Graphics_mode	= "Graphics mode";
const char *STR_MNU_Spawn_camera	= "Spawn camera";
const char *STR_MNU_Close		= "Close";

/* Camera->Lens submenu */
const char *STR_MNU_Custom		= "Custom";

/* Camera->Info_display submenu */
const char *STR_MNU_Active		= "Active";
const char *STR_MNU_Velocity		= "Velocity";
const char *STR_MNU_Time_t		= "Time t";
const char *STR_MNU_Gamma_factor	= "Gamma factor";
const char *STR_MNU_Framerate		= "Framerate";

/* Camera->Background submenu colors
 * (corresponding color hues are defined in globals.c) */
const char *STRS_MNU_bkgd_color_names[]	= { "Black", "Grey", "White", "Very white" };

/* Camera->Graphics_mode submenu */
const char *STR_MNU_Wireframe		= "Wireframe";
const char *STR_MNU_Shaded		= "Shaded";

/* Help menu */
const char *STR_MNU_Overview		= "Overview";
const char *STR_MNU_Controls		= "Controls";
const char *STR_MNU_About		= "About";

/**** INFO DISPLAY ****/

/* Time t
 * %s == time string as returned by time_string( ) */
const char *STR_INF_time_ARG		= "t = %ssec";

/* Framerate */
const char *STR_INF_fps_ARG		= "%.1f fps";

/* Velocity
 * %s == velocity strings as returned by velocity_string( ???, TRUE ) */
const char *STR_INF_velocity_ARG	= "Velocity: %s";

/* Gamma factor */
const char *STR_INF_gamma_ARG		= "gamma = %.3f";

/* Relativistic toggle messages */
const char *STR_INF_no_contraction	= "LORENTZ CONTRACTION NOT SHOWN";
const char *STR_INF_no_doppler_shift	= "DOPPLER RED/BLUE SHIFT NOT SHOWN";
const char *STR_INF_no_headlight_effect	= "HEADLIGHT EFFECT NOT SHOWN";
const char *STR_INF_no_deformation	= "OPTICAL ABERRATION NOT SHOWN";
const char *STR_INF_no_relativity	= "NO RELATIVISTIC EFFECTS SHOWN!!!";

/**** DIALOGS ****/

const char *STR_DLG_Okay_btn		= "OK";
const char *STR_DLG_Cancel_btn		= "Cancel";
const char *STR_DLG_Close_btn		= "Close";

/* New Lattice dialog */
const char *STR_DLG_New_lattice		= "New lattice";
const char *STR_DLG_Dimensions		= "Dimensions";
const char *STR_DLG_Smoothness		= "Smoothness";

/* Load Object dialog */
const char *STR_DLG_Load_Object		= "Load Object";
const char *STR_DLG_formats_all		= "All 3D object files";
const char *STR_DLG_formats_3ds		= "3D Studio file(*.3ds)";
const char *STR_DLG_formats_lwo		= "LightWave 3D file(*.lwo)";

/* Save snapshot dialog */
const char *STR_DLG_Save_Snapshot	= "Save Snapshot";
const char *STR_DLG_snapshot_Parameters	= "Parameters";
const char *STR_DLG_snapshot_Size	= "Size";
const char *STR_DLG_snapshot_Format	= "Format";
const char *STR_DLG_snapshot_basename	= "snapshot";

/* Export SRS dialog */
const char *STR_DLG_Export_srs		= "Export SRS";
const char *STR_DLG_srs			= "Special Relativity Scene (SRS)";
const char *STR_DLG_srs_Parameters	= "Parameters";
const char *STR_DLG_srs_Size		= "Rendered size";
const char *STR_DLG_srs_Stereo_view	= "Stereoscopic view";
const char *STR_DLG_srs_Vis_faces_only	= "Visible faces only";
/* basename1 is for lattices, basename2 is for imported geometry */
const char *STR_DLG_srs_basename1	= "lattice";
const char *STR_DLG_srs_basename2	= "object";

/* Animation dialog */
const char *STR_DLG_Animation		= "Animation";
const char *STR_DLG_Observed_range	= "Observed range of motion";
const char *STR_DLG_Start_X		= "Starting X";
const char *STR_DLG_End_X		= "Ending X";
const char *STR_DLG_Loop_time		= "Loop time: ";
const char *STR_DLG_seconds		= " seconds";
const char *STR_DLG_Begin_btn		= "Begin";
const char *STR_DLG_Stop_btn		= "Stop";

/* Camera Position dialog */
const char *STR_DLG_Camera_Position	= "Camera Position";
const char *STR_DLG_Location		= "Location";
const char *STR_DLG_View_target		= "View target";
const char *STR_DLG_Direction		= "Direction";
const char *STR_DLG_Phi_label		= "Phi [0, 360)";
const char *STR_DLG_Theta_label		= "Theta [-90, 90]";
const char *STR_DLG_Angles_instead	= "Specify phi/theta direction instead";
const char *STR_DLG_Xyz_instead		= "Specify (x,y,z) view target instead";
const char *STR_DLG_Reposition_btn	= "Reposition";

/* Custom Lens dialog */
const char *STR_DLG_Custom_Lens		= "Custom Lens";
const char *STR_DLG_Lens_length		= "Lens length";
const char *STR_DLG_Field_of_view	= "Field of view";
const char *STR_DLG_degree_suffix	= "deg";

/* Overview dialog */
const char *STR_DLG_Overview		= "Overview";

/* Controls dialog */
const char *STR_DLG_Controls		= "Controls";

/* About dialog */
const char *STR_DLG_About		= "About";
/* %d == version major, %d == version minor */
const char *STR_DLG_Version_x_y_ARG	= "<b><span size='x-large'>Light Speed! %s</span></b>";
/* %s == author's name */
const char *STR_DLG_authorship_ARG	= "A Theory Toy by %s";
const char *STR_DLG_home_page_url	= "<a href='http://lightspeed.sourceforge.net/'>Website</a>";

/* Spawned camera window title */
const char *STR_DLG_Camera		= "Camera";

/* Miscellaneous dialog titles */
const char *STR_DLG_Warning		= "Warning";
const char *STR_DLG_Error		= "Error";

/* Help->Overview text
 * (from the OVERVIEW file) */
const char *STR_DLG_Overview_TEXT = "\
Light Speed! is a simulator which can illustrate the effects of\n\
special relativity on the appearance of objects travelling at\n\
ultra-high speeds. Depending on the particular speed, and one's\n\
point of view, relativistic effects can cause the object to\n\
appear shorter, longer, brighter, darker, deformed and/or\n\
off-color.\n\
\n\
To adjust the velocity, use the slider along the right edge of\n\
the main window. You can also type in a value, using the entry\n\
at the top right (press <Enter> once it is in). To change the\n\
units shown, press the button to the right of the entry, and it\n\
will cycle through a small list.\n\
\n\
The object travels in the positive-x direction, which can be\n\
visually checked by activating Objects -> Coordinate axes.\n\
";

/* Help->Controls text
 * (from the CONTROLS file) */
const char *STR_DLG_Controls_TEXT = "\
Most interactive control is performed with the mouse. By holding\n\
down a particular button, and dragging the pointer around,\n\
various camera motions can be obtained:\n\
\n\
   Left button:  Revolve camera around view targetg\n\
\n\
   Left button + Shift key:  Revolve view target around camera\n\
\n\
   Middle button:  Translate camera left, right, up or down\n\
\n\
   Right button:  Dolly in or out\n\
\n\
The first and last motions are generally the most useful.\n\
Should the camera become difficult to control at any point, it\n\
may be re-initialized by selecting Camera -> Reset View.\n\
";

/**** MESSAGES ****/

/* For quick-and-dirty command feedback */
const char *STR_MSG_Okay		= "OK";
const char *STR_MSG_Invalid		= "INVALID";

/* File overwrite warning
 * %s == filename (basename) of file at risk */
const char *STR_MSG_overwrite_warn_ARG	= "The file \"%s\" already exists.\nPress OK to overwrite it.";

/* Object importer error messages */
/* %s == (bogus) filename */
const char *STR_MSG_no_object_file_ARG	= "The specified object file could not be opened.\n(%s)\nImport operation failed.";
const char *STR_MSG_not_3ds_file	= "The file lacks a valid 3DS signature.\nImport operation failed.";
const char *STR_MSG_not_prj_file	= "The file lacks a valid 3D Studio PRJ signature.\nImport operation failed.";
const char *STR_MSG_not_lwo_file	= "The file lacks a valid LWOB signature.\nImport operation failed.";
const char *STR_MSG_unknown_obj_format	= "The object must be in 3D Studio (3DS)\nor LightWave 3D (LWO) file format.\nImport operation failed.";
const char *STR_MSG_bad_3ds_file	= "The 3D Studio file could not be properly read.\nImport operation failed.";
const char *STR_MSG_empty_3ds_file	= "The 3D Studio file has no valid geometry.\nImport operation failed.";
const char *STR_MSG_bad_lwo_file	= "The LightWave file could not be properly read.\nImport operation failed.";
const char *STR_MSG_empty_lwo_file	= "The LightWave file has no valid geometry.\nImport operation failed.";

/* Snapshot exporter error messages */
const char *STR_MSG_no_ogl_visual	= "The system could not provide the required visual.\nSave operation failed.";
/* %d == width, %d == height */
const char *STR_MSG_no_render_buf_ARG	= "A %dx%d render buffer could not be allocated.\nSave operation failed.";
const char *STR_MSG_no_ogl_context	= "The required OpenGL context was not available.\nSave operation failed.";
const char *STR_MSG_no_snapshot_output	= "An error occurred in creating the output file.\nSave operation failed.";

/* Viewport-centered messages */
const char *STR_MSG_Generating_lattice	= "GENERATING LATTICE . . .";
const char *STR_MSG_Importing_object	= "LOADING OBJECT . . .";
const char *STR_MSG_Rendering_snapshot	= "RENDERING . . .";
/* %d == percent of snapshot written */
const char *STR_MSG_Saving_snapshot_ARG	= "SAVING SNAPSHOT . . .\n(%d%%)";

/* end lstrings.c */
