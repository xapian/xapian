/** @file positionlist.h
 * @brief Abstract base class for iterating term positions in a document.
 */
/* Copyright (C) 2007,2010,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSITIONLIST_H
#define XAPIAN_INCLUDED_POSITIONLIST_H

#include <xapian/intrusive_ptr.h>
#include <xapian/positioniterator.h>
#include <xapian/types.h>

namespace Xapian {

/// Abstract base class for iterating term positions in a document.
class PositionIterator::Internal : public Xapian::Internal::intrusive_base
{
    /// Don't allow assignment.
    void operator=(const Internal &) = delete;

    /// Don't allow copying.
    Internal(const Internal &) = delete;

  protected:
    /// Only constructable as a base class for derived classes.
    Internal() { }

  public:
    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~Internal() { }

    /// Return approximate size of this positionlist.
    virtual Xapian::termcount get_approx_size() const = 0;

    /// Return the current position.
    virtual Xapian::termpos get_position() const = 0;

    /** Advance to the next entry in the positionlist.
     *
     *  The list starts before the first entry, so next() or skip_to() must be
     *  called before get_position().
     *
     *  @return	true if we're on a valid entry; false if we've reached the end
     *		of the list.
     */
    virtual bool next() = 0;

    /** Skip forward to the specified position.
     *
     *  If the specified position isn't in the list, position ourselves on the
     *  first entry after it.
     *
     *  @return	true if we're on a valid entry; false if we've reached the end
     *		of the list.
     */
    virtual bool skip_to(Xapian::termpos termpos) = 0;
};

}

typedef Xapian::PositionIterator::Internal PositionList;

#endif // XAPIAN_INCLUDED_POSITIONLIST_H
