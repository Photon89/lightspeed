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

#include <libintl.h>
#include <locale.h>

#define _(STRING) gettext(STRING)


/* All changes necessary to port Light Speed! to a new language should be
 * contained within this file. If modifications are necessary elsewhere in
 * the code, please let me know. There are surely a few odd English-specific
 * constructs not quite in here yet :-) */


const char *STR_Light_Speed		= _("Light Speed!");
/* %d = year, %s = author's name or author's name + e-mail address */
const char *STR_copyright_ARG		= _("Copyright (C) %d by %s");

/**** COMMAND-LINE INTERFACE ****/

/* %s = executable name */
const char *STR_CLI_usage_ARG		= _("usage: %s [-hsa] [object]");
struct option_desc STRS_CLI_options[]	= {
	{ 'h', "help", _("Print this help screen") },
#ifdef DEF_ADVANCED_INTERFACE
	{ 's', "simple", _("Use simplified interface") },
	{ 'a', "advanced", _("Use advanced interface (default)") },
#else
	{ 's', "simple", _("Use simple interface (default)") },
	{ 'a', "advanced", _("Use more advanced interface") },
#endif /* not DEF_ADVANCED_INTERFACE */
	{ '\0', "object", _("3D file to load on startup (.3DS or .LWO)") }
};
const char *STR_CLI_option_chars	= "hsa";

/**** MENUS ****/

/* Menu bar headings */
const char *STR_MNU_File		= _("File");
const char *STR_MNU_Objects		= _("Objects");
const char *STR_MNU_Warp		= _("Warp");
const char *STR_MNU_Camera		= _("Camera");
const char *STR_MNU_Help		= _("Help");

/* File menu */
const char *STR_MNU_New_lattice		= _("New lattice...");
const char *STR_MNU_Load_object		= _("Load object...");
const char *STR_MNU_Save_snapshot	= _("Save snapshot...");
const char *STR_MNU_Export_srs		= _("Export SRS...");
const char *STR_MNU_Exit		= _("Exit");

/* Objects menu */
const char *STR_MNU_Coordinate_axes	= _("Coordinate axes");
const char *STR_MNU_Floating_grid	= _("Floating grid");
const char *STR_MNU_Bounding_box	= _("Bounding box");
const char *STR_MNU_Animation		= _("Animation...");

/* Warp menu */
const char *STR_MNU_Lorentz_contraction	= _("Lorentz contraction");
const char *STR_MNU_Doppler_shift		= _("Doppler red/blue shift");
const char *STR_MNU_Headlight_effect	= _("Headlight effect");
const char *STR_MNU_Optical_deformation	= _("Optical aberration");

/* Camera menu */
const char *STR_MNU_Lens		= _("Lens");
const char *STR_MNU_Position		= _("Position...");
const char *STR_MNU_Reset_view		= _("Reset view");
const char *STR_MNU_Info_display	= _("Info display");
const char *STR_MNU_Background		= _("Background");
const char *STR_MNU_Graphics_mode	= _("Graphics mode");
const char *STR_MNU_Spawn_camera	= _("Spawn camera");
const char *STR_MNU_Close		= _("Close");

/* Camera->Lens submenu */
const char *STR_MNU_Custom		= _("Custom");

/* Camera->Info_display submenu */
const char *STR_MNU_Active		= _("Active");
const char *STR_MNU_Velocity		= _("Velocity");
const char *STR_MNU_Time_t		= _("Time t");
const char *STR_MNU_Gamma_factor	= _("Gamma factor");
const char *STR_MNU_Framerate		= _("Framerate");

/* Camera->Background submenu colors
 * (corresponding color hues are defined in globals.c) */
const char *STRS_MNU_bkgd_color_names[]	= { _("Black"), _("Grey"), _("White"), _("Very white") };

/* Camera->Graphics_mode submenu */
const char *STR_MNU_Wireframe		= _("Wireframe");
const char *STR_MNU_Shaded		= _("Shaded");

/* Help menu */
const char *STR_MNU_Overview		= _("Overview");
const char *STR_MNU_Controls		= _("Controls");
const char *STR_MNU_About		= _("About");

