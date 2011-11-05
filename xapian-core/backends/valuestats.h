/* valuestats.h: Statistics about values.
 *
 * Copyright 2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_VALUESTATS_H
#define XAPIAN_INCLUDED_VALUESTATS_H

#include <string>
#include "xapian/types.h"

/** Class to hold statistics for a given slot. */
struct ValueStats {
    /** The number of documents which have a (non-empty) value stored in the
     *  slot.
     */
    Xapian::doccount freq;

    /** A lower bound on the values stored in the given value slot.
     */
    std::string lower_bound;

    /** An upper bound on the values stored in the given value slot.
     */
    std::string upper_bound;

    /// Construct an empty ValueStats object.
    ValueStats() : freq(0), lower_bound(), upper_bound() {}

    /** Clear the statistics.
     */
    void clear() {
	freq = 0;
	lower_bound.resize(0);
	upper_bound.resize(0);
    }
};

#endif /* XAPIAN_INCLUDED_VALUESTATS_H */
