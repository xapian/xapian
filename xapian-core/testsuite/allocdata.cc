/* allocdata.cc: implementation of functions related to allocdata
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
#include "omdebug.h"
#include <iostream>
#include <cstdio>

const unsigned int max_allocators = 10;
static unsigned int num_allocators = 0;

struct allocator_desc {
    const char *name;
    struct allocation_data *allocdata;
};

static allocator_desc allocators[max_allocators];

/** Get a snapshot of the current allocation state
 */
allocation_snapshot
get_alloc_snapshot()
{
    return allocation_snapshot(allocators, num_allocators);
}

/** Compare two allocation snapshots.
 */
bool
allocation_snapshot::operator==(const allocation_snapshot &other)
{
    // FIXME: perhaps make this a more thorough comparison
    if (data.size() != other.data.size()) {
	return false;
    }
    std::vector<snapshot_data>::const_iterator i;
    std::vector<snapshot_data>::const_iterator j;
    for (i = data.begin(), j = other.data.begin();
	 i != data.end(), j != other.data.end();
	 ++i, ++j) {
	if (i->num_allocations != j->num_allocations) {
	    return false;
	}
    }
    return true;
}

allocation_snapshot::allocation_snapshot(const allocator_desc *allocators,
					 int num_allocators)
{
    data.resize(num_allocators);
    for (int i=0; i<num_allocators; ++i) {
	DEBUGLINE(UNKNOWN, "Allocations for " << allocators[i].name << ": ["
		  << allocators[i].allocdata->num_allocations << "/"
		  << allocators[i].allocdata->allocations_bound << "]");
	data[i].num_allocations = allocators[i].allocdata->num_allocations;
	data[i].allocations_bound = allocators[i].allocdata->allocations_bound;
    }
}

void
print_alloc_differences(const struct allocation_snapshot &before,
			const struct allocation_snapshot &after,
			std::ostream &out)
{
    if (before.data.size() != after.data.size() ||
	before.data.size() > num_allocators) {
	out << "allocation snapshots are incompatible!";
	return;
    }
    for (unsigned int alloc = 0; alloc < before.data.size(); ++alloc) {
	if (after.data[alloc].num_allocations >
	    before.data[alloc].num_allocations) {
	    out << after.data[alloc].num_allocations -
		    before.data[alloc].num_allocations
		    << " extra " << allocators[alloc].name
		    << " allocations not freed: ";
	    for (int i=before.data[alloc].allocations_bound;
		 i<after.data[alloc].allocations_bound;
		 ++i) {
		allocation_data *allocdata = allocators[alloc].allocdata;
		if (allocdata->allocations[i].p != 0) {
		    out << hex;
		    out << allocdata->allocations[i].p << "("
			    << allocdata->allocations[i].size << ") ";
		    out << dec;
		}
	    }
	    out << std::endl;
	} else if (after.data[alloc].num_allocations <
	    before.data[alloc].num_allocations) {
	    out << before.data[alloc].num_allocations -
		    after.data[alloc].num_allocations
		    << " more allocations freed than were allocated by "
		    << allocators[alloc].name
		    << std::endl;
	}
    }
}

/************ Functions for use of allocator functions **********/

/** Register an allocator type to be used with leak detection. */
void
register_allocator(const char *name,
		   struct allocation_data *allocdata)
{
    if (num_allocators >= max_allocators) {
	std::cerr << "Tried to register allocator " << name
		<< ", but there are no free slots." << std::endl;
	return;
    };
//    std::cerr << "Registering allocator: " << name << std::endl;
    allocators[num_allocators].name = name;
    allocators[num_allocators].allocdata = allocdata;
    num_allocators++;
}
