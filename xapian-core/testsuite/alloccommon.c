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

#include "alloccommon.h"
#include <stdio.h>

/************ Functions for use of allocator functions **********/

/** Register an allocation in the table.
 */
void
handle_allocation(struct allocation_data *data,
		       void *address,
		       size_t size)
{
    if (data->allocations_bound >= max_allocations) {
	/* our array is too small - panic! */
	fprintf(stderr, "Ran out of room for memory tracking!\n");
	abort();
    } else {
	data->allocations[data->allocations_bound].p = address;
	data->allocations[data->allocations_bound].size = size;
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
    for (i = data->allocations_bound - 1;
	 i >= 0;
	 --i) {
	if (data->allocations[i].p == old_address) {
	    data->allocations[i].p = new_address;
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
    for (i = data->allocations_bound - 1;
	 i >= 0;
	 --i) {
	if (data->allocations[i].p == address) {
	    data->allocations[i].p = 0;
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
    --data->num_allocations;
    return alloc_ok;
}
