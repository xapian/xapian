/* allocdata.h: common definitions for the malloc and new tracking
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

#ifndef OM_HGUARD_ALLOCDATA_H
#define OM_HGUARD_ALLOCDATA_H

#include <iosfwd>
#include <cstdlib>
#include "alloccommon.h"
#include <vector>

/// A class encapsulating a snapshot of the allocation state.
class allocation_snapshot {
    public:
	friend allocation_snapshot get_alloc_snapshot();
	friend void print_alloc_differences(const allocation_snapshot &,
					    const allocation_snapshot &,
					    std::ostream &);

	allocation_snapshot(const allocation_snapshot &other)
		: data(other.data) {};
	void operator =(const allocation_snapshot &other) {
	    data = other.data;
	}

	bool operator==(const allocation_snapshot &other);
	bool operator!=(const allocation_snapshot &other) {
	    return !(*this == other);
	}
    private:
	allocation_snapshot(const struct allocator_desc *, int);

	struct snapshot_data {
	    long num_allocations;
	    long allocations_bound;
	};

	std::vector<snapshot_data> data;
};

/** Get a snapshot of the current allocation state
 */
allocation_snapshot get_alloc_snapshot();

/** Print out the differences between the before and after allocations
 */
void
print_alloc_differences(const allocation_snapshot &before,
			const allocation_snapshot &after,
			std::ostream &out);

#endif /* OM_HGUARD_ALLOCDATA_H */
