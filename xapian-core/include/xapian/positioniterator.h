/** \file positioniterator.h
 * \brief Classes for iterating through position lists
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2007,2009,2012 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSITIONITERATOR_H
#define XAPIAN_INCLUDED_POSITIONITERATOR_H

#include <iterator>
#include <string>

#include <xapian/base.h>
#include <xapian/derefwrapper.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class Database;
class PostingIterator;
class TermIterator;

/** An iterator pointing to items in a list of positions.
 */
class XAPIAN_VISIBILITY_DEFAULT PositionIterator {
    private:
	// friend classes which need to be able to construct us
	friend class PostingIterator;
	friend class TermIterator;
	friend class Database;

    public:
	class Internal;
	/// @private @internal Reference counted internals.
	Xapian::Internal::RefCntPtr<Internal> internal;

        friend bool operator==(const PositionIterator &a, const PositionIterator &b);

	// FIXME: ought to be private
	explicit PositionIterator(Internal *internal_);

	/// Default constructor - for declaring an uninitialised iterator
	PositionIterator();

	/// Destructor
        ~PositionIterator();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	PositionIterator(const PositionIterator &o);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const PositionIterator &o);

	/// Return the term position at the current iterator position.
	Xapian::termpos operator *() const;

	/// Advance the iterator to the next position.
	PositionIterator & operator++();

	/// Advance the iterator to the next position (postfix version).
	DerefWrapper_<termpos> operator++(int) {
	    Xapian::termpos tmp = **this;
	    operator++();
	    return DerefWrapper_<termpos>(tmp);
	}

	/** Advance the iterator to the specified termpos.
	 *
	 *  If the specified termpos isn't in the list, position ourselves on the
	 *  first termpos after it (or at_end() if no greater term positions are
	 *  present).
	 */
	void skip_to(Xapian::termpos pos);

	/// Return a string describing this object.
	std::string get_description() const;

	// Allow use as an STL iterator
	typedef std::input_iterator_tag iterator_category;
	typedef Xapian::termpos value_type;
	typedef Xapian::termpos_diff difference_type;  // "om_termposcount"
	typedef Xapian::termpos * pointer;
	typedef Xapian::termpos & reference;
};

/// Test equality of two PositionIterators
inline bool
operator==(const PositionIterator &a, const PositionIterator &b)
{
    return (a.internal.get() == b.internal.get());
}

/// Test inequality of two PositionIterators
inline bool
operator!=(const PositionIterator &a, const PositionIterator &b)
{
    return !(a == b);
}

}

#endif /* XAPIAN_INCLUDED_POSITIONITERATOR_H */
