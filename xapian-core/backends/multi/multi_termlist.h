/** @file
 * @brief Adapter class for a TermList in a multidatabase
 */
/* Copyright (C) 2007,2010,2013,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTI_TERMLIST_H
#define XAPIAN_INCLUDED_MULTI_TERMLIST_H

#include "api/termlist.h"

#include <xapian/database.h>

using Xapian::Internal::intrusive_ptr_nonnull;

/** Adapter class for a TermList in a multidatabase.
 *
 *  Most methods just forward to @a real_termlist, but @a get_termfreq()
 *  fetches the combined term frequency from the multidatabase.
 */
class MultiTermList : public TermList {
    /// Don't allow assignment.
    void operator=(const MultiTermList &) = delete;

    /// Don't allow copying.
    MultiTermList(const MultiTermList &) = delete;

    /// The TermList in the subdatabase.
    TermList* real_termlist;

    /// The multidatabase.
    intrusive_ptr_nonnull<const Xapian::Database::Internal> db;

  public:
    /// Constructor.
    MultiTermList(const Xapian::Database::Internal* db_, TermList* real_termlist_);

    /// Destructor.
    ~MultiTermList();

    /// Return approximate size of this termlist.
    Xapian::termcount get_approx_size() const;

    /// Return the termname at the current position.
    std::string get_termname() const;

    /// Return the wdf for the term at the current position.
    Xapian::termcount get_wdf() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /** Advance the current position to the next term in the termlist.
     *
     *  The list starts before the first term in the list, so next(), skip_to()
     *  or check() must be called before any methods which need the context of
     *  the current position.
     *
     *  @return	If a non-NULL pointer is returned, then the caller should
     *		substitute the returned pointer for its pointer to us, and then
     *		delete us.  This "pruning" can only happen for a non-leaf
     *		subclass of this class.
     */
    Internal * next();

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after tname (or at_end() if no terms after tname exist).
     */
    Internal * skip_to(const std::string &term);

    /// Return true if the current position is past the last term in this list.
    bool at_end() const;

    /// Return the length of the position list for the current position.
    Xapian::termcount positionlist_count() const;

    /// Return a PositionIterator for the current position.
    PositionList* positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_MULTI_TERMLIST_H
