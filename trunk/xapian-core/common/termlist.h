/** @file termlist.h
 * @brief Abstract base class for termlists.
 */
/* Copyright (C) 2007,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TERMLIST_H
#define XAPIAN_INCLUDED_TERMLIST_H

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/termiterator.h>

namespace Xapian {
    class PositionIterator;
    namespace Internal {
	class ExpandStats;
    }
}

/// Abstract base class for termlists.
class Xapian::TermIterator::Internal : public Xapian::Internal::RefCntBase {
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

    /// Return approximate size of this termlist.
    virtual Xapian::termcount get_approx_size() const = 0;

    /// Collate weighting information for the current term.
    virtual void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

    /// Return the termname at the current position.
    virtual std::string get_termname() const = 0;

    /// Return the wdf for the term at the current position.
    virtual Xapian::termcount get_wdf() const = 0;

    /// Return the term frequency for the term at the current position.
    virtual Xapian::doccount get_termfreq() const = 0;

    /** Return the collection frequency for the term at the current position.
     *
     *  This method is only implemented for subclasses of AllTermsList
     *  (and isn't currently used).
     */
    virtual Xapian::termcount get_collection_freq() const;

    /** Advance the current position to the next term in the termlist.
     *
     *  The list starts before the first term in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  @return	If a non-NULL pointer is returned, then the caller should
     *		substitute the returned pointer for its pointer to us, and then
     *		delete us.  This "pruning" can only happen for a non-leaf
     *		subclass of this class.
     */
    virtual Internal * next() = 0;

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after tname (or at_end() if no terms after tname exist).
     */
    virtual Internal * skip_to(const std::string &term) = 0;

    /// Return true if the current position is past the last term in this list.
    virtual bool at_end() const = 0;

    /// Return the length of the position list for the current position.
    virtual Xapian::termcount positionlist_count() const = 0;

    /// Return a PositionIterator for the current position.
    virtual Xapian::PositionIterator positionlist_begin() const = 0;
};

// In the external API headers, this class is Xapian::TermIterator::Internal,
// but in the library code it's still known as "TermList" in most places.
typedef Xapian::TermIterator::Internal TermList;

#endif // XAPIAN_INCLUDED_TERMLIST_H
