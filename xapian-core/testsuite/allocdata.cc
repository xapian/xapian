/* allocdata.cc: implementation of functions related to allocdata
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

#include "allocdata.h"
#include "omdebug.h"
#include <iostream>
#include <cstdio>

/** Get a snapshot of the current allocation state
 */
allocation_snapshot
get_alloc_snapshot()
{
    return allocdata.id;
}

bool
check_alloc_differences(allocation_snapshot before, allocation_snapshot after)
{
    int i;
    for (i = allocdata.allocations_bound - 1; i >= 0; --i) {
	if (allocdata.allocations[i].p
	    && allocdata.allocations[i].id > before) return false;
    }
    return true;
}

void
print_alloc_differences(allocation_snapshot before, allocation_snapshot after,
			std::ostream &out)
{
    int i;
    // out << "BEFORE " << int(before) << " AFTER " << int(after) << std::endl;
    for (i = allocdata.allocations_bound - 1; i >= 0; --i) {
	if (allocdata.allocations[i].p
	    && allocdata.allocations[i].id > before) {
	    out << std::hex;
	    out << allocdata.allocations[i].p << "["
		<< allocdata.allocations[i].size << "] "
		<< "(" << allocdata.allocations[i].id << ") ";
	    out << std::dec;
	    out << std::endl;
	}
    }
}
