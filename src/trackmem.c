/* trackmem.h */

/* Memory allocation accounting utilities
 * This is what I used to nail down memory leaks :) */

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


#include "trackmem.h"


#ifdef TRACKMEM_VERBOSE
static int trackmem_verbose = TRUE;
#else
static int trackmem_verbose = FALSE;
#endif


#ifdef MALLOC_HOOKS

/* Use this if you want to track memory use via malloc() hooks
 * (you might not want to have TRACKMEM_VERBOSE set... it gets awful spammy) */
void
trackmem_init( void )
{
	fprintf( stderr, "trackmem_init() says hello\n" );

	prev_malloc_hook = __malloc_hook;
	__malloc_hook = trackmem_malloc_hook;

	prev_realloc_hook = __realloc_hook;
	__realloc_hook = trackmem_realloc_hook;

	prev_free_hook = __free_hook;
	__free_hook = trackmem_free_hook;

	fprintf( stderr, "malloc() hooks are in place\n" );
	fflush( stderr );
}

static void *
trackmem_malloc_hook( size_t size )
{
	void *block;

	__malloc_hook = prev_malloc_hook;
	__realloc_hook = prev_realloc_hook;
	__free_hook = prev_free_hook;

	block = malloc( size );
	trackmem( TRACKMEM_MALLOC, block, NULL, size );

	__free_hook = trackmem_free_hook;
	__realloc_hook = trackmem_realloc_hook;
	__malloc_hook = trackmem_malloc_hook;

	return block;
}

static void *
trackmem_realloc_hook( void *block, size_t size )
{
	void *block2;

	__malloc_hook = prev_malloc_hook;
	__realloc_hook = prev_realloc_hook;
	__free_hook = prev_free_hook;

	block2 = realloc( block, size );
	trackmem( TRACKMEM_REALLOC, block, block2, size );

	__free_hook = trackmem_free_hook;
	__realloc_hook = trackmem_realloc_hook;
	__malloc_hook = trackmem_malloc_hook;

	return block;
}

static void
trackmem_free_hook( void *block )
{
	__malloc_hook = prev_malloc_hook;
	__realloc_hook = prev_realloc_hook;
	__free_hook = prev_free_hook;

	trackmem( TRACKMEM_FREE, block, NULL, 0 );
	free( block );

	__free_hook = trackmem_free_hook;
	__realloc_hook = trackmem_realloc_hook;
	__malloc_hook = trackmem_malloc_hook;
}

void
trackmem_stop( void )
{
	__malloc_hook = prev_malloc_hook;
	__realloc_hook = prev_realloc_hook;
	__free_hook = prev_free_hook;
	trackmem( TRACKMEM_QUIT, NULL, NULL, 0 );
}

#endif /* MALLOC_HOOKS */


void
trackmem_show_stats( void )
{
	trackmem( TRACKMEM_SHOW_STATS, NULL, NULL, 0 );
}

void
trackmem_show_blocks( void )
{
	trackmem( TRACKMEM_SHOW_BLOCKS, NULL, NULL, 0 );
}

