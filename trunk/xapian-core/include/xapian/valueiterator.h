/** \file valueiterator.h
 * \brief classes for iterating through values
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
#include <xapian/document.h>

namespace Xapian {

/** An iterator pointing to values associated with a document.
 */
class ValueIterator {
    private:
	friend class Document;
        friend bool operator==(const ValueIterator &a, const ValueIterator &b);
        friend bool operator!=(const ValueIterator &a, const ValueIterator &b);

	ValueIterator(Xapian::valueno index_, const Document & doc_)
	    : index(index_), doc(doc_) { }

	Xapian::valueno index;
	Document doc;

    public:
	/** Create an uninitialised iterator; this cannot be used, but is
	 *  convenient syntactically.
	 */
        ValueIterator() : index(0), doc() { }

        ~ValueIterator() { }

	/// Copying is allowed (and is cheap).
	ValueIterator(const ValueIterator &other) {
	    index = other.index;
	    doc = other.doc;
	}

        /// Assignment is allowed (and is cheap).
	void operator=(const ValueIterator &other) {
	    index = other.index;
	    doc = other.doc;
	}

	/// Advance the iterator.
	ValueIterator & operator++() {
	    ++index;
	    return *this;
	}

	/// Advance the iterator (postfix variant).
	ValueIterator operator++(int) {
	    ValueIterator tmp = *this;
	    ++index;
	    return tmp;
	}

	/// Get the value for the current position.
	const std::string & operator*() const;

	/// Get the value for the current position.
	const std::string * operator->() const;

	/// Get the number of the value at the current position
        Xapian::valueno get_valueno() const;

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

inline bool operator==(const ValueIterator &a, const ValueIterator &b)
{
    return (a.index == b.index);
}

inline bool operator!=(const ValueIterator &a, const ValueIterator &b)
{
    return (a.index != b.index);
}

}

#endif /* OM_HGUARD_OMVALUEITERATOR_H */