/**** INFO DISPLAY ****/

/* Time t
 * %s == time string as returned by time_string( ) */
const char *STR_INF_time_ARG		= _("t = %ssec");

/* Framerate */
const char *STR_INF_fps_ARG		= _("%.1f fps");

/* Velocity
 * %s == velocity strings as returned by velocity_string( ???, TRUE ) */
const char *STR_INF_velocity_ARG	= _("Velocity: %s");

/* Gamma factor */
const char *STR_INF_gamma_ARG		= _("gamma = %.3f");

/* Relativistic toggle messages */
const char *STR_INF_no_contraction	= _("LORENTZ CONTRACTION NOT SHOWN");
const char *STR_INF_no_doppler_shift	= _("DOPPLER RED/BLUE SHIFT NOT SHOWN");
const char *STR_INF_no_headlight_effect	= _("HEADLIGHT EFFECT NOT SHOWN");
const char *STR_INF_no_deformation	= _("OPTICAL ABERRATION NOT SHOWN");
const char *STR_INF_no_relativity	= _("NO RELATIVISTIC EFFECTS SHOWN!!!");

/**** DIALOGS ****/

const char *STR_DLG_Okay_btn		= _("OK");
const char *STR_DLG_Cancel_btn		= _("Cancel");
const char *STR_DLG_Close_btn		= _("Close");

/* New Lattice dialog */
const char *STR_DLG_New_lattice		= _("New lattice");
const char *STR_DLG_Dimensions		= _("Dimensions");
const char *STR_DLG_Smoothness		= _("Smoothness");

/* Load Object dialog */
const char *STR_DLG_Load_Object		= _("Load Object");
const char *STR_DLG_formats_all		= _("All 3D object files");
const char *STR_DLG_formats_3ds		= _("3D Studio file(*.3ds)");
const char *STR_DLG_formats_lwo		= _("LightWave 3D file(*.lwo)");

/* Save snapshot dialog */
const char *STR_DLG_Save_Snapshot	= _("Save Snapshot");
const char *STR_DLG_snapshot_Parameters	= _("Parameters");
const char *STR_DLG_snapshot_Size	= _("Size");
const char *STR_DLG_snapshot_Format	= _("Format");
const char *STR_DLG_snapshot_basename	= _("snapshot");

/* Export SRS dialog */
const char *STR_DLG_Export_srs		= _("Export SRS");
const char *STR_DLG_srs			= _("Special Relativity Scene (SRS)");
const char *STR_DLG_srs_Parameters	= _("Parameters");
const char *STR_DLG_srs_Size		= _("Rendered size");
const char *STR_DLG_srs_Stereo_view	= _("Stereoscopic view");
const char *STR_DLG_srs_Vis_faces_only	= _("Visible faces only");
/* basename1 is for lattices, basename2 is for imported geometry */
const char *STR_DLG_srs_basename1	= _("lattice");
const char *STR_DLG_srs_basename2	= _("object");

/* Animation dialog */
const char *STR_DLG_Animation		= _("Animation");
const char *STR_DLG_Observed_range	= _("Observed range of motion");
const char *STR_DLG_Start_X		= _("Starting X");
const char *STR_DLG_End_X		= _("Ending X");
const char *STR_DLG_Loop_time		= _("Loop time: ");
const char *STR_DLG_seconds		= _(" seconds");
const char *STR_DLG_Begin_btn		= _("Begin");
const char *STR_DLG_Stop_btn		= _("Stop");

/* Camera Position dialog */
const char *STR_DLG_Camera_Position	= _("Camera Position");
const char *STR_DLG_Location		= _("Location");
const char *STR_DLG_View_target		= _("View target");
const char *STR_DLG_Direction		= _("Direction");
const char *STR_DLG_Phi_label		= _("Phi [0, 360)");
const char *STR_DLG_Theta_label		= _("Theta [-90, 90]");
const char *STR_DLG_Angles_instead	= _("Specify phi/theta direction instead");
const char *STR_DLG_Xyz_instead		= _("Specify (x,y,z) view target instead");
const char *STR_DLG_Reposition_btn	= _("Reposition");