void
trackmem( int message, void *block, void *block2, size_t size )
{
	static void **allblocks = NULL;
	static int *allsizes = NULL;
	static int num_blocks = 0;
	static int total_size = 0;
	int verbose = trackmem_verbose;
	int block_id;

	switch (message) {
	case TRACKMEM_MALLOC:
		if (verbose) fprintf( stderr, "%p   ALLOC: +%d ", block, size );
	case TRACKMEM_MALLOC_EQUIV:
		if (size == 0)
			fprintf( stderr, "[BUG! Allocating zero-size block]" );
		++num_blocks;
		allblocks = realloc( allblocks, num_blocks * sizeof(void *) );
		allblocks[num_blocks - 1] = block;
		allsizes = realloc( allsizes, num_blocks * sizeof(int) );
		allsizes[num_blocks - 1] = (int)size;
		total_size += (int)size;
		if (message == TRACKMEM_MALLOC_EQUIV)
			return;
		break;

	case TRACKMEM_REALLOC:
		if (verbose) fprintf( stderr, "%p REALLOC: ", block2 );
		if (block == NULL) {
			if (verbose) fprintf( stderr, "+%d (from NULL)", size );
			trackmem( TRACKMEM_MALLOC_EQUIV, block2, NULL, size );
		}
		else {
			block_id = trackmem_search_for_block( block, allblocks, num_blocks );
			if (block_id == -1) {
				fprintf( stderr, "+%d [BUG! Reallocating unknown block]\n", size );
				fflush( stderr );
				return;
			}
			else {
				if (verbose) fprintf( stderr, "%+d (from %p)", size - allsizes[block_id], block );
				allblocks[block_id] = block2;
				total_size += (size - allsizes[block_id]);
				allsizes[block_id] = (int)size;
			}
		}
		break;

	case TRACKMEM_STRDUP:
		size = strlen( (char *)block ) + 1;
		if (verbose) fprintf( stderr, "%p  STRDUP: +%d", block, size );
		trackmem( TRACKMEM_MALLOC_EQUIV, block, NULL, size );
		break;

	case TRACKMEM_FREE:
		if (verbose) fprintf( stderr, "%p    FREE: ", block );
		block_id = trackmem_search_for_block( block, allblocks, num_blocks );
		if (block == NULL) {
			fprintf( stderr, "[BUG! Attempting to free null block]\n" );
			fflush( stderr );
			return;
		}
		else if (block_id == -1) {
			fprintf( stderr, "[BUG! Attempting to free unknown block]\n" );
			fflush( stderr );
			return;
		}
		else {
			total_size -= allsizes[block_id];
			if (verbose) fprintf( stderr, "-%d", allsizes[block_id] );
			--num_blocks;
			if (block_id != num_blocks) {
				memmove( &allblocks[block_id], &allblocks[block_id + 1], (num_blocks - block_id) * sizeof(void *) );
				memmove( &allsizes[block_id], &allsizes[block_id + 1], (num_blocks - block_id) * sizeof(int) );
			}
			allblocks = realloc( allblocks, num_blocks * sizeof(void *) );
			allsizes = realloc( allsizes, num_blocks * sizeof(int) );
		}
		break;

	case TRACKMEM_SHOW_STATS:
		fprintf( stderr, "Total: %d blocks (%d bytes)\n", num_blocks, total_size );
		fflush( stdout );
		return;

	case TRACKMEM_SHOW_BLOCKS:
		fprintf( stderr, "==== Current TRACKMEM database ====\n" );
		for (block_id = 0; block_id < num_blocks; block_id++)
			fprintf( stderr, "  %p: %d bytes\n", allblocks[block_id], allsizes[block_id] );
		trackmem( TRACKMEM_SHOW_STATS, NULL, NULL, 0 );
		fprintf( stderr, "===================================\n" );
		fflush( stdout );
		return;

	case TRACKMEM_QUIT:
		free( allblocks );
		free( allsizes );
		num_blocks = 0;
		total_size = 0;
		return;
	}

	if (verbose) {
		fprintf( stderr, "    " );
		trackmem( TRACKMEM_SHOW_STATS, NULL, NULL, 0 );
	}
}

int
trackmem_search_for_block( void *block, void **allblocks, int num_blocks )
{
	int i;

	for (i = 0; i < num_blocks; i++)
		if (block == allblocks[i])
			return i;
	return -1;
}

/* Alternate interface-- use inside malloc()/realloc()/etc. wrappers */

void
trackmem_malloc( void *block, size_t size )
{
	trackmem( TRACKMEM_MALLOC, block, NULL, size );
}

void
trackmem_realloc( void *block, void *block2, size_t size )
{
	trackmem( TRACKMEM_REALLOC, block, block2, size );
}

void
trackmem_strdup( char *string )
{
	trackmem( TRACKMEM_STRDUP, (void *)string, NULL, 0 );
}

void
trackmem_free( void *block )
{
	trackmem( TRACKMEM_FREE, block, NULL, 0 );
}

/* end trackmem.h */
