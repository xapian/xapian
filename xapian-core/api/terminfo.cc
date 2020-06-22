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

#include <algorithm>

using namespace std;

void
TermInfo::merge() const
{
    Assert(!is_deleted());
    inplace_merge(positions.begin(),
		  positions.begin() + split,
		  positions.end());
    split = 0;
}

bool
TermInfo::add_position(Xapian::termcount wdf_inc, Xapian::termpos termpos)
{
    if (rare(is_deleted())) {
	wdf = wdf_inc;
	split = 0;
	positions.push_back(termpos);
	return true;
    }

    wdf += wdf_inc;

    // Optimise the common case of adding positions in ascending order.
    if (positions.empty()) {
	positions.push_back(termpos);
	return false;
    }
    if (termpos > positions.back()) {
	if (split) {
	    // Check for duplicate before split.
	    auto i = lower_bound(positions.cbegin(),
				 positions.cbegin() + split,
				 termpos);
	    if (i != positions.cbegin() + split && *i == termpos)
		return false;
	}
	positions.push_back(termpos);
	return false;
    }

    if (termpos == positions.back()) {
	// Duplicate of last entry.
	return false;
    }

    if (split > 0) {
	// We could merge in the new entry at the same time, but that seems to
	// make things much more complex for minor gains.
	merge();
    }

    // We keep positions sorted, so use lower_bound() which can binary chop to
    // find the entry.
    auto i = lower_bound(positions.cbegin(), positions.cend(), termpos);
    // Add unless termpos is already in the list.
    if (i == positions.cend() || *i != termpos) {
	auto new_split = positions.size();
	if (sizeof(split) < sizeof(Xapian::termpos)) {
	    if (rare(new_split > numeric_limits<decltype(split)>::max())) {
		// The split point would be beyond the size of the type used to
		// hold it, which is really unlikely if that type is 32-bit.
		// Just insert the old way in this case.
		positions.insert(i, termpos);
		return false;
	    }
	} else {
	    // This assertion should always be true because we shouldn't have
	    // duplicate entries and the split point can't be after the final
	    // entry.
	    AssertRel(new_split, <=, numeric_limits<decltype(split)>::max());
	}
	split = new_split;
	positions.push_back(termpos);
    }
    return false;
}

bool
TermInfo::remove_position(Xapian::termpos termpos)
{
    Assert(!is_deleted());

    if (rare(positions.empty()))
	return false;

    // Special case removing the final position, which we can handle in O(1).
    if (positions.back() == termpos) {
	positions.pop_back();
	if (split == positions.size()) {
	    split = 0;
	    // We removed the only entry from after the split.
	}
	return true;
    }

    if (split > 0) {
	// We could remove the requested entry at the same time, but that seems
	// fiddly to do.
	merge();
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
    Assert(!is_deleted());

    if (split > 0) {
	// We could remove the requested entries at the same time, but that
	// seems fiddly to do.
	merge();
    }

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
