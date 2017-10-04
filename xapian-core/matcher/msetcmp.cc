/** @file msetcmp.cc
 * @brief Result comparison functions and functors.
 */
/* Copyright (C) 2006,2009,2013 Olly Betts
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

#include <config.h>

#include "msetcmp.h"

#include "api/result.h"

/* We use templates to generate the 14 different comparison functions
 * which we need.  This avoids having to write them all out by hand.
 */

// Order by did.  Helper comparison template function, which is used as the
// last fallback by the others.
template<bool FORWARD_DID, bool CHECK_DID_ZERO> inline bool
msetcmp_by_did(const Result& a, const Result& b)
{
    if (FORWARD_DID) {
	if (CHECK_DID_ZERO) {
	    // We want dummy did 0 to compare worse than any other.
	    if (a.get_docid() == 0) return false;
	    if (b.get_docid() == 0) return true;
	}
	return (a.get_docid() < b.get_docid());
    } else {
	return (a.get_docid() > b.get_docid());
    }
}

// Order by relevance, then docid.
template<bool FORWARD_DID> bool
msetcmp_by_relevance(const Result& a, const Result& b)
{
    if (a.get_weight() > b.get_weight()) return true;
    if (a.get_weight() < b.get_weight()) return false;
    return msetcmp_by_did<FORWARD_DID, true>(a, b);
}

// Order by value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID> bool
msetcmp_by_value(const Result& a, const Result& b)
{
    if (!FORWARD_VALUE) {
	// We want dummy did 0 to compare worse than any other.
	if (a.get_docid() == 0) return false;
	if (b.get_docid() == 0) return true;
    }
    if (a.get_sort_key() > b.get_sort_key()) return FORWARD_VALUE;
    if (a.get_sort_key() < b.get_sort_key()) return !FORWARD_VALUE;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

// Order by value, then relevance, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID> bool
msetcmp_by_value_then_relevance(const Result& a, const Result& b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.get_docid() == 0) return false;
	if (b.get_docid() == 0) return true;
    }
    if (a.get_sort_key() > b.get_sort_key()) return FORWARD_VALUE;
    if (a.get_sort_key() < b.get_sort_key()) return !FORWARD_VALUE;
    if (a.get_weight() > b.get_weight()) return true;
    if (a.get_weight() < b.get_weight()) return false;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

// Order by relevance, then value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID> bool
msetcmp_by_relevance_then_value(const Result& a, const Result& b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.get_docid() == 0) return false;
	if (b.get_docid() == 0) return true;
    }
    if (a.get_weight() > b.get_weight()) return true;
    if (a.get_weight() < b.get_weight()) return false;
    if (a.get_sort_key() > b.get_sort_key()) return FORWARD_VALUE;
    if (a.get_sort_key() < b.get_sort_key()) return !FORWARD_VALUE;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

static mset_cmp mset_cmp_table[] = {
    // Xapian::Enquire::Internal::REL
    msetcmp_by_relevance<false>,
    0,
    msetcmp_by_relevance<true>,
    0,
    // Xapian::Enquire::Internal::VAL
    msetcmp_by_value<false, false>,
    msetcmp_by_value<true, false>,
    msetcmp_by_value<false, true>,
    msetcmp_by_value<true, true>,
    // Xapian::Enquire::Internal::VAL_REL
    msetcmp_by_value_then_relevance<false, false>,
    msetcmp_by_value_then_relevance<true, false>,
    msetcmp_by_value_then_relevance<false, true>,
    msetcmp_by_value_then_relevance<true, true>,
    // Xapian::Enquire::Internal::REL_VAL
    msetcmp_by_relevance_then_value<false, false>,
    msetcmp_by_relevance_then_value<true, false>,
    msetcmp_by_relevance_then_value<false, true>,
    msetcmp_by_relevance_then_value<true, true>
};

mset_cmp get_msetcmp_function(Xapian::Enquire::Internal::sort_setting sort_by, bool sort_forward, bool sort_value_forward) {
    if (sort_by == Xapian::Enquire::Internal::REL) sort_value_forward = false;
    return mset_cmp_table[sort_by * 4 + sort_forward * 2 + sort_value_forward];
}
