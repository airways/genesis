/*
// ColdMUD was created and is copyright 1993, 1994 by Greg Hudson
//
// Genesis is a derivitive work, and is copyright 1995 by Brandon Gillespie.
// Full details and copyright information can be found in the file doc/CREDITS
//
// File: include/cache.h
// ---
// Declarations for the object cache.
*/

#ifndef _cache_h_
#define _cache_h_

#include "object.h"

#ifndef _cache_
extern int cache_width;
extern int cache_depth;
#else
int cache_width = 10;
int cache_depth = 30;
#endif

void init_cache(void);
object_t *cache_get_holder(long objnum);
object_t *cache_retrieve(long objnum);
object_t *cache_grab(object_t *object);
void cache_discard(object_t *obj);
int cache_check(long objnum);
void cache_sync(void);
object_t *cache_first(void);
object_t *cache_next(void);
void cache_sanity_check(void);
void cache_cleanup(void);

#endif

