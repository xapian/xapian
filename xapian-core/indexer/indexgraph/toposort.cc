/* toposort.cc: A topological sort
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
#include "config.h"
#include "toposort.h"
#include "om/omerror.h"
#include "omassert.h"

/*  A class for doing topological sorts.  The implementation is based on
 *  "Algorithm T" from Knuth, _The Art of Computer Programming_, volume 1,
 *  ~pages 267-271.
 */
TopoSort::TopoSort(int numelements_)
	: v(numelements_)
{}

void
TopoSort::add_pair(int first, int second)
{
    Assert(first >= 0 && first < v.size());
    Assert(second >= 0 && second < v.size());

    // increment predecessor count of second
    v[second].count++;

    // add second to first's list of successors
    v[first].suc.push_back(second);
}

TopoSort::result_type
TopoSort::get_result()
{
    // first step: scan through for zeroes.
    int next_zero = -1;

    for (int i=0; i<v.size(); ++i) {
	if (v[i].count == 0) {
	    // link element in
	    v[i].qlink = next_zero;
	    next_zero = i;
	}
    }

    size_t elems_to_go = v.size();

    result_type result;

    while (next_zero != -1) {
	// append this to the result
	result.push_back(next_zero);
	elems_to_go--;

	// iterate through the successors
	std::vector<int>::const_iterator i = v[next_zero].suc.begin();
	while (i != v[next_zero].suc.end()) {
	    v[*i].count--;
	    if (v[*i].count == 0) {
		// link this into the list of zeroes
		v[*i].qlink = v[next_zero].qlink;
		v[next_zero].qlink = *i;
	    }
	    ++i;
	}
	next_zero = v[next_zero].qlink;
    }
    if (elems_to_go > 0) {
	// FIXME: It would be helpful to tell the user what the loop was here.
	throw OmInvalidDataError("The indexer graph had a loop");
    }
    return result;
}
