/** @file  valueiterator.h
 *  @brief Class for iterating over document values.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_VALUEITERATOR_H
#define XAPIAN_INCLUDED_VALUEITERATOR_H

#include <iterator>
#include <string>

#include <xapian/base.h>
#include <xapian/derefwrapper.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

/// @private @internal A proxy class for an end ValueIterator.
class ValueIteratorEnd_ { };

/// Class for iterating over document values.
class XAPIAN_VISIBILITY_DEFAULT ValueIterator {
  public:
    /// Class representing the ValueIterator internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// @private @internal Construct given internals.
    explicit ValueIterator(Internal *internal_);

    /// Copy constructor.
    ValueIterator(const ValueIterator & o);

    /// @internal Copy from an end iterator proxy.
    ValueIterator(const ValueIteratorEnd_ &);

    /// Assignment.
    ValueIterator & operator=(const ValueIterator & o);

    /// @internal Assignment of an end iterator proxy.
    ValueIterator & operator=(const ValueIteratorEnd_ &);

    /** Default constructor.
     *
     *  Creates an uninitialised iterator, which can't be used before being
     *  assigned to, but is sometimes syntactically convenient.
     */
    ValueIterator();

    /// Destructor.
    ~ValueIterator();

    /// Return the value at the current position.
    std::string operator*() const;

    /// Advance the iterator to the next position.
    ValueIterator & operator++();

    /// Advance the iterator to the next position (postfix version).
    DerefWrapper_<std::string> operator++(int) {
	const std::string & value(**this);
	operator++();
	return DerefWrapper_<std::string>(value);
    }

    /** Return the docid at the current position.
     *
     *  If we're iterating over values of a document, this method will throw
     *  Xapian::InvalidOperationError.
     */
    Xapian::docid get_docid() const;

    /** Return the value slot number for the current position.
     *
     *  If the iterator is over all values in a slot, this returns that slot's
     *  number.  If the iterator is over the values in a particular document,
     *  it returns the number of each slot in turn.
     */
    Xapian::valueno get_valueno() const;

    /** Advance the iterator to document id or value slot @a docid_or_slot.
     *
     *  If this iterator is over values in a document, then this method
     *  advances the iterator to value slot @a docid_or_slot, or the first slot
     *  after it if there is no value in slot @a slot.
     *
     *  If this iterator is over values in a particular slot, then this
     *  method advances the iterator to document id @a docid_or_slot, or the
     *  first document id after it if there is no value in the slot we're
     *  iterating over for document @a docid_or_slot.
     *
     *  Note: The "two-faced" nature of this method is due to how C++
     *  overloading works.  Xapian::docid and Xapian::valueno are both typedefs
     *  for the same unsigned integer type, so overloading can't distinguish
     *  them.
     *
     *  @param docid_or_slot	The docid/slot to advance to.
     */
    void skip_to(Xapian::docid docid_or_slot);

    /** Check if the specified docid occurs.
     *
     *  The caller is required to ensure that the specified document id
     *  @a did actually exists in the database.
     *
     *  This method acts like skip_to() if that can be done at little extra
     *  cost, in which case it then returns true.  This is how brass and
     *  chert databases behave because they store values in streams which allow
     *  for an efficient implementation of skip_to().
     *
     *  Otherwise it simply checks if a particular docid is present.  If it
     *  is, it returns true.  If it isn't, it returns false, and leaves the
     *  position unspecified (and hence the result of calling methods which
     *  depends on the current position, such as get_docid(), are also
     *  unspecified).  In this state, next() will advance to the first matching
     *  position after document @a did, and skip_to() will act as it would if
     *  the position was the first matching position after document @a did.
     *
     *  Currently the inmemory, flint, and remote backends behave in the
     *  latter way because they don't support streamed values and so skip_to()
     *  must check each document it skips over which is significantly slower.
     *
     *  @param docid	The document id to check.
     */
    bool check(Xapian::docid docid);

    /// Return a string describing this object.
    std::string get_description() const;

    /** @private @internal ValueIterator is what the C++ STL calls an
     *  input_iterator.
     *
     *  The following typedefs allow std::iterator_traits<> to work so that
     *  this iterator can be used with with STL.
     *
     *  These are deliberately hidden from the Doxygen-generated docs, as the
     *  machinery here isn't interesting to API users.  They just need to know
     *  that Xapian iterator classes are compatible with the STL.
     */
    // @{
    /// @private
    typedef std::input_iterator_tag iterator_category;
    /// @private
    typedef std::string value_type;
    /// @private
    typedef Xapian::doccount_diff difference_type;
    /// @private
    typedef std::string * pointer;
    /// @private
    typedef std::string & reference;
    // @}
};

/// Equality test for ValueIterator objects.
inline bool
operator==(const ValueIterator &a, const ValueIterator &b)
{
    // Use a pointer comparison - this ensures both that (a == a) and correct
    // handling of end iterators (which we ensure have NULL internals).
    return a.internal.get() == b.internal.get();
}

/// @internal Equality test for ValueIterator object and end iterator.
inline bool
operator==(const ValueIterator &a, const ValueIteratorEnd_ &)
{
    return a.internal.get() == NULL;
}

/// @internal Equality test for ValueIterator object and end iterator.
inline bool
operator==(const ValueIteratorEnd_ &a, const ValueIterator &b)
{
    return b == a;
}

/// @internal Equality test for end iterators.
inline bool
operator==(const ValueIteratorEnd_ &, const ValueIteratorEnd_ &)
{
    return true;
}

/// Inequality test for ValueIterator objects.
inline bool
operator!=(const ValueIterator &a, const ValueIterator &b)
{
    return !(a == b);
}

/// @internal Inequality test for ValueIterator object and end iterator.
inline bool
operator!=(const ValueIterator &a, const ValueIteratorEnd_ &b)
{
    return !(a == b);
}

/// @internal Inequality test for ValueIterator object and end iterator.
inline bool
operator!=(const ValueIteratorEnd_ &a, const ValueIterator &b)
{
    return !(a == b);
}

/// @internal Inequality test for end iterators.
inline bool
operator!=(const ValueIteratorEnd_ &a, const ValueIteratorEnd_ &b)
{
    return !(a == b);
}

}

#endif // XAPIAN_INCLUDED_VALUEITERATOR_H
