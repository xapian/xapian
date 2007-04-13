/* alltermslist.h: Base class for iterating all terms in a database.
 *
 * Copyright (C) 2007 Olly Betts
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

#include "termlist.h"

/// Base class for iterating all terms in a database.
class AllTermsList : public TermList {
    /// Don't allow assignment.
    void operator=(const AllTermsList &);

    /// Don't allow copying.
    AllTermsList(const AllTermsList &);

  protected:
    /// Only constructable as a base class for derived classes.
    AllTermsList() { }

  public:
    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~AllTermsList();

    /// Return approximate size of this termlist.
    virtual Xapian::termcount get_approx_size() const = 0;

    /// Return weighting infomation for the current term.
    virtual OmExpandBits get_weighting() const;

    /// Return the current termname.
    virtual std::string get_termname() const = 0;

    /** Return the wdf of the current term.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::termcount get_wdf() const;

    /// Return the term frequency of the current term.
    virtual Xapian::doccount get_termfreq() const = 0;

    /// Return the collection frequency of the current term.
    virtual Xapian::termcount get_collection_freq() const = 0;

    /// Advanced to the next term in the list.
    virtual TermList *next() = 0;

    /** Skip foward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after tname (or at_end() if no terms after tname exist).
     */
    virtual TermList *skip_to(const std::string &tname) = 0;

    /// Returns true if we've advanced past the end of the list.
    virtual bool at_end() const = 0;

    /** Return length of position list for the current term.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::termcount positionlist_count() const;

    /** Return PositionIterator for the current term.
     *
     *  This isn't meaningful for an AllTermsList, and will throw
     *  Xapian::InvalidOperationError if called.
     */
    virtual Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_ALLTERMSLIST_H
