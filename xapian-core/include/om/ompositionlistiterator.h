/** \file ompositionlistiterator.h
 * \brief Classes for iterating through position lists
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_OMPOSITIONLISTITERATOR_H
#define OM_HGUARD_OMPOSITIONLISTITERATOR_H

#include <iterator>
#include <string>
#include "om/omtypes.h"

class OmPostListIterator;
class Xapian::TermIterator;
class OmDatabase;

class OmPositionListIterator {
    private:
	// friend classes which need to be able to construct us
	friend class OmPostListIterator;
	friend class Xapian::TermIterator;
	friend class OmDatabase;

    public:
	class Internal;
	/// @internal Reference counted internals.
	Internal *internal;

        friend bool operator==(const OmPositionListIterator &a, const OmPositionListIterator &b);

	// FIXME: ought to be private
	OmPositionListIterator(Internal *internal_);

	/// Default constructor - for declaring an uninitialised iterator
	OmPositionListIterator();

	/// Destructor
        ~OmPositionListIterator();

	void operator=(OmPositionListIterator &o);
	OmPositionListIterator (const OmPositionListIterator &o);

	om_termpos operator *() const;

	OmPositionListIterator & operator++();

	void operator++(int);

	// extra method, not required for an input_iterator
	void skip_to(om_termpos pos);

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	// Allow use as an STL iterator
	typedef std::input_iterator_tag iterator_category;
	typedef om_termpos value_type;
	typedef om_termpos_diff difference_type;  // "om_termposcount"
	typedef om_termpos * pointer;
	typedef om_termpos & reference;
};

inline bool operator!=(const OmPositionListIterator &a,
		       const OmPositionListIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMPOSITIONLISTITERATOR_H */
