/** @file
 * @brief Iterate keys in a remote database.
 */
/* Copyright (C) 2007,2008,2011,2018,2020 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REMOTE_KEYLIST_H
#define XAPIAN_INCLUDED_REMOTE_KEYLIST_H

#include "backends/alltermslist.h"

/// Iterate keys in a remote database.
class RemoteKeyList : public AllTermsList {
    /// Don't allow assignment.
    void operator=(const RemoteKeyList&) = delete;

    /// Don't allow copying.
    RemoteKeyList(const RemoteKeyList&) = delete;

    std::string data;

    const char* p = NULL;

  public:
    /// Construct.
    RemoteKeyList(const std::string& prefix, std::string&& data_)
	: data(data_) {
	current_term = prefix;
    }

    /// Return approximate size of this termlist.
    Xapian::termcount get_approx_size() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /// Advance the current position to the next term in the termlist.
    TermList* next();

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after @a term (or at_end() if no terms after @a term exist).
     */
    TermList* skip_to(const std::string& term);
};

#endif // XAPIAN_INCLUDED_REMOTE_KEYLIST_H
