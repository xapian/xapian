/** \file omvalueiterator.h
 * \brief classes for iterating through values
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#ifndef OM_HGUARD_OMVALUEITERATOR_H
#define OM_HGUARD_OMVALUEITERATOR_H

#include <iterator>
#include <string>
#include <xapian/types.h>

class OmDocument;

/** An iterator pointing to values associated with a document.
 */
class OmValueIterator {
    public:
	class Internal;
	/// @internal Reference counted internals.
	Internal *internal;

    private:
	friend class OmDocument; // So OmDocument can construct us

        friend bool operator==(const OmValueIterator &a, const OmValueIterator &b);

	OmValueIterator(Internal *internal_);

    public:
	/// Default constructor - for declaring an uninitialised iterator
	OmValueIterator();

	/// Destructor
        ~OmValueIterator();

	/// Copying is allowed (and is cheap).
	OmValueIterator(const OmValueIterator &other);

        /// Assignment is allowed (and is cheap).
	void operator=(const OmValueIterator &other);

	OmValueIterator & operator++();

	void operator++(int);

	/// Get the value for the current position
	const std::string & operator *() const;

	/// Get the value for the current position
	const std::string * operator ->() const;

	/// Get the number of the value at the current position
        om_valueno get_valueno() const;

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{
	typedef std::input_iterator_tag iterator_category;
	typedef std::string value_type;
	typedef Xapian::valueno_diff difference_type;
	typedef std::string * pointer;
	typedef std::string & reference;
	//@}
};

inline bool operator!=(const OmValueIterator &a, const OmValueIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMVALUEITERATOR_H */
