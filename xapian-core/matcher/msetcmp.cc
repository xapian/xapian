/** @file
 * @brief MSetItem comparison functions.
 */
/* Copyright (C) 2006,2009,2013,2017 Olly Betts
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

#include "omassert.h"

/* We use templates to generate the 14 different comparison functions
 * which we need.  This avoids having to write them all out by hand.
 */

// Order by did.  Helper comparison template function, which is used as the
// last fallback by the others.
template<bool FORWARD_DID, bool CHECK_DID_ZERO>
static inline bool
msetcmp_by_did(const Xapian::Internal::MSetItem &a,
	       const Xapian::Internal::MSetItem &b)
{
    if (FORWARD_DID) {
	if (CHECK_DID_ZERO) {
	    // We want dummy did 0 to compare worse than any other.
	    if (a.did == 0) return false;
	    if (b.did == 0) return true;
	}
	return (a.did < b.did);
    } else {
	return (a.did > b.did);
    }
}

// Order by relevance, then docid.
template<bool FORWARD_DID>
static bool
msetcmp_by_relevance(const Xapian::Internal::MSetItem &a,
		     const Xapian::Internal::MSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return msetcmp_by_did<FORWARD_DID, true>(a, b);
}

// Order by value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID>
static bool
msetcmp_by_value(const Xapian::Internal::MSetItem &a,
		 const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// We want dummy did 0 to compare worse than any other.
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
    int sort_cmp = a.sort_key.compare(b.sort_key);
    if (sort_cmp > 0) return FORWARD_VALUE;
    if (sort_cmp < 0) return !FORWARD_VALUE;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

// Order by value, then relevance, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID>
static bool
msetcmp_by_value_then_relevance(const Xapian::Internal::MSetItem &a,
				const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
    int sort_cmp = a.sort_key.compare(b.sort_key);
    if (sort_cmp > 0) return FORWARD_VALUE;
    if (sort_cmp < 0) return !FORWARD_VALUE;
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

// Order by relevance, then value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID>
static bool
msetcmp_by_relevance_then_value(const Xapian::Internal::MSetItem &a,
				const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    int sort_cmp = a.sort_key.compare(b.sort_key);
    if (sort_cmp > 0) return FORWARD_VALUE;
    if (sort_cmp < 0) return !FORWARD_VALUE;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

MSetCmp
get_msetcmp_function(Xapian::Enquire::Internal::sort_setting sort_by,
		     bool sort_forward,
		     bool sort_val_reverse)
{
    switch (sort_by) {
	case Xapian::Enquire::Internal::REL:
	    if (sort_forward)
		return msetcmp_by_relevance<true>;
	    else
		return msetcmp_by_relevance<false>;
	case Xapian::Enquire::Internal::VAL:
	    if (sort_forward) {
		if (sort_val_reverse) {
		    return msetcmp_by_value<true, true>;
		} else {
		    return msetcmp_by_value<false, true>;
		}
	    } else {
		if (sort_val_reverse) {
		    return msetcmp_by_value<true, false>;
		} else {
		    return msetcmp_by_value<false, false>;
		}
	    }
	case Xapian::Enquire::Internal::VAL_REL:
	    if (sort_forward) {
		if (sort_val_reverse) {
		    return msetcmp_by_value_then_relevance<true, true>;
		} else {
		    return msetcmp_by_value_then_relevance<false, true>;
		}
	    } else {
		if (sort_val_reverse) {
		    return msetcmp_by_value_then_relevance<true, false>;
		} else {
		    return msetcmp_by_value_then_relevance<false, false>;
		}
	    }
	default:
	    // Must be REL_VAL, but handle with "default" to avoid warnings
	    // about falling off the end of the function.
	    AssertEq(sort_by, Xapian::Enquire::Internal::REL_VAL);
	    if (sort_forward) {
		if (sort_val_reverse) {
		    return msetcmp_by_relevance_then_value<true, true>;
		} else {
		    return msetcmp_by_relevance_then_value<false, true>;
		}
	    } else {
		if (sort_val_reverse) {
		    return msetcmp_by_relevance_then_value<true, false>;
		} else {
		    return msetcmp_by_relevance_then_value<false, false>;
		}
	    }
    }
}
