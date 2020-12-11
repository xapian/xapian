/** @file
 * @brief A termlist containing all terms in a honey database.
 */
/* Copyright (C) 2005,2007,2008,2009,2010,2011,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_HONEY_ALLTERMSLIST_H

#include "backends/alltermslist.h"
#include "honey_database.h"
#include "honey_postlist.h"

class HoneyCursor;

class HoneyAllTermsList : public AllTermsList {
    /// Copying is not allowed.
    HoneyAllTermsList(const HoneyAllTermsList&) = delete;

    /// Assignment is not allowed.
    HoneyAllTermsList& operator=(const HoneyAllTermsList&) = delete;

    /** Reference to our database.
     *
     *  We need this to stop it being deleted, and so we can lazily initialise
     *  @a cursor.
     */
    Xapian::Internal::intrusive_ptr<const HoneyDatabase> database;

    /** A cursor which runs through the postlist table reading termnames from
     *  the keys.
     *
     *  The cursor is lazily initialised so that we can avoid the initial
     *  positioning if skip_to() is the first action, and also so we can
     *  avoid having to seek to the item before the first matching @a prefix
     *  just so that the first next() advances us to where we want to start.
     *
     *  We also set this to NULL to signal the iterator has reached the
     *  end - in this case database will also be NULL.
     */
    HoneyCursor* cursor = NULL;

    /// The termname at the current position.
    std::string current_term;

    /// The prefix to restrict the terms to.
    std::string prefix;

    /** The term frequency of the term at the current position.
     *
     *  If this value is zero, then we haven't read the term frequency for the
     *  current term yet.  We need to call read_termfreq() to read this.
     */
    mutable Xapian::doccount termfreq = 0;

    /// Read and cache the term frequency.
    void read_termfreq() const;

  public:
    HoneyAllTermsList(const HoneyDatabase* database_,
		      const std::string& prefix_)
	: database(database_), prefix(prefix_) {}

    /// Destructor.
    ~HoneyAllTermsList();

    Xapian::termcount get_approx_size() const;

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Returns the term frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::doccount get_termfreq() const;

    /// Advance to the next term in the list.
    TermList* next();

    /// Advance to the first term which is >= term.
    TermList* skip_to(const std::string& term);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_INCLUDED_HONEY_ALLTERMSLIST_H */
