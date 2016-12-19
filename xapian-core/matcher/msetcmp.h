/** @file msetcmp.h
 * @brief MSetItem comparison functions and functors.
 */
/* Copyright (C) 2006,2007,2011 Olly Betts
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


#ifndef XAPIAN_INCLUDED_MSETCMP_H
#define XAPIAN_INCLUDED_MSETCMP_H

#include "api/omenquireinternal.h"

// typedef for MSetItem comparison function.
typedef bool (* mset_cmp)(const Xapian::Internal::MSetItem &,
			  const Xapian::Internal::MSetItem &);

/// Select the appropriate msetcmp function.
mset_cmp get_msetcmp_function(Xapian::Enquire::Internal::sort_setting sort_by, bool sort_forward, bool sort_value_forward);

/// MSetItem comparison functor.
class MSetCmp {
    mset_cmp fn;
  public:
    explicit MSetCmp(mset_cmp fn_) : fn(fn_) { }
    /// Return true if MSetItem a should be ranked above MSetItem b.
    bool operator()(const Xapian::Internal::MSetItem &a,
		    const Xapian::Internal::MSetItem &b) const {
	return fn(a, b);
    }
};

#endif // XAPIAN_INCLUDED_MSETCMP_H
