/* malloccheck.c: a checking malloc() etc. wrapper
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

#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>

static void *(*c_malloc)(size_t) = 0;
static void *(*c_calloc)(size_t, size_t) = 0;
static void (*c_free)(void *, const char *) = 0;
static void *(*c_realloc)(void *, size_t) = 0;

static int have_symbols = 0;
static int in_get_symbols = 0;

static void
get_symbols(void)
{
    void *h;
    if (in_get_symbols) {
	fprintf(stderr, "get_symbols() being called before exiting!\n");
	abort();
    }
    in_get_symbols = 1;

    h = dlopen(NULL, RTLD_NOW);
    /* h = RTLD_DEFAULT; */
    if (c_malloc == 0) {
	c_malloc = dlsym(h, "checked_malloc");
    }
    if (c_realloc == 0) {
	c_realloc = dlsym(h, "checked_realloc");
    }
    if (c_calloc == 0) {
	c_calloc = dlsym(h, "checked_calloc");
    }
    if (c_free == 0) {
	c_free = dlsym(h, "checked_free");
    }
    dlclose(h);

    if (c_malloc && c_realloc && c_calloc && c_free) {
	have_symbols = 1;
    } else {
	fprintf(stderr, "%p %p %p %p\n", c_malloc, c_realloc, c_calloc, c_free);
	fprintf(stderr, "get_symbols(): can't get symbols for checked_malloc and friends\n");
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
    if (in_get_symbols) return naive_allocator(size);
    CHECK_SYMBOLS;
    if (!c_malloc) return naive_allocator(size);
    return c_malloc(size);
}

void *
calloc(size_t nmemb, size_t size)
{
    if (in_get_symbols) return naive_allocator(size * nmemb);;
    CHECK_SYMBOLS;
    if (!c_calloc) return naive_allocator(size * nmemb);
    return c_calloc(nmemb, size);
}

void *
realloc(void *ptr, size_t size)
{
    if (in_get_symbols) abort(); /* FIXME: handle better */
    CHECK_SYMBOLS;
    return c_realloc(ptr, size);
}

void
free(void *ptr)
{
    if (in_get_symbols) return; /* FIXME: handle better */
    CHECK_SYMBOLS;
    c_free(ptr, "free()ing memory at %p which wasn't malloc()ed!\n");
}
