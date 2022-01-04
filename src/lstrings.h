/* lstrings.h */

/* Language-specific strings */

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


extern const char *STR_Light_Speed;
extern const char *STR_copyright_ARG;
extern const char *STR_CLI_usage_ARG;
extern struct option_desc STRS_CLI_options[];
extern const char *STR_CLI_option_chars;
extern const char *STR_MNU_File;
extern const char *STR_MNU_Objects;
extern const char *STR_MNU_Warp;
extern const char *STR_MNU_Camera;
extern const char *STR_MNU_Help;
extern const char *STR_MNU_New_lattice;
extern const char *STR_MNU_Load_object;
extern const char *STR_MNU_Save_snapshot;
extern const char *STR_MNU_Export_srs;
extern const char *STR_MNU_Exit;
extern const char *STR_MNU_Coordinate_axes;
extern const char *STR_MNU_Floating_grid;
extern const char *STR_MNU_Bounding_box;
extern const char *STR_MNU_Animation;
extern const char *STR_MNU_Lorentz_contraction;
extern const char *STR_MNU_Doppler_shift;
extern const char *STR_MNU_Headlight_effect;
extern const char *STR_MNU_Optical_deformation;
extern const char *STR_MNU_Lens;
extern const char *STR_MNU_Position;
extern const char *STR_MNU_Reset_view;
extern const char *STR_MNU_Info_display;
extern const char *STR_MNU_Background;
extern const char *STR_MNU_Graphics_mode;
extern const char *STR_MNU_Spawn_camera;
extern const char *STR_MNU_Close;
extern const char *STR_MNU_Custom;
extern const char *STR_MNU_Active;
extern const char *STR_MNU_Velocity;
extern const char *STR_MNU_Time_t;
extern const char *STR_MNU_Gamma_factor;
extern const char *STR_MNU_Framerate;
extern const char *STRS_MNU_bkgd_color_names[];
extern const char *STR_MNU_Wireframe;
extern const char *STR_MNU_Shaded;
extern const char *STR_MNU_Overview;
extern const char *STR_MNU_Controls;
extern const char *STR_MNU_About;
extern const char *STR_INF_time_ARG;
extern const char *STR_INF_fps_ARG;
extern const char *STR_INF_velocity_ARG;
extern const char *STR_INF_gamma_ARG;
extern const char *STR_INF_no_contraction;
extern const char *STR_INF_no_doppler_shift;
extern const char *STR_INF_no_headlight_effect;
extern const char *STR_INF_no_deformation;
extern const char *STR_INF_no_relativity;
extern const char *STR_DLG_Okay_btn;
extern const char *STR_DLG_Cancel_btn;
extern const char *STR_DLG_Close_btn;
extern const char *STR_DLG_New_lattice;
extern const char *STR_DLG_Dimensions;
extern const char *STR_DLG_Smoothness;
extern const char *STR_DLG_Load_Object;
extern const char *STR_DLG_formats_all;
extern const char *STR_DLG_formats_3ds;
extern const char *STR_DLG_formats_lwo;
extern const char *STR_DLG_Save_Snapshot;
extern const char *STR_DLG_snapshot_Parameters;
extern const char *STR_DLG_snapshot_Size;
extern const char *STR_DLG_snapshot_Format;
extern const char *STR_DLG_snapshot_basename;
extern const char *STR_DLG_Export_srs;
extern const char *STR_DLG_srs;
extern const char *STR_DLG_srs_Parameters;
extern const char *STR_DLG_srs_Size;
extern const char *STR_DLG_srs_Stereo_view;
extern const char *STR_DLG_srs_Vis_faces_only;
extern const char *STR_DLG_srs_basename1;
extern const char *STR_DLG_srs_basename2;
extern const char *STR_DLG_Animation;
extern const char *STR_DLG_Observed_range;
extern const char *STR_DLG_Start_X;
extern const char *STR_DLG_End_X;
extern const char *STR_DLG_Loop_time;
extern const char *STR_DLG_seconds;
extern const char *STR_DLG_Begin_btn;
extern const char *STR_DLG_Stop_btn;
extern const char *STR_DLG_Camera_Position;
extern const char *STR_DLG_Location;
extern const char *STR_DLG_View_target;
extern const char *STR_DLG_Direction;
extern const char *STR_DLG_Phi_label;
extern const char *STR_DLG_Theta_label;
extern const char *STR_DLG_Angles_instead;
extern const char *STR_DLG_Xyz_instead;
extern const char *STR_DLG_Reposition_btn;
extern const char *STR_DLG_Custom_Lens;
extern const char *STR_DLG_Lens_length;
extern const char *STR_DLG_Field_of_view;
extern const char *STR_DLG_degree_suffix;
extern const char *STR_DLG_Overview;
extern const char *STR_DLG_Controls;
extern const char *STR_DLG_About;
extern const char *STR_DLG_Version_x_y_ARG;
extern const char *STR_DLG_authorship_ARG;
extern const char *STR_DLG_home_page_url;
extern const char *STR_DLG_Camera;
extern const char *STR_DLG_Warning;
extern const char *STR_DLG_Error;
extern const char *STR_DLG_Overview_TEXT;
extern const char *STR_DLG_Controls_TEXT;
extern const char *STR_MSG_Okay;
extern const char *STR_MSG_Invalid;
extern const char *STR_MSG_overwrite_warn_ARG;
extern const char *STR_MSG_no_object_file_ARG;
extern const char *STR_MSG_not_3ds_file;
extern const char *STR_MSG_not_prj_file;
extern const char *STR_MSG_not_lwo_file;
extern const char *STR_MSG_unknown_obj_format;
extern const char *STR_MSG_bad_3ds_file;
extern const char *STR_MSG_empty_3ds_file;
extern const char *STR_MSG_bad_lwo_file;
extern const char *STR_MSG_empty_lwo_file;
extern const char *STR_MSG_no_ogl_visual;
extern const char *STR_MSG_no_render_buf_ARG;
extern const char *STR_MSG_no_ogl_context;
extern const char *STR_MSG_no_snapshot_output;
extern const char *STR_MSG_Generating_lattice;
extern const char *STR_MSG_Importing_object;
extern const char *STR_MSG_Rendering_snapshot;
extern const char *STR_MSG_Saving_snapshot_ARG;

/* end lstrings.h */
