/* ompaditerator.h
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

#ifndef OM_HGUARD_OMPADITERATOR_H
#define OM_HGUARD_OMPADITERATOR_H

#include <iterator>
#include "omtypes.h"
#include "omindexercommon.h"

class OmPadIterator {
    private:
	// friend classes which need to be able to construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmPadIterator &a, const OmPadIterator &b);

    public:
	// FIXME: ought to be private
	OmPadIterator(Internal *internal_);

        ~OmPadIterator();

	void operator=(OmPadIterator &o);
	OmPadIterator (const OmPadIterator &o);

	/** Return the name of the pointed-to pad */
	std::string operator *() const;

	OmPadIterator & operator++();

	void operator++(int);

	/* Extra methods not required for iterators */

	/** Return the high-level type of this pad */
	std::string get_type() const;

	/** Return the basic type of this pad */
	OmIndexerMessageType get_phys_type() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	// Allow use as an STL iterator
	typedef std::input_iterator_tag iterator_category;
	typedef std::string value_type;
	typedef int difference_type;  // "om_termposcount"
	typedef std::string * pointer;
	typedef std::string & reference;
};

inline bool operator!=(const OmPadIterator &a,
		       const OmPadIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMPADITERATOR_H */
