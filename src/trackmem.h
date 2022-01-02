/* trackmem.h */

/* Memory allocation accounting utility */

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


#include <stdio.h>
#include <malloc.h>
#include <string.h>

enum {
	TRACKMEM_MALLOC,
	TRACKMEM_MALLOC_EQUIV,
	TRACKMEM_REALLOC,
	TRACKMEM_STRDUP,
	TRACKMEM_FREE,
	TRACKMEM_SHOW_STATS,
	TRACKMEM_SHOW_BLOCKS,
	TRACKMEM_QUIT
};

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Some systems don't support malloc() hooks */
#ifdef MALLOC_HOOKS
static void *(*prev_malloc_hook)( size_t size );
static void *(*prev_realloc_hook)( void *block, size_t size );
static void (*prev_free_hook)( void *block );
static void *trackmem_malloc_hook( size_t size );
static void *trackmem_realloc_hook( void *block, size_t size );
static void trackmem_free_hook( void *block );
#endif

void trackmem_init( void );
void trackmem_stop( void );
void trackmem_show_stats( void );
void trackmem_show_blocks( void );
void trackmem( int message, void *block, void *block2, size_t size );
int trackmem_search_for_block( void *block, void **allblocks, int num_blocks );
void trackmem_malloc( void *block, size_t size );
void trackmem_realloc( void *block, void *block2, size_t size );
void trackmem_strdup( char *string );
void trackmem_free( void *block );

/* end trackmem.h */
