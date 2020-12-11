/** @file
 * @brief Abstract base class for iterating all terms in a database.
 */
/* Copyright (C) 2007,2008,2011 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_ALLTERMSLIST_H

#include "api/termlist.h"

/// Abstract base class for iterating all terms in a database.
class AllTermsList : public TermList {
    /// Don't allow assignment.
    void operator=(const AllTermsList &);

    /// Don't allow copying.
    AllTermsList(const AllTermsList &);

  protected:
    /// Only constructable as a base class for derived classes.
    AllTermsList() { }

  public:
    /// Return approximate size of this termlist.
    virtual Xapian::termcount get_approx_size() const;

    /// Return the termname at the current position.
    virtual std::string get_termname() const = 0;

    /** Return the wdf for the term at the current position.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::termcount get_wdf() const;

    /// Return the term frequency for the term at the current position.
    virtual Xapian::doccount get_termfreq() const = 0;

    /// Advance the current position to the next term in the termlist.
    virtual TermList *next() = 0;

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after @a term (or at_end() if no terms after @a term exist).
     */
    virtual TermList *skip_to(const std::string &term) = 0;

    /// Return true if the current position is past the last term in this list.
    virtual bool at_end() const = 0;

    /** Return true if the current position is past the last term in this list.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::termcount positionlist_count() const;

    /** Return a PositionIterator for the current position.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_ALLTERMSLIST_H
