/* ompostlistiterator.h
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

#ifndef OM_HGUARD_OMPOSTLISTITERATOR_H
#define OM_HGUARD_OMPOSTLISTITERATOR_H

#include <iterator>
#include "omtypes.h"

class OmDatabase;
class OmPositionListIterator;

class OmPostListIterator {
    private:
	friend class OmDatabase; // So OmDatabase can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmPostListIterator &a, const OmPostListIterator &b);

	OmPostListIterator(Internal *internal_);

    public:
        ~OmPostListIterator();

	OmPostListIterator operator=(OmPostListIterator &o);
    
	const om_docid operator *();

	OmPostListIterator & operator++();

	OmPostListIterator operator++(int);

	// extra method, not required for an input_iterator
	OmPostListIterator skip_to(om_docid did);

    	// allow iteration of positionlist for current term
	OmPositionListIterator positionlist_begin();

	OmPositionListIterator positionlist_end();

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	// Allow use as an STL iterator
	typedef input_iterator_tag iterator_category;
	typedef om_docid value_type;
	typedef om_docid difference_type;
	typedef om_docid * pointer;
	typedef om_docid & reference;
};

#endif /* OM_HGUARD_OMPOSTLISTITERATOR_H */
