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

#include "alloccommon.h"
#include <sys/types.h>

void *
malloc(size_t size)
{
    fprintf(stderr, "malloc(%d)\n", size);
    return checked_malloc(size);
}

void
free(void *ptr)
{
    fprintf(stderr, "free(%p)\n", ptr);
    checked_free(ptr, "free()ing memory at %p which wasn't malloc()ed!\n");
}
