/* alloccommon.c: implementation of functions related to allocdata
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#define _GNU_SOURCE /* glibc 2.2 needs this to give us RTLD_NEXT */

#include "alloccommon.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

/************ Functions for use of allocator functions **********/

struct allocation_data malloc_allocdata = ALLOC_DATA_INIT;

static void *(*real_malloc)(size_t) = 0;
static void *(*real_calloc)(size_t, size_t) = 0;
static void (*real_free)(void *) = 0;
static void *(*real_realloc)(void *, size_t) = 0;

static int have_symbols = 0;
static int in_get_symbols = 0;

/** bookkeeping data for the malloc traps */
static unsigned long malloc_trap_count = 0;

static void
get_symbols()
{
    if (in_get_symbols) {
	fprintf(stderr,
		"get_symbols() being called before exiting!\n");
	abort();
    }
    in_get_symbols = 1;

    if (real_malloc == 0) {
	real_malloc = dlsym(RTLD_NEXT, "malloc");
    }
    if (real_realloc == 0) {
	real_realloc = dlsym(RTLD_NEXT, "realloc");
    }
    if (real_calloc == 0) {
	real_calloc = dlsym(RTLD_NEXT, "calloc");
    }
    if (real_free == 0) {
	real_free = dlsym(RTLD_NEXT, "free");
    }
    if (real_malloc && real_realloc &&
	real_calloc && real_free) {
	have_symbols = 1;
    } else {
	fprintf(stderr, "get_symbols(): can't get symbols for malloc and friends\n");
	abort();
    }
    {
	/** Handle OM_ALLOC_TRAP */
	const char *count = getenv("OM_ALLOC_TRAP");
	if (count) malloc_trap_count = atol(count);
    }

    in_get_symbols = 0;
}

#define CHECK_SYMBOLS if (have_symbols) ; else get_symbols()

/** naive_allocator is used to handle memory requests from anything that
 *  dlsym() calls, since we won't yet have access to the real malloc() etc.
 *  by then.
 */
static void *
naive_allocator(size_t size)
{
    void *result;
    int fd = open("/dev/zero", 0);
    if (fd < 0) return 0;
    result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
		  fd, 0);

    close(fd);
    return result;
}

#define HANDLE_MALLOC_TRAP(address) \
	if (malloc_trap_count != 0) {\
	    --malloc_trap_count; \
            if (malloc_trap_count == 0) abort();\
	}

/** Register an allocation in the table.
 */
void
handle_allocation(struct allocation_data *data,
		  void *address,
		  size_t size)
{
    fprintf(stderr, "handle_allocation(%p, %d)\n", address, size);
    if (!address) {
	fprintf(stderr, "Attempt to register an allocation of NULL\n");
	abort();
    }
    if (data->allocations_bound >= max_allocations) {
	/* our array is too small - panic! */
	fprintf(stderr, "Ran out of room for memory tracking!\n");
	abort();
    } else {
	data->allocations[data->allocations_bound].p = address;
	data->allocations[data->allocations_bound].size = size;
	data->allocations[data->allocations_bound].id = ++data->id;
	++data->allocations_bound;
	++data->num_allocations;
    }
}

enum dealloc_result
handle_reallocation(struct allocation_data *data,
		    void *old_address, void *new_address,
		    size_t size)
{
    int found_it = 0;
    int i;
    fprintf(stderr, "handle_reallocation(%p, %p, %d)\n",
	    old_address, new_address, size);
    if (!old_address || !new_address) {
	fprintf(stderr, "Attempt to register a reallocation with NULL\n");
	abort();
    }
    for (i = data->allocations_bound - 1;
	 i >= 0;
	 --i) {
	if (data->allocations[i].p == old_address) {
	    data->allocations[i].p = new_address;
	    data->allocations[i].size = size;
	    found_it = 1;

	    break;
	}
    }
    if (!found_it) {
	/* The caller should report the error as appropriate */
	handle_allocation(data, new_address, size);
	return alloc_notfound;
    }
    return alloc_ok;
}

/** Remove an entry from the allocation table.  Returns nonzero if something
 *  is wrong (ie alloc_notfound).
 */
enum dealloc_result
handle_deallocation(struct allocation_data *data,
		    void *address)
{
    int found_it = 0;
    int i;
    fprintf(stderr, "handle_deallocation(%p)\n", address);
    if (!address) {
	fprintf(stderr, "Attempt to register a deallocation of NULL\n");
	abort();
    }
    for (i = data->allocations_bound - 1;
	 i >= 0;
	 --i) {
	if (data->allocations[i].p == address) {
	    data->allocations[i].p = 0;
	    --data->num_allocations;
	    found_it = 1;

	    /* lower allocations_bound if possible */
	    if (i == (data->allocations_bound - 1)) {
		while (data->allocations_bound > 0 &&
		       data->allocations[data->allocations_bound-1].p == 0) {
		    data->allocations_bound--;
		}
	    }
	    break;
	}
    }
    if (!found_it) {
	/* The caller should report the error as appropriate */
#if 0
	/* note: we can use C-style I/O, but nothing C++ish in
	 * case new is needed.
	 */
	fprintf(stderr,
		"Trying to delete %p which wasn't allocated with this allocator\n",
		address);
#endif
	return alloc_notfound;
    }
    return alloc_ok;
}

void *
checked_malloc(size_t size)
{
    void *result;
    fprintf(stderr, "checked_malloc(%d)\n", size);
    if (in_get_symbols) {
	return naive_allocator(size);
    }
    CHECK_SYMBOLS;

    result = real_malloc(size);
    HANDLE_MALLOC_TRAP(result);
    if (result) {
	handle_allocation(&malloc_allocdata, result, size);
    }
    return result;
}

void
checked_free(void *ptr, const char *msg)
{
    fprintf(stderr, "checked_free(%p)\n", ptr);
    CHECK_SYMBOLS;
    if (!ptr) return;
    if (handle_deallocation(&malloc_allocdata, ptr) != alloc_ok) {
	fprintf(stderr, msg, ptr);
	abort();
    }
    real_free(ptr);
}

void *
calloc(size_t nmemb, size_t size)
{
    void *result;
    if (in_get_symbols) {
	return naive_allocator(size * nmemb);;
    }
    CHECK_SYMBOLS;

    result = real_calloc(nmemb, size);
    HANDLE_MALLOC_TRAP(result);
    if (result) {
	handle_allocation(&malloc_allocdata, result, size * nmemb);
    }
    return result;
}

void *
realloc(void *ptr, size_t size)
{
    void *result;
    CHECK_SYMBOLS;

    result = real_realloc(ptr, size);
    if (ptr == 0 && size > 0) {
	/* equivalent to malloc(size) */
	if (result) {
	    HANDLE_MALLOC_TRAP(result);
	    handle_allocation(&malloc_allocdata, result, size);
	}
    } else if (size == 0) {
	if (ptr != 0) {
	    /* equivalent to free(ptr) */
	    if (handle_deallocation(&malloc_allocdata, ptr) != alloc_ok) {
		fprintf(stderr,
			"realloc()ing memory at %p to 0 which wasn't malloc()ed!\n",
			ptr);
	    }
	}
    } else {
	HANDLE_MALLOC_TRAP(result);
	if (handle_reallocation(&malloc_allocdata,
				ptr, result, size) != alloc_ok) {
	    fprintf(stderr,
		    "realloc()ing memory at %p to %d which wasn't malloc()ed!\n",
		    ptr, size);
	    abort();
	}
    }
    return result;
}
