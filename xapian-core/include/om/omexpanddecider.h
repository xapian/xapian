/* omexpanddecider.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_OMEXPANDDECIDER_H
#define OM_HGUARD_OMEXPANDDECIDER_H

#include "om/omenquire.h"
#include <set>

/** One useful expand decision functor, which provides a way of
 *  filtering out a fixed list of terms from the expand set.
 */
class OmExpandDeciderFilterTerms : public OmExpandDecider {
    public:
        /** Constructor, which takes a list of terms which
	 *  will be filtered out.
	 */
        OmExpandDeciderFilterTerms(OmTermIterator terms,
				   OmTermIterator termsend);

        virtual int operator()(const om_termname &tname) const;
    private:
        std::set<om_termname> tset;
};

/** An expand decision functor which can be used to join two
 *  functors with an AND operation.
 */
class OmExpandDeciderAnd : public OmExpandDecider {
    public:
    	/** Constructor, which takes as arguments the two
	 *  decision functors to AND together.
	 *  OmExpandDeciderAnd will not delete its sub-functors.
	 */
	OmExpandDeciderAnd(const OmExpandDecider *left_,
	                   const OmExpandDecider *right_);

	virtual int operator()(const om_termname &tname) const;

    private:
        const OmExpandDecider *left;
	const OmExpandDecider *right;
};

#endif /* OM_HGUARD_OMEXPANDDECIDER_H */

