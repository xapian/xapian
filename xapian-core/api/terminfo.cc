/** @file terminfo.cc
 * @brief Metadata for a term in a document
 */
/* Copyright 2017,2018 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "terminfo.h"

#include "omassert.h"

bool
TermInfo::add_position(Xapian::termcount wdf_inc, Xapian::termpos termpos)
{
    if (rare(deleted)) {
	wdf = wdf_inc;
	deleted = false;
	positions.push_back(termpos);
	return true;
    }

    wdf += wdf_inc;

    // Optimise the common case of adding positions in ascending order.
    if (positions.empty() || termpos > positions.back()) {
	positions.push_back(termpos);
	return false;
    }

    // We keep positions sorted, so use lower_bound() which can binary chop to
    // find the entry.
    auto i = lower_bound(positions.cbegin(), positions.cend(), termpos);
    // Add unless termpos is already in the list.
    if (i == positions.cend() || *i != termpos) {
	positions.insert(i, termpos);
    }
    return false;
}

bool
TermInfo::remove_position(Xapian::termpos termpos)
{
    Assert(!deleted);

    if (rare(positions.empty()))
	return false;

    // Special case removing the final position, which we can handle in O(1).
    if (positions.back() == termpos) {
	positions.pop_back();
	return true;
    }

    // We keep positions sorted, so use lower_bound() which can binary chop to
    // find the entry.
    auto i = lower_bound(positions.cbegin(), positions.cend(), termpos);
    if (i == positions.cend() || *i != termpos) {
	// Not there.
	return false;
    }
    positions.erase(i);
    return true;
}

Xapian::termpos
TermInfo::remove_positions(Xapian::termpos termpos_first,
			  Xapian::termpos termpos_last)
{
    Assert(!deleted);

    // Find the range [i, j) that the specified termpos range maps to.  Use
    // binary chop to search, since this is a sorted list.
    auto i = lower_bound(positions.cbegin(), positions.cend(), termpos_first);
    if (i == positions.cend() || *i > termpos_last) {
	return 0;
    }
    auto j = upper_bound(i, positions.cend(), termpos_last);
    size_t size_before = positions.size();
    positions.erase(i, j);
    return Xapian::termpos(size_before - positions.size());
}
