/** \file expanddecider.h
 * \brief Classes for filtering which terms returned by expand
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef XAPIAN_INCLUDED_EXPANDDECIDER_H
#define XAPIAN_INCLUDED_EXPANDDECIDER_H

#include <set>

#include <xapian/enquire.h>
#include <xapian/visibility.h>

namespace Xapian {

/** One useful expand decision functor, which provides a way of
 *  filtering out a fixed list of terms from the expand set.
 */
class XAPIAN_VISIBILITY_DEFAULT ExpandDeciderFilterTerms : public ExpandDecider {
    public:
        /** Constructor, which takes a list of terms which
	 *  will be filtered out.
	 */
        ExpandDeciderFilterTerms(Xapian::TermIterator terms,
				 Xapian::TermIterator termsend);

        virtual int operator()(const std::string &tname) const;
    private:
        std::set<std::string> tset;
};

/** An expand decision functor which can be used to join two
 *  functors with an AND operation.
 */
class XAPIAN_VISIBILITY_DEFAULT ExpandDeciderAnd : public ExpandDecider {
    public:
    	/** Constructor, which takes as arguments the two
	 *  decision functors to AND together.
	 *  ExpandDeciderAnd will not delete its sub-functors.
	 */
	ExpandDeciderAnd(const ExpandDecider *left_,
	                 const ExpandDecider *right_);

	virtual int operator()(const std::string &tname) const;

    private:
        const ExpandDecider *left;
	const ExpandDecider *right;
};

}

#endif /* XAPIAN_INCLUDED_EXPANDDECIDER_H */
