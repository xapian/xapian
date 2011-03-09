/** @file esetinternal.h
 * @brief Xapian::ESet::Internal class
 */
/* Copyright (C) 2008,2010 Olly Betts
 * Copyright (C) 2011 Action Without Borders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_ESETINTERNAL_H
#define XAPIAN_INCLUDED_ESETINTERNAL_H

#include "xapian/base.h"
#include "xapian/enquire.h"
#include "xapian/types.h"

#include <algorithm>
#include <string>
#include <vector>

namespace Xapian {
    class Database;
    class ExpandDecider;

    namespace Internal {
	class ExpandWeight;

/// Class combining a term and its expand weight.
class ExpandTerm {
    friend class Xapian::ESetIterator;
    friend class Xapian::ESet::Internal;

    /// The expand weight calculated for this term.
    Xapian::weight wt;

    /// The term.
    std::string term;

  public:
    /// Constructor.
    ExpandTerm(Xapian::weight wt_, const std::string & term_)
	: wt(wt_), term(term_) { }

    /// Implement custom swap for ESet sorting efficiency.
    void swap(ExpandTerm & o) {
	std::swap(wt, o.wt);
	std::swap(term, o.term);
    }

    /// Ordering relation for ESet contents.
    bool operator<(const ExpandTerm & o) const {
	if (wt > o.wt) return true;
	if (wt < o.wt) return false;
	return term > o.term;
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

}

/// Class which actually implements Xapian::ESet.
class ESet::Internal : public Xapian::Internal::RefCntBase {
    friend class ESet;
    friend class ESetIterator;

    /** This is a lower bound on the ESet size if an infinite number of results
     *  were requested.
     *
     *  It will of course always be true that: ebound >= items.size()
     */
    Xapian::termcount ebound;

    /// The ExpandTerm objects which represent the items in the ESet.
    std::vector<Xapian::Internal::ExpandTerm> items;

    /// Don't allow assignment.
    void operator=(const Internal &);

    /// Don't allow copying.
    Internal(const Internal &);

  public:
    /// Construct an empty ESet::Internal.
    Internal() : ebound(0) { }

    /// Run the "expand" operation which fills the ESet.
    void expand(Xapian::termcount max_esize,
		const Xapian::Database & db,
		const Xapian::RSet & rset,
		const Xapian::ExpandDecider * edecider,
		const Xapian::Internal::ExpandWeight & eweight,
		Xapian::weight min_wt);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_ESETINTERNAL_H
