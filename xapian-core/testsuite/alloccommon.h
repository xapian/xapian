/* alloccommon.h: common definitions for the malloc and new tracking
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_ALLOCCOMMON_H
#define OM_HGUARD_ALLOCCOMMON_H

#ifdef __cplusplus
#include <iosfwd>
#include <cstdlib>
extern "C" {
#else
#include <stdlib.h>
#endif

/* The maximum number of allocations which can be tracked
 * This is a macro because in C you can't use a static const
 * as an array size.
 */
#define MAX_ALLOCATIONS 1000000
static const int max_allocations = MAX_ALLOCATIONS;

extern struct allocation_data allocdata;

/* the structure used to describe an allocation */
struct allocation_info {
    void *p;
    unsigned long id;    
    size_t size;
};

/* The full allocation state */
struct allocation_data {
    long num_allocations;
    long allocations_bound;
    unsigned long id;
    struct allocation_info allocations[MAX_ALLOCATIONS];
};

#define ALLOC_DATA_INIT { 0, 0, 0 }

/************ Functions for use of allocator functions **********/

/** Register an allocation in the table. */
void
handle_allocation(void *address, size_t size);

enum dealloc_result {
    alloc_ok,
    alloc_notfound
};

/** Replace an existing allocation in the table.
 */
enum dealloc_result
handle_reallocation(void *old_address, void *new_address, size_t size);

/** Remove an entry from the allocation table.  Returns nonzero if something
 *  is wrong (ie alloc_notfound).
 */
enum dealloc_result
handle_deallocation(void *address);

void * checked_malloc(size_t size);

void * checked_calloc(size_t nmemb, size_t size);

void * checked_realloc(void *ptr, size_t size);

void checked_free(void *ptr, const char *msg);
    
#ifdef __cplusplus
}  // from extern "C"
#endif

#endif /* OM_HGUARD_ALLOCCOMMON_H */
