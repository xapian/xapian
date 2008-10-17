/** @file  valueiterator.h
 *  @brief Class for iterating over document values.
 */
/* Copyright (C) 2008 Olly Betts
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

/// Class for iterating over document values.
class XAPIAN_VISIBILITY_DEFAULT ValueIterator {
  public:
    /// Class representing the valueiterator internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// @private @internal Construct given internals.
    explicit ValueIterator(Internal *internal_);

    /// Copy constructor.
    ValueIterator(const ValueIterator & o);

    /// Assignment.
    ValueIterator & operator=(const ValueIterator & o);

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
    DerefStringWrapper_ operator++(int) {
	std::string value(**this);
	operator++();
	return DerefStringWrapper_(value);
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
     *  advances the iterator to value slot @a docid_or_slot, or the first slot after
     *  it if there is no value in slot @a slot.
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
     */
    void skip_to(Xapian::docid docid_or_slot);

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

inline bool
operator==(const ValueIterator &a, const ValueIterator &b)
{
    // Use a pointer comparison - this ensures both that (a == a) and correct
    // handling of end iterators (which we ensure have NULL internals).
    return a.internal.get() == b.internal.get();
}

inline bool
operator!=(const ValueIterator &a, const ValueIterator &b)
{
    return !(a == b);
}

}

#endif // XAPIAN_INCLUDED_VALUEITERATOR_H
