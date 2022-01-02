/* animation.c */

/* Main update loop + transition/animation handlers */

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


/* Forward declarations */
static int idle_loop( void );
static int transition_engine( trans_var *new_tvar );


/* Queue a redraw, i.e. flag a camera [viewport] as needing a redraw
 * Pass a value of -1 to redraw everything */
void
queue_redraw( int cam_id )
{
	int i;

	if (cam_id < 0) {
		for (i = 0; i < num_cams; i++)
			usr_cams[i]->redraw = TRUE;
	}
	else
		usr_cams[cam_id]->redraw = TRUE;

	/* Idle loop will bring about the actual redraws */
	update( ACTIVATE );
}


/* This function keeps everything moving and up-to-date */
int
update( int message )
{
	static int active = FALSE;
	int state_change;
	int redraw_occurred = FALSE;
	int redraw_deferred = FALSE;
	int i;

	switch( message ) {
	case ITERATION:
		/* we're being called from idle_loop( ) */
		break;

	case ACTIVATE:
		if (active)
			return FALSE; /* already running */
		active = TRUE;
		/* Hook our idle loop in */
		gtk_idle_add( (GtkFunction)idle_loop, NULL );
		return TRUE;

	default:
#ifdef DEBUG
		crash( "update( ): invalid message" );
#endif
		return FALSE;
	}

	/* Begin timing */
	profile( PROFILE_START_ITERATION );
	profile( PROFILE_FRAME_BEGIN );

	/* Update all variables in transitory status */
	state_change = transition_engine( NULL );

	/* Do pending redraws (if any) ONLY if event queue is empty */
	if (gtk_events_pending( ) == 0) {
		for (i = 0; i < num_cams; i++)
			if (usr_cams[i]->redraw) {
				camera_calc_xyz( CAM_POSITION, usr_cams[i] );
				ogl_draw( i );
				redraw_occurred = TRUE;
			}

#if VELOCITY_SLIDER
		/* Update velocity slider */
		velocity_slider( NULL, MESG_(RESET) );
#endif
	}
	else
		redraw_deferred = TRUE;

	/* Semi-BUG: Framerate goes artificially high if only spawned cameras are moved
	 * (remember that moving the primary camera updates *all* views) */
	if (redraw_occurred)
		profile( PROFILE_FRAME_DONE );

	if (!state_change && !redraw_deferred) {
		active = FALSE; /* no state change, no pending redraw */
		profile( PROFILE_IDLE );
	}

	return active;
}


/* "Lackey" idle loop for update( )
 * Remember, returning TRUE makes loop run again, returning FALSE stops it... */
static int
idle_loop( void )
{
	return update( ITERATION );
}


