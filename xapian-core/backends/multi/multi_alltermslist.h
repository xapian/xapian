/** @file
 * @brief Class for merging AllTermsList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2011,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTI_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_MULTI_ALLTERMSLIST_H

#include "backends/alltermslist.h"

#include <string>

namespace Xapian {
class Database;
}

/// Class for merging AllTermsList objects from subdatabases.
class MultiAllTermsList : public AllTermsList {
    /// Don't allow assignment.
    void operator=(const MultiAllTermsList &);

    /// Don't allow copying.
    MultiAllTermsList(const MultiAllTermsList &);

    /** Current termname.
     *
     *  If current_term.empty(), then either we haven't started yet (and
     *  count != 0) or we've reached the end (and count == 0).
     */
    std::string current_term;

    /// Current termfreq (or 0 if not yet calculated).
    mutable Xapian::doccount current_termfreq;

    /// Number of TermList* entries in @a termlists.
    mutable size_t count;

    /// Sub-termlists which we use as a heap.
    TermList** termlists;

  public:
    /// Constructor.
    MultiAllTermsList(size_t count_, TermList** termlists_);

    /// Destructor.
    ~MultiAllTermsList();

    Xapian::termcount get_approx_size() const;

    /// Return the termname at the current position.
    std::string get_termname() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /// Advance the current position to the next term in the termlist.
    TermList *next();

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after @a term (or at_end() if no terms after @a term exist).
     */
    TermList *skip_to(const std::string &term);

    /// Return true if the current position is past the last term in this list.
    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_MULTI_ALLTERMSLIST_H
