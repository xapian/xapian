/** @file
 * @brief Abstract base class for value streams.
 */
/* Copyright (C) 2007,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_VALUELIST_H
#define XAPIAN_INCLUDED_VALUELIST_H

#include <string>

#include "xapian/intrusive_ptr.h"
#include <xapian/types.h>
#include <xapian/valueiterator.h>

/// Abstract base class for value streams.
class Xapian::ValueIterator::Internal : public Xapian::Internal::intrusive_base {
    /// Don't allow assignment.
    void operator=(const Internal &);

    /// Don't allow copying.
    Internal(const Internal &);

  protected:
    /// Only constructable as a base class for derived classes.
    Internal() { }

  public:
    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~Internal();

    /// Return the docid at the current position.
    virtual Xapian::docid get_docid() const = 0;

    /// Return the value at the current position.
    virtual std::string get_value() const = 0;

    /// Return the value slot for the current position/this iterator.
    virtual Xapian::valueno get_valueno() const = 0;

    /// Return true if the current position is past the last entry in this list.
    virtual bool at_end() const = 0;

    /** Advance the current position to the next document in the value stream.
     *
     *  The list starts before the first entry in the list, so next(),
     *  skip_to() or check() must be called before any methods which need the
     *  context of the current position.
     */
    virtual void next() = 0;

    /** Skip forward to the specified docid.
     *
     *  If the specified docid isn't in the list, position ourselves on the
     *  first document after it (or at_end() if no greater docids are present).
     */
    virtual void skip_to(Xapian::docid) = 0;

    /** Check if the specified docid occurs in this valuestream.
     *
     *  The caller is required to ensure that the specified @a docid actually
     *  exists in the database.
     *
     *  This method acts like skip_to() if that can be done at little extra
     *  cost, in which case it then returns true.
     *
     *  Otherwise it simply checks if a particular docid is present.  If it
     *  is, it returns true.  If it isn't, it returns false, and leaves the
     *  position unspecified (and hence the result of calling methods which
     *  depend on the current position, such as get_docid(), are also
     *  unspecified).  In this state, next() will advance to the first matching
     *  position after @a docid, and skip_to() will act as it would if the
     *  position was the first matching position after @a docid.
     *
     *  The default implementation calls skip_to().
     */
    virtual bool check(Xapian::docid did);

    /// Return a string description of this object.
    virtual std::string get_description() const = 0;
};

// In the external API headers, this class is Xapian::ValueIterator::Internal,
// but in the library code it's known as "ValueList" in most places.
typedef Xapian::ValueIterator::Internal ValueList;

#endif // XAPIAN_INCLUDED_VALUELIST_H