/* Performance profiler, keeps track of framerate and other timely stats */
void
profile( int message )
{
	static double main_t0;
	static double iteration_t0 = -1.0;
	static double active_total_t = 0.0;
	static double frame_t0;
	static double *frametimes = NULL;
	static double sum_frametimes;
	static double warp_t0;
	static double warp_total_t = 0.0;
	static double ogldraw_t0;
	static double ogldraw_total_t = 0.0;
	static int num_frametimes;
	static int frame_num = -1;
	double delta_t;
	double cur_t;
	double p;
	int i;

	cur_t = read_system_clock( );

	switch (message) {
	case INITIALIZE:
		main_t0 = cur_t;
		break;

	case PROFILE_START_ITERATION:
		if (iteration_t0 > 0.0) {
			delta_t = cur_t - iteration_t0;
			active_total_t += delta_t;
		}
		iteration_t0 = cur_t;
		break;

	case PROFILE_IDLE:
		delta_t = cur_t - iteration_t0;
		active_total_t += delta_t;
		iteration_t0 = -1.0;
		break;

	case PROFILE_FRAME_BEGIN:
		frame_t0 = cur_t;
		break;

	case PROFILE_FRAME_DONE:
		delta_t = cur_t - frame_t0;
		if (frame_num == -1) {
			/* (Re)initialize frametime buffer */
			num_frametimes = (int)((FRAMERATE_AVERAGE_TIME + 0.5) / delta_t);
			num_frametimes = MAX(4, num_frametimes);
			frametimes = xrealloc( frametimes, num_frametimes * sizeof(double) );
			for (i = 0; i < num_frametimes; i++)
				frametimes[i] = delta_t;
			sum_frametimes = (double)num_frametimes * delta_t;
			frame_num = 0;
		}
		/* Calculate framerate */
		sum_frametimes -= frametimes[frame_num];
		frametimes[frame_num] = delta_t;
		sum_frametimes += delta_t;
		framerate = (double)num_frametimes / sum_frametimes;
		/* Adjust frametime buffer length (if necessary) */
		while (sum_frametimes > (FRAMERATE_AVERAGE_TIME + 1.0)) {
			if (num_frametimes <= 4)
				break;
			/* Shrink frametime buffer (animation is slowing down) */
			delta_t = sum_frametimes / (double)num_frametimes;
			--num_frametimes;
			frametimes = xrealloc( frametimes, num_frametimes * sizeof(double) );
			for (i = 0; i < num_frametimes; i++)
				frametimes[i] = delta_t;
			sum_frametimes = (double)num_frametimes * delta_t;
		}
		while (sum_frametimes < FRAMERATE_AVERAGE_TIME) {
			/* Expand frametime buffer (animation is speeding up) */
			delta_t = sum_frametimes / (double)num_frametimes;
			++num_frametimes;
			frametimes = xrealloc( frametimes, num_frametimes * sizeof(double) );
			for (i = 0; i < num_frametimes; i++)
				frametimes[i] = delta_t;
			sum_frametimes = (double)num_frametimes * delta_t;
		}
		frame_num = (frame_num + 1) % num_frametimes;
		break;

	case PROFILE_FRAMERATE_RESET:
		/* (Re)initialize frametime buffer on next completed frame */
		frame_num = -1;
		break;

	case PROFILE_WARP_BEGIN:
		warp_t0 = cur_t;
		break;

	case PROFILE_WARP_DONE:
		delta_t = cur_t - warp_t0;
		warp_total_t += delta_t;
		break;

	case PROFILE_OGLDRAW_BEGIN:
		ogldraw_t0 = cur_t;
		break;

	case PROFILE_OGLDRAW_DONE:
		delta_t = cur_t - ogldraw_t0;
		ogldraw_total_t += delta_t;
		break;

	case PROFILE_SHOW_STATS:
		printf( "==== Light Speed! profiler stats ====\n" );
		delta_t = cur_t - main_t0;
		printf( "Idle time..: %.3f sec\n", delta_t - active_total_t );
		printf( "Active time: %.3f sec (100%%)\n", active_total_t );
		p = 100.0 * warp_total_t / active_total_t;
		printf( "Warp engine: %.3f sec (%.2f%%)\n", warp_total_t, p );
		p = 100.0 * ogldraw_total_t / active_total_t;
		printf( "OpenGL draw: %.3f sec (%.2f%%)\n", ogldraw_total_t, p );
		printf( "Framerate..: %.3f fps\n", framerate );
		printf( "=====================================\n" );
		fflush( stdout );
		break;

	default:
#ifdef DEBUG
		crash( "profile( ): invalid message" );
#endif
		return;
	}
}


/* Convenience function to initiate a variable transition */
void
transition( void *var, int is_double, int trans_type, double duration, double final, int cam_id )
{
	trans_var *tvar;
	double t;

	t = read_system_clock( );

	tvar = xmalloc( sizeof(trans_var) );
	tvar->var = var;
	tvar->is_double = is_double;
	tvar->looping = FALSE;
	tvar->type = trans_type;
	tvar->start_t = t;
	tvar->end_t = t + duration;
	tvar->cam_id = cam_id;
	if (is_double)
		tvar->initial = *((double *)var);
	else
		tvar->initial = *((float *)var);
	tvar->final = final;
	transition_engine( tvar );
}


/* Same as earlier, only this starts a looping variable transition
 * (i.e. continuous, run-until-you-tell-it-to-stop animation) */
void
animate( void *var, int is_double, int trans_type, double duration, double initial, double final, int cam_id )
{
	trans_var *tvar;
	double t, val;

	t = read_system_clock( );

	tvar = xmalloc( sizeof(trans_var) );
	tvar->var = var;
	tvar->is_double = is_double;
	tvar->looping = TRUE;
	tvar->type = trans_type;
	/* Define starting time so animation debuts at current value (*var) */
	if (is_double)
		val = *((double *)var);
	else
		val = *((float *)var);
	tvar->start_t = t - duration * (val - initial) / (final - initial);
	tvar->end_t = tvar->start_t + duration;
	tvar->cam_id = cam_id;
	tvar->initial = initial;
	tvar->final = final;
	transition_engine( tvar );
}


/* Stops an ongoing transition/animation
 * (does nothing if the specified variable isn't being transitioned) */
void
break_transition( void *var )
{
	trans_var stop_tvar;

	stop_tvar.var = var;
	stop_tvar.type = TRANS_STOP;
	transition_engine( &stop_tvar );
}


/* This function is what makes dynamic variable transitioning happen!
 * Return status indicates whether any state change occurred or not */
