/** \file positionlistiterator.h
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

#ifndef XAPIAN_INCLUDED_POSITIONLISTITERATOR_H
#define XAPIAN_INCLUDED_POSITIONLISTITERATOR_H

#include <iterator>
#include <string>

#include <xapian/base.h>

#include <xapian/types.h>

class OmPostListIterator;
class OmDatabase;

namespace Xapian {

class TermIterator;

class PositionListIterator {
    private:
	// friend classes which need to be able to construct us
	friend class OmPostListIterator;
	friend class TermIterator;
	friend class OmDatabase;

    public:
	class Internal;
	/// @internal Reference counted internals.
	Xapian::Internal::RefCntPtr<Internal> internal;

        friend bool operator==(const PositionListIterator &a, const PositionListIterator &b);

	// FIXME: ought to be private
	PositionListIterator(Internal *internal_);

	/// Default constructor - for declaring an uninitialised iterator
	// PositionListIterator();

	/// Destructor
        ~PositionListIterator();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	PositionListIterator(const PositionListIterator &o);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(PositionListIterator &o);

	om_termpos operator *() const;

	PositionListIterator & operator++();

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

inline bool
operator==(const PositionListIterator &a,
	   const PositionListIterator &b)
{
    return (a.internal.get() == b.internal.get());
}

inline bool
operator!=(const Xapian::PositionListIterator &a,
	   const Xapian::PositionListIterator &b)
{
    return !(a == b);
}

}

#endif /* XAPIAN_INCLUDED_POSITIONLISTITERATOR_H */
