/* malloccheck.c: a checking malloc() etc. wrapper
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

#include "allocdata.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

static struct allocation_data allocdata = ALLOC_DATA_INIT;

static void *(*real_malloc)(size_t) = 0;
static void *(*real_calloc)(size_t, size_t) = 0;
static void (*real_free)(void *) = 0;
static void *(*real_realloc)(void *, size_t) = 0;
static const char *(*real_dlerror)() = 0;
static int have_symbols = 0;

static int in_get_symbols = 0;

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

void *
malloc(size_t size)
{
    void *result;
    if (in_get_symbols) {
	return naive_allocator(size);;
    }
    CHECK_SYMBOLS;

    result = real_malloc(size);
    handle_allocation(&allocdata, result, size);
    return result;
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
    handle_allocation(&allocdata, result, size * nmemb);
    return result;
}

void
free(void *ptr)
{
    CHECK_SYMBOLS;
    if (handle_deallocation(&allocdata, ptr) != alloc_ok) {
	fprintf(stderr,
		"free()ing memory at %p which wasn't malloc()ed!\n",
		ptr);
	abort();
    }
    real_free(ptr);
}

void *
realloc(void *ptr, size_t size)
{
    void *result;
    CHECK_SYMBOLS;

    result = real_realloc(ptr, size);
    if (ptr != 0 && handle_deallocation(&allocdata, ptr) != alloc_ok) {
	fprintf(stderr,
		"realloc()ing memory at %p to %d which wasn't malloc()ed!\n",
		ptr, size);
	abort();
    }
    if (result != 0) {
	handle_allocation(&allocdata, result, size);
    }
    return result;
}