/* Custom Lens dialog */
const char *STR_DLG_Custom_Lens		= _("Custom Lens");
const char *STR_DLG_Lens_length		= _("Lens length");
const char *STR_DLG_Field_of_view	= _("Field of view");
const char *STR_DLG_degree_suffix	= _("deg");

/* Overview dialog */
const char *STR_DLG_Overview		= _("Overview");

/* Controls dialog */
const char *STR_DLG_Controls		= _("Controls");

/* About dialog */
const char *STR_DLG_About		= _("About");
/* %d == version major, %d == version minor */
const char *STR_DLG_Version_x_y_ARG	= _("<b><span size='x-large'>Light Speed! %s</span></b>");
/* %s == author's name */
const char *STR_DLG_authorship_ARG	= _("A Theory Toy by %s");
const char *STR_DLG_home_page_url	= _("<a href='http://lightspeed.sourceforge.net/'>Website</a>");

/* Spawned camera window title */
const char *STR_DLG_Camera		= _("Camera");

/* Miscellaneous dialog titles */
const char *STR_DLG_Warning		= _("Warning");
const char *STR_DLG_Error		= _("Error");

/* Help->Overview text
 * (from the OVERVIEW file) */
const char *STR_DLG_Overview_TEXT	= _("\
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
");

/* Help->Controls text
 * (from the CONTROLS file) */
const char *STR_DLG_Controls_TEXT	= _("\
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
");

/**** MESSAGES ****/

/* For quick-and-dirty command feedback */
const char *STR_MSG_Okay		= _("OK");
const char *STR_MSG_Invalid		= _("INVALID");

/* File overwrite warning
 * %s == filename (basename) of file at risk */
const char *STR_MSG_overwrite_warn_ARG	= _("The file \"%s\" already exists.\nPress OK to overwrite it.");

/* Object importer error messages */
/* %s == (bogus) filename */
const char *STR_MSG_no_object_file_ARG	= _("The specified object file could not be opened.\n(%s)\nImport operation failed.");
const char *STR_MSG_not_3ds_file	= _("The file lacks a valid 3DS signature.\nImport operation failed.");
const char *STR_MSG_not_prj_file	= _("The file lacks a valid 3D Studio PRJ signature.\nImport operation failed.");
const char *STR_MSG_not_lwo_file	= _("The file lacks a valid LWOB signature.\nImport operation failed.");
const char *STR_MSG_unknown_obj_format	= _("The object must be in 3D Studio (3DS)\nor LightWave 3D (LWO) file format.\nImport operation failed.");
const char *STR_MSG_bad_3ds_file	= _("The 3D Studio file could not be properly read.\nImport operation failed.");
const char *STR_MSG_empty_3ds_file	= _("The 3D Studio file has no valid geometry.\nImport operation failed.");
const char *STR_MSG_bad_lwo_file	= _("The LightWave file could not be properly read.\nImport operation failed.");
const char *STR_MSG_empty_lwo_file	= _("The LightWave file has no valid geometry.\nImport operation failed.");

/* Snapshot exporter error messages */
const char *STR_MSG_no_ogl_visual	= _("The system could not provide the required visual.\nSave operation failed.");
/* %d == width, %d == height */
const char *STR_MSG_no_render_buf_ARG	= _("A %dx%d render buffer could not be allocated.\nSave operation failed.");
const char *STR_MSG_no_ogl_context	= _("The required OpenGL context was not available.\nSave operation failed.");
const char *STR_MSG_no_snapshot_output	= _("An error occurred in creating the output file.\nSave operation failed.");

/* Viewport-centered messages */
const char *STR_MSG_Generating_lattice	= _("GENERATING LATTICE . . .");
const char *STR_MSG_Importing_object	= _("LOADING OBJECT . . .");
const char *STR_MSG_Rendering_snapshot	= _("RENDERING . . .");
/* %d == percent of snapshot written */
const char *STR_MSG_Saving_snapshot_ARG	= _("SAVING SNAPSHOT . . .\n(%d%%)");

/* end lstrings.c */