static int
transition_engine( trans_var *new_tvar )
{
	static trans_var *trans_queue = NULL;
	trans_var *prev_tvar;
	trans_var *u;
	trans_var *next_tvar;
	double cur_t;
	double delta_t;
	double val;
	double percent;
	int redraw_all = FALSE;
	int i;

	if (new_tvar != NULL) {
		/* New transition variable is entering the queue */
		/* Check to see if it's already in there */
		prev_tvar = NULL;
		u = trans_queue;
		while (u != NULL) {
			if (new_tvar->var == u->var) {
				/* Whoa! This variable is already in transition
				 * Change existing transition record */
				if (new_tvar->type == TRANS_STOP) {
					/* Remove variable from queue */
					next_tvar = u->next;
					if (prev_tvar == NULL) /* i.e. u is 1st in queue */
						trans_queue = next_tvar;
					else
						prev_tvar->next = next_tvar;
					break;
				}

				/* Alter the existing transition */
				u->type = new_tvar->type;
				u->start_t = new_tvar->start_t;
				u->end_t = new_tvar->end_t;
				if (u->is_double)
					u->initial = *((double *)u->var);
				else
					u->initial = *((float *)u->var);
				u->final = new_tvar->final;
				u->looping = new_tvar->looping;
				xfree( new_tvar ); /* Won't need this again */
				break;
			}
			prev_tvar = u;
			u = u->next; /* cdr down the list */
		}

		/* (u is NULL if no match occurred) */
		if ((u == NULL) && (new_tvar->type != TRANS_STOP)) {
			/* All clear, add new_tvar to head of transition queue */
			new_tvar->next = trans_queue;
			/* and fire up the idle loop if it's the first one */
			if (trans_queue == NULL)
				update( ACTIVATE );
			trans_queue = new_tvar;
		}

		/* Done for now, don't do actual state change in this iteration */
		return FALSE;
	}

	/* Double-check that the inbox isn't empty */
	if (trans_queue == NULL)
		return FALSE;

	cur_t = read_system_clock( );

	/* Perform incremental update of all variables in the queue */
	prev_tvar = NULL;
	u = trans_queue;
	while (u != NULL) {
		/* First, set camera update flag(s) as appropriate
		 * (don't use queue_redraw( ) here! bad!) */
		if (u->cam_id >= 0)
			usr_cams[u->cam_id]->redraw = TRUE;
		else
			redraw_all = TRUE; /* affects all views */

		if (cur_t >= u->end_t) {
			if (u->looping) {
				/* start over */
				delta_t = u->end_t - u->start_t;
				u->start_t += delta_t;
				u->end_t += delta_t;
			}
			else {
				/* Transition complete, remove tvar from queue */
				if (u->is_double)
					*((double *)u->var) = u->final;
				else
					*((float *)u->var) = u->final;
				next_tvar = u->next;
				if (prev_tvar == NULL) /* i.e. u is 1st in queue */
					trans_queue = next_tvar;
				else
					prev_tvar->next = next_tvar;
				xfree( u );
				u = next_tvar;
				continue;
			}
		}

		/* Update variable value as per appropriate transition function */
		percent = ((cur_t - u->start_t) / (u->end_t - u->start_t));
		switch( u->type ) {
		case TRANS_LINEAR: /* No remapping */
			break;

		case TRANS_QTR_SIN: /* 1/4 sine: starts fast, finishes slow */
			percent = sin( PI * percent / 2 );
			break;

		case TRANS_RAMP: /* Ramp: starts slow, finishes fast */
			percent = 1 - cos( PI * percent / 2 );
			break;

		case TRANS_SIGMOID: /* Sigmoidal (S-like) remapping */
			percent = (1 - cos( PI * percent )) / 2;
			break;

		default:
#ifdef DEBUG
			crash( "transition_engine( ): invalid transition type" );
#endif
			break;
		}

		val = u->initial + percent * (u->final - u->initial);
		if (u->is_double)
			*((double *)u->var) = val;
		else
			*((float *)u->var) = val;

		prev_tvar = u;
		u = u->next;
	}

	/* Don't use queue_redraw( ) here either */
	if (redraw_all) {
		for (i = 0; i < num_cams; i++)
			usr_cams[i]->redraw = TRUE;
	}

	return TRUE;
}


/* Thoughts: The above system could be extended to allow chained transitions
 * (i.e. when a transition ends, another is automatically started). This
 * can be implemented by adding a "trans_var *chain_queue" field to the
 * trans_var structure, and amending the code that terminates transitions.
 * This would also allow for a somewhat clever way of specifying looping
 * animations-- have chain_queue point to the very structure it is in }:) */

/* end animation.c */
