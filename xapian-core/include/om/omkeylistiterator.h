/* omkeylistiterator.h
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

#ifndef OM_HGUARD_OMKEYLISTITERATOR_H
#define OM_HGUARD_OMKEYLISTITERATOR_H

#include <iterator>
#include "omtypes.h"

class OmDocument;
class OmKey;

class OmKeyListIterator {
    private:
	friend class OmDocument; // So OmDocument can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmKeyListIterator &a, const OmKeyListIterator &b);

	OmKeyListIterator(Internal *internal_);

    public:
        ~OmKeyListIterator();

	/// Copying is allowed (and is cheap).
	OmKeyListIterator(const OmKeyListIterator &other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmKeyListIterator &other);

	OmKeyListIterator & operator++();

	void operator++(int);

	/// Get the OmKey for the current position
	const OmKey & operator *() const;

	/// Get the OmKey for the current position
	const OmKey * operator ->() const;

	/// Get the number of the key at the current position
        om_keyno get_keyno() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{
	typedef std::input_iterator_tag iterator_category;
	// FIXME: these are almost certainly wrong:
	typedef om_docid value_type;
	typedef om_docid difference_type;
	typedef om_docid * pointer;
	typedef om_docid & reference;
	//@}
};

inline bool operator!=(const OmKeyListIterator &a,
		       const OmKeyListIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMKEYLISTITERATOR_H */
