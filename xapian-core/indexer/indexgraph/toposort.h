/* toposort.h: A topological sort
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
#ifndef OM_HGUARD_TOPOSORT_H
#define OM_HGUARD_TOPOSORT_H

#include "config.h"

#include <vector>

/** A class for doing topological sorts.  The implementation is based on
 *  "Algorithm T" from Knuth, _The Art of Computer Programming_, volume 1,
 *  ~pages 267-271.
 */
class TopoSort {
    public:
	/** Initialise the object.
	 *
	 *  @param numelements_	The number of elements to be sorted.
	 */
	TopoSort(int numelements_);

	/** Add a pair to the relation.
	 *
	 *  add_pair(a, b) means that a -< b (a precedes b) in the relation.
	 *  After sorting, a will come sometime before b.
	 *
	 *  @param first	An element which precedes second
	 *  @param second	An element which follows first
	 */
	void add_pair(int first, int second);

	typedef std::vector<int> result_type;

	/** Get the result of the sort.
	 *
	 * @except OmInvalidDataError	Thrown when the relation has loops.
	 *
	 * @return A vector v describing the order.  Element a is at
	 *         position v[a] in the sorted order.  The list will
	 *         contain the integers 0 ... (numelements-1) in some
	 *         order.
	 */
	result_type get_result();
    private:
	/** The information held on each item. */
	struct elem {
	    /** The number of direct predecessors of this element.
	     */
	    int count;
	    /** Link to the next element with zero count.  (Only used
	     *  after count becomes 0.  Knuth uses count for this purpose,
	     *  but it's clearer this way.
	     */
	    int qlink;
	    /** The list of direct successors to this element.
	     */
	    std::vector<int> suc;

	    /** The default initialisation */
	    elem() : count(0), qlink(-1) {};
	};
	/** The structure containing all the information built up from
	 *  the pairs.
	 */
	std::vector<elem> v;
};

#endif // OM_HGUARD_TOPOSORT_H
